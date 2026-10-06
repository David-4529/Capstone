"""PC-side logger for the STM32 slip/load test rig.

Connects to the Nucleo's ST-LINK virtual COM port and saves every motor run
automatically - no TEST/STOP needed. At startup it asks for the baseline weight
(hook + scale) and the test weight on the hook. Each time the motor runs, that run
is saved to its own CSV; when the motor stops, the logger asks for the RPM shown on
the SERVO display and adds one line to the summary.

  data/2026-10-05_2106_base0.35kg/                         one folder per session
    2026-10-05_210712_run01_load2.5kg_base0.35kg.csv       one file per motor run
    2026-10-05_2106_summary_base0.35kg.csv                 one line per run
    2026-10-05_2106_all_rows_base0.35kg.csv                every row, runs and idle
    2026-10-05_2106_session_log.txt                        everything sent/received

Commands typed in the window:
  291 / 291 down     after a run: the SERVO RPM (and optionally the direction)
  Enter              after a run: skip the RPM
  WEIGHT 2.5         change the test weight for the following runs
  QUIT               save and exit
Anything else (VFDCHECK, DEBUG 1, ...) is sent to the board.

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
import traceback

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
    "pc_time", "run", "direction", "load_kg", "baseline_kg", "total_kg", "run_s",
    "rows", "steady_rows", "avg_freq_hz", "avg_current_a", "sync_rpm", "actual_rpm",
    "slip", "file",
]
STLINK_VID = 0x0483
RUNNING_FREQ_HZ = 0.5  # output frequency above this counts as "motor running"
NO_REPLY_WARNING_S = 2.0

# Steady rows more than this below the highest steady speed in a run (pauses while
# turning the pot, slowing down) are left out of that run's averages.
PLATEAU_TOL_HZ = 0.2
RPM_ANSWER_RE = re.compile(r"^(?:RPM\s*)?(\d+(?:\.\d+)?)(?:\s+(UP|DOWN|U|D))?$")


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


def report_crash(context):
    # Prints the error and saves it to crash_log.txt next to the script, so a problem
    # can be diagnosed instead of the window just closing.
    details = traceback.format_exc()
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "crash_log.txt")
    try:
        with open(path, "a", encoding="utf-8") as f:
            f.write(f"--- {now_str()} {context}\n{details}\n")
    except OSError:
        pass
    print(f"\n!! ERROR {context}\n{details}!! saved to {path} - send this to Claude\n")


def kg_label(kg):
    return f"{float(kg):g}kg"


def ask_kg(question):
    while True:
        try:
            text = input(f">> {question} (kg, Enter = 0): ").strip()
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
    def __init__(self, ser, out_dir, max_run_s, show_all, baseline_kg, load_kg, session_stamp):
        self.ser = ser
        self.out_dir = out_dir
        self.max_run_s = max_run_s
        self.show_all = show_all
        self.baseline_kg = baseline_kg
        self.load_kg = load_kg
        self.base_label = "base" + kg_label(baseline_kg)
        self.columns = list(DEFAULT_COLUMNS)
        self.lock = threading.Lock()
        self.running = True
        self.closed = False
        self.last_reply_time = 0.0

        self.run_count = 0
        self.run = None        # the run in progress: rows, file, start time
        self.pending = None    # finished run waiting for its SERVO RPM

        prefix = f"{session_stamp}_"
        self.session_log = open(os.path.join(out_dir, f"{prefix}session_log.txt"), "a", encoding="utf-8")
        self.session_log.write(f"{now_str()} baseline_kg={baseline_kg:g} load_kg={load_kg:g}\n")
        self.all_rows_file = open(os.path.join(out_dir, f"{prefix}all_rows_{self.base_label}.csv"),
                                  "a", newline="", encoding="utf-8")
        self.all_rows = csv.writer(self.all_rows_file)
        self.all_rows.writerow(["pc_time"] + self.columns)

        summary_path = os.path.join(out_dir, f"{prefix}summary_{self.base_label}.csv")
        self.summary_name = os.path.basename(summary_path)
        self.summary_file = open(summary_path, "a", newline="", encoding="utf-8")
        self.summary = csv.writer(self.summary_file)
        self.summary.writerow(SUMMARY_COLUMNS)

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
                    try:
                        self.handle_line(line)
                    except Exception:
                        report_crash(f"while handling the line: {line!r}")

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
                if line.startswith("# millis,"):
                    cols = line[2:].split(",")
                    if cols != self.columns:
                        self.columns = cols
                        self.all_rows.writerow(["pc_time"] + self.columns)
                return

            fields = line.split(",")
            if len(fields) != len(self.columns):
                return  # partial/garbled line - kept in the session log only
            self.all_rows.writerow([stamp] + fields)
            self.all_rows_file.flush()
            row = dict(zip(self.columns, fields))
            running = self.is_running(row)
            # Idle rows are saved but not shown, so the screen stays still while you type.
            if self.show_all or running or self.run is not None:
                print(line)

            if running and self.run is None:
                self.start_run()
            if self.run is not None:
                self.run["writer"].writerow([stamp] + fields)
                self.run["file"].flush()
                self.run["rows"].append(row)
                elapsed = time.monotonic() - self.run["t0"]
                if running and not self.run["warned"] and elapsed >= self.max_run_s:
                    self.run["warned"] = True
                    print(f"\a!!! {elapsed:.1f} s running - RELEASE THE BUTTON (--max-run {self.max_run_s:g})")
                if not running:
                    self.end_run()

    @staticmethod
    def is_running(row):
        try:
            return float(row.get("freq_hz", "0")) > RUNNING_FREQ_HZ
        except ValueError:
            return False

    # --- runs ----------------------------------------------------------------------
    def start_run(self):
        if self.pending is not None:
            print(f">> run {self.pending['n']}: no RPM entered - saved without it")
            self.write_summary(self.pending, None, "")
        self.run_count += 1
        name = (f"{datetime.datetime.now():%Y-%m-%d_%H%M%S}_run{self.run_count:02d}"
                f"_load{kg_label(self.load_kg)}_{self.base_label}.csv")
        f = open(os.path.join(self.out_dir, name), "w", newline="", encoding="utf-8")
        writer = csv.writer(f)
        writer.writerow(["pc_time"] + self.columns + ["load_kg_test", "baseline_kg"])
        self.run = {"n": self.run_count, "name": name, "file": f, "writer": writer,
                    "rows": [], "t0": time.monotonic(), "warned": False, "load": self.load_kg}

    def end_run(self):
        run = self.run
        self.run = None
        run["file"].close()
        run["seconds"] = time.monotonic() - run["t0"]
        run.update(self.steady_stats(run["rows"]))
        self.pending = run
        if run["steady_rows"]:
            print(f">> run {run['n']} saved ({run['seconds']:.1f} s): {run['avg_freq_hz']:.2f} Hz, "
                  f"{run['avg_current_a']:.3f} A over {run['steady_rows']} steady rows -> {run['name']}")
        else:
            print(f">> run {run['n']} saved ({run['seconds']:.1f} s) - too short, no steady speed -> {run['name']}")
        print(f">> type the SERVO RPM for run {run['n']} (e.g. 291, or 291 down) and press Enter,"
              f" or just Enter to skip")

    @staticmethod
    def steady_stats(rows):
        # Average only the rows at the highest constant speed the run held.
        steady = []
        for r in rows:
            try:
                if r.get("steady") == "1":
                    steady.append((float(r["freq_hz"]), float(r["current_a"]), float(r["sync_rpm"])))
            except (KeyError, ValueError):
                pass
        if not steady:
            return {"steady_rows": 0}
        top = max(f for f, _, _ in steady)
        plateau = [s for s in steady if s[0] >= top - PLATEAU_TOL_HZ]
        n = len(plateau)
        return {
            "steady_rows": n,
            "avg_freq_hz": sum(s[0] for s in plateau) / n,
            "avg_current_a": sum(s[1] for s in plateau) / n,
            "sync_rpm": sum(s[2] for s in plateau) / n,
        }

    def write_summary(self, run, rpm, direction):
        slip = ""
        if rpm is not None and run.get("sync_rpm"):
            slip = f"{(run['sync_rpm'] - rpm) / run['sync_rpm']:.4f}"
        fmt = lambda key, spec: format(run[key], spec) if key in run else ""
        self.summary.writerow([
            now_str(), run["n"], direction, f"{run['load']:g}", f"{self.baseline_kg:g}",
            f"{run['load'] + self.baseline_kg:g}", f"{run['seconds']:.1f}", len(run["rows"]),
            run["steady_rows"], fmt("avg_freq_hz", ".2f"), fmt("avg_current_a", ".3f"),
            fmt("sync_rpm", ".1f"), "" if rpm is None else f"{rpm:g}", slip, run["name"],
        ])
        self.summary_file.flush()
        return slip

    # --- typed input -----------------------------------------------------------
    def handle_input(self, text):
        cmd = text.strip().upper()
        with self.lock:
            if self.pending is not None:
                if not cmd:
                    self.write_summary(self.pending, None, "")
                    print(f">> run {self.pending['n']} added to {self.summary_name} without RPM")
                    self.pending = None
                    return
                m = RPM_ANSWER_RE.match(cmd)
                if m:
                    rpm = float(m.group(1))
                    direction = {"U": "up", "D": "down"}.get(m.group(2), (m.group(2) or "").lower())
                    slip = self.write_summary(self.pending, rpm, direction)
                    print(f">> run {self.pending['n']}: RPM {rpm:g}{' ' + direction if direction else ''}"
                          f"{', slip ' + slip if slip else ''} -> added to {self.summary_name}")
                    self.pending = None
                    return
            if not cmd:
                return
            parts = cmd.split()
            if parts[0] in ("WEIGHT", "LOAD") and len(parts) == 2:
                try:
                    kg = float(parts[1])
                except ValueError:
                    kg = -1
                if kg >= 0:
                    self.load_kg = kg
                    self.session_log.write(f"{now_str()} load_kg={kg:g}\n")
                    print(f">> test weight is now {kg:g} kg for the next runs")
                else:
                    print(">> usage: WEIGHT 2.5")
                return
        self.send(cmd)

    def send(self, cmd):
        with self.lock:
            self.session_log.write(f"{now_str()} > {cmd}\n")
            self.session_log.flush()
        sent_at = time.monotonic()
        self.ser.write((cmd + "\r\n").encode("ascii", errors="replace"))
        threading.Timer(NO_REPLY_WARNING_S, self.check_reply, args=(sent_at, cmd)).start()

    def check_reply(self, sent_at, cmd):
        if self.running and self.last_reply_time < sent_at:
            print(f"!! no reply to '{cmd}' from the board - is it running (LD2 blinking)?")

    def close(self):
        self.running = False
        with self.lock:
            self.closed = True
            if self.run is not None:
                self.run["file"].close()
                print(f">> run {self.run['n']} was still going - its rows are saved in {self.run['name']}")
            if self.pending is not None:
                self.write_summary(self.pending, None, "")
            for f in (self.session_log, self.all_rows_file, self.summary_file):
                f.close()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", help="serial port, e.g. COM5 (default: auto-detect ST-LINK)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", default="data", help="folder for session folders (default: ./data)")
    ap.add_argument("--max-run", type=float, default=5.0,
                    help="beep and warn when the motor has run this many seconds (default: 5.0)")
    ap.add_argument("--show-all", action="store_true",
                    help="show every data row, including idle ones (default: only while the motor runs)")
    ap.add_argument("--baseline", type=float,
                    help="baseline weight in kg (hook + scale); skips the startup question")
    ap.add_argument("--weight", type=float,
                    help="test weight in kg on the hook; skips the startup question")
    args = ap.parse_args()

    port = args.port or find_stlink_port()
    baseline_kg = args.baseline if args.baseline is not None else ask_kg(
        "Baseline weight - hook + scale only, no test weight")
    load_kg = args.weight if args.weight is not None else ask_kg(
        "Test weight on the hook for these runs (0 for the no-load baseline)")
    stamp = f"{datetime.datetime.now():%Y-%m-%d_%H%M}"
    out_dir = os.path.join(args.out, f"{stamp}_base{kg_label(baseline_kg)}")
    os.makedirs(out_dir, exist_ok=True)

    try:
        ser = serial.Serial(port, args.baud, timeout=0.2)
    except serial.SerialException as e:
        sys.exit(f"Couldn't open {port}: {e}\n"
                 f"Close anything else using it (PuTTY, Tera Term) and try again.")

    log = Logger(ser, out_dir, args.max_run, args.show_all, baseline_kg, load_kg, stamp)
    print(f">> connected to {port}, baseline {baseline_kg:g} kg, test weight {load_kg:g} kg")
    print(f">> saving to {out_dir}")
    print(">> every motor run is saved automatically - after each run, type the SERVO RPM")
    print(">> commands: WEIGHT <kg> (change test weight)   VFDCHECK   QUIT")
    print(f">> run-time warning at {args.max_run:g} s (change with --max-run)\n")

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
            try:
                log.handle_input(text)
            except Exception:
                report_crash(f"while handling {text!r}")
    except KeyboardInterrupt:
        pass
    finally:
        log.close()
        time.sleep(0.3)
        ser.close()
        print(f">> session saved in {out_dir}")


if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except Exception:
        report_crash("in the logger")
        sys.exit(1)
