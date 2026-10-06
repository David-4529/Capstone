"""PC-side logger for the STM32 slip/load test rig.

Connects to the Nucleo's ST-LINK virtual COM port, shows the live data stream, sends
the commands you type (TEST, RPM <value>, STOP, ...) to the board, and saves the
data to CSV files:

  data/2026-10-05_2106_base0.35kg/                      one folder per session
    2026-10-05_2107_test01_load2.5kg_base0.35kg.csv     one file per test (TEST -> STOP)
    2026-10-05_2106_summary_base0.35kg.csv              one line per finished test
    2026-10-05_2106_all_rows_base0.35kg.csv             every row, tests and idle alike
    2026-10-05_2106_session_log.txt                     everything sent/received

At startup it asks for the baseline weight: what hangs on the cable with no test
weight (hook + scale), in kg. It goes into every file name and the summary, so
you know later what the rig carried on top of each test load.

Usage:
  python vfd_logger.py              # auto-detects the ST-LINK COM port
  python vfd_logger.py --port COM5  # or name it yourself

Type QUIT (or press Ctrl+C) to exit.
"""

import argparse
import csv
import datetime
import os
import re
import sys
import threading
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    sys.exit("pyserial is not installed. Run:  pip install pyserial")

# Matches the firmware's CSV header; replaced by the header line the board prints
# at startup ("# millis,freq_hz,...") if the logger sees it.
DEFAULT_COLUMNS = [
    "millis", "freq_hz", "freq_source", "sync_rpm", "actual_rpm", "slip",
    "current_a", "current_source", "known_load_kg", "test_id", "steady",
]
SUMMARY_COLUMNS = [
    "pc_time", "test_id", "load_kg", "rows", "steady_rows", "avg_freq_hz",
    "avg_current_a", "avg_rpm", "avg_slip", "baseline_kg", "total_kg", "file",
]
STLINK_VID = 0x0483
RUNNING_FREQ_HZ = 0.5  # output frequency above this counts as "motor running"
NO_REPLY_WARNING_S = 2.0

TEST_START_RE = re.compile(r"^# TEST (\d+) START load_kg=([\d.]+)")
TEST_END_RE = re.compile(r"^# TEST (\d+) END (.*)$")


def find_stlink_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if p.vid == STLINK_VID or "STLink" in (p.description or "") or "STM" in (p.description or ""):
            return p.device
    names = ", ".join(f"{p.device} ({p.description})" for p in ports) or "none found"
    sys.exit(f"Couldn't find the ST-LINK COM port. Ports seen: {names}\n"
             f"Run again with --port COMx (check Device Manager -> Ports).")


def now_str():
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


def kg_label(kg):
    return f"{float(kg):g}kg"


def ask_baseline_kg():
    # The weight that's always on the cable (hook + scale), so each file records
    # what the rig carried in addition to the test load.
    while True:
        try:
            text = input(">> Baseline weight in kg (hook + scale, no test weight; Enter = 0): ").strip()
        except EOFError:
            return 0.0
        if not text:
            return 0.0
        try:
            kg = float(text)
            if kg >= 0:
                return kg
        except ValueError:
            pass
        print(">> enter a number, e.g. 0.35")


class Logger:
    def __init__(self, ser, out_dir, max_run_s, show_all=False, baseline_kg=0.0, session_stamp=""):
        self.ser = ser
        self.baseline_kg = baseline_kg
        self.base_label = "base" + kg_label(baseline_kg)
        self.session_stamp = session_stamp
        self.show_all = show_all
        self.max_run_s = max_run_s
        self.run_start = None
        self.run_warned = False
        self.out_dir = out_dir
        self.columns = list(DEFAULT_COLUMNS)
        self.lock = threading.Lock()
        self.running = True
        self.closed = False
        self.last_reply_time = 0.0

        prefix = f"{session_stamp}_"
        self.session_log = open(os.path.join(out_dir, f"{prefix}session_log.txt"), "a", encoding="utf-8")
        self.session_log.write(f"{now_str()} baseline_kg={baseline_kg:g}\n")
        self.all_rows_file = open(os.path.join(out_dir, f"{prefix}all_rows_{self.base_label}.csv"),
                                  "a", newline="", encoding="utf-8")
        self.all_rows = csv.writer(self.all_rows_file)
        self.all_rows.writerow(["pc_time"] + self.columns)

        summary_path = os.path.join(out_dir, f"{prefix}summary_{self.base_label}.csv")
        self.summary_name = os.path.basename(summary_path)
        self.summary_file = open(summary_path, "a", newline="", encoding="utf-8")
        self.summary = csv.writer(self.summary_file)
        self.summary.writerow(SUMMARY_COLUMNS)

        self.test_id = None
        self.test_file = None
        self.test_writer = None
        self.test_path = None
        self.test_rows = 0

    # --- receiving -------------------------------------------------------------
    def reader_loop(self):
        buf = b""
        while self.running:
            try:
                chunk = self.ser.read(256)
            except serial.SerialException as e:
                print(f"\n!! serial port error: {e}")
                self.running = False
                break
            if not chunk:
                continue
            buf += chunk
            while b"\n" in buf:
                raw, buf = buf.split(b"\n", 1)
                line = raw.decode("utf-8", errors="replace").strip()
                if line:
                    self.handle_line(line)

    def handle_line(self, line):
        with self.lock:
            if self.closed:
                return
            stamp = now_str()
            self.session_log.write(f"{stamp} < {line}\n")
            self.session_log.flush()

            if line.startswith("#"):
                print(line)
                self.last_reply_time = time.monotonic()
                self.handle_comment(line, stamp)
                return

            fields = line.split(",")
            if len(fields) != len(self.columns):
                return  # partial/garbled line - kept in session_log.txt only
            self.all_rows.writerow([stamp] + fields)
            self.all_rows_file.flush()
            row = dict(zip(self.columns, fields))
            # Idle rows (motor stopped) are saved but not shown, so the screen stays
            # still and you can see what you type. --show-all shows every row.
            if self.show_all or self.is_running(row) or self.run_start is not None:
                print(line)
            self.track_run(row)

            if self.test_writer is not None:
                row = dict(zip(self.columns, fields))
                if row.get("test_id") == str(self.test_id):
                    self.test_writer.writerow([stamp] + fields)
                    self.test_file.flush()
                    self.test_rows += 1

    @staticmethod
    def is_running(row):
        try:
            return float(row.get("freq_hz", "0")) > RUNNING_FREQ_HZ
        except ValueError:
            return False

    def track_run(self, row):
        # Times each motor run from the data stream, so you know how many seconds of
        # cable travel you have, and beeps when a run passes --max-run seconds.
        running = self.is_running(row)
        t = time.monotonic()
        if running and self.run_start is None:
            self.run_start = t
            self.run_warned = False
        elif running and not self.run_warned and t - self.run_start >= self.max_run_s:
            self.run_warned = True
            print(f"\a!!! {t - self.run_start:.1f} s running - RELEASE THE BUTTON (--max-run {self.max_run_s:g})")
        elif not running and self.run_start is not None:
            print(f">> run lasted {t - self.run_start:.1f} s")
            self.run_start = None

    def handle_comment(self, line, stamp):
        if line.startswith("# millis,"):
            cols = line[2:].split(",")
            if cols != self.columns:
                self.columns = cols
                self.all_rows.writerow(["pc_time"] + self.columns)
            return

        m = TEST_START_RE.match(line)
        if m:
            self.close_test_file()
            self.test_id = int(m.group(1))
            load = m.group(2)
            name = (f"{datetime.datetime.now():%Y-%m-%d_%H%M%S}_test{self.test_id:02d}"
                    f"_load{kg_label(load)}_{self.base_label}.csv")
            self.test_path = os.path.join(self.out_dir, name)
            self.test_file = open(self.test_path, "w", newline="", encoding="utf-8")
            self.test_writer = csv.writer(self.test_file)
            self.test_writer.writerow(["pc_time"] + self.columns)
            self.test_rows = 0
            print(f">> logging test {self.test_id} ({load} kg) to {name}")
            return

        m = TEST_END_RE.match(line)
        if m:
            fields = dict(kv.split("=", 1) for kv in m.group(2).split() if "=" in kv)
            fname = os.path.basename(self.test_path) if self.test_path else ""
            self.summary.writerow([
                stamp, m.group(1), fields.get("load_kg", ""), fields.get("rows", ""),
                fields.get("steady_rows", ""), fields.get("avg_freq_hz", ""),
                fields.get("avg_current_a", ""), fields.get("avg_rpm", ""),
                fields.get("avg_slip", ""), f"{self.baseline_kg:g}",
                self.total_kg(fields.get("load_kg", "")), fname,
            ])
            self.summary_file.flush()
            saved = self.test_rows
            self.close_test_file()
            print(f">> test {m.group(1)} saved: {saved} rows -> {fname}  ({self.summary_name} updated)")

    def total_kg(self, load):
        try:
            return f"{float(load) + self.baseline_kg:g}"
        except ValueError:
            return ""

    def close_test_file(self):
        if self.test_file is not None:
            self.test_file.close()
        self.test_file = None
        self.test_writer = None
        self.test_id = None
        self.test_path = None

    # --- sending ---------------------------------------------------------------
    def send(self, text):
        cmd = text.strip().upper()
        if not cmd:
            return
        with self.lock:
            self.session_log.write(f"{now_str()} > {cmd}\n")
            self.session_log.flush()
        sent_at = time.monotonic()
        self.ser.write((cmd + "\r\n").encode("ascii", errors="replace"))
        threading.Timer(NO_REPLY_WARNING_S, self.check_reply, args=(sent_at, cmd)).start()

    def check_reply(self, sent_at, cmd):
        if self.running and self.last_reply_time < sent_at:
            print(f"!! no reply to '{cmd}' from the board. If typed commands never get a reply,\n"
                  f"!! tick NVIC -> 'USART2 global interrupt' in the .ioc, regenerate, rebuild.")

    def close(self):
        self.running = False
        with self.lock:
            self.closed = True
            if self.test_file is not None:
                print(f">> test {self.test_id} was still running - its rows so far are saved in "
                      f"{os.path.basename(self.test_path)}")
            self.close_test_file()
            for f in (self.session_log, self.all_rows_file, self.summary_file):
                f.close()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", help="serial port, e.g. COM5 (default: auto-detect ST-LINK)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", default="data", help="folder for session folders (default: ./data)")
    ap.add_argument("--max-run", type=float, default=3.0,
                    help="beep and warn when the motor has run this many seconds (default: 3.0)")
    ap.add_argument("--show-all", action="store_true",
                    help="show every data row, including idle ones (default: only while the motor runs)")
    ap.add_argument("--baseline", type=float,
                    help="baseline weight in kg (hook + scale); skips the startup question")
    args = ap.parse_args()

    port = args.port or find_stlink_port()
    baseline_kg = args.baseline if args.baseline is not None else ask_baseline_kg()
    stamp = f"{datetime.datetime.now():%Y-%m-%d_%H%M}"
    out_dir = os.path.join(args.out, f"{stamp}_base{kg_label(baseline_kg)}")
    os.makedirs(out_dir, exist_ok=True)

    try:
        ser = serial.Serial(port, args.baud, timeout=0.2)
    except serial.SerialException as e:
        sys.exit(f"Couldn't open {port}: {e}\n"
                 f"Close anything else using it (PuTTY, Tera Term) and try again.")

    log = Logger(ser, out_dir, args.max_run, args.show_all, baseline_kg, stamp)
    print(f">> connected to {port} at {args.baud} baud, baseline {baseline_kg:g} kg, saving to {out_dir}")
    print(">> commands: TEST (asks for weight)  RPM <value>  STOP  VFDCHECK  QUIT")
    print(f">> run-time warning at {args.max_run:g} s (change with --max-run)")
    print(">> data rows are shown only while the motor runs (all rows are still saved)")
    print(">> tip: press the Nucleo's reset button to see the startup VFDCHECK\n")

    reader = threading.Thread(target=log.reader_loop, daemon=True)
    reader.start()

    try:
        while log.running:
            try:
                text = input()
            except EOFError:
                break
            if text.strip().upper() in ("QUIT", "EXIT"):
                break
            log.send(text)
    except KeyboardInterrupt:
        pass
    finally:
        log.close()
        time.sleep(0.3)
        ser.close()
        print(f">> session saved in {out_dir}")


if __name__ == "__main__":
    main()
