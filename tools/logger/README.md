# VFD test logger (PC side)

Logs the STM32's data stream over the Nucleo's USB port (ST-LINK virtual COM port),
lets you type commands to the board, and saves each session's results straight to an
Excel workbook - no CSV import step needed to edit or plot it.

## One-time setup

1. Install **Python 3** from python.org. On the first installer screen, tick
   **"Add python.exe to PATH"**.
2. In the `.ioc`: **Connectivity → USART2 → NVIC Settings** → tick **USART2 global
   interrupt** → save → generate code → rebuild → flash. Without this, the board
   prints data but ignores anything you type.

## Running it

Double-click **`run_logger.bat`**. The first run installs `pyserial` and `openpyxl`
automatically. It finds the ST-LINK COM port by itself. If it can't, run it from a
command prompt as `run_logger.bat --port COM5`, using the number shown in Device
Manager → Ports.

Only one program can use the COM port at a time, so close PuTTY/Tera Term first. The
STM32CubeIDE debugger and SWV console can stay open: they use a different connection.

## Running tests

At startup the logger asks three questions:

1. **Equipment being tested** - whatever's on the hook, e.g. `Metal housing`.
2. **Its weight** in kg, e.g. `1.46`.
3. **Intended test frequency** in Hz, e.g. `6.36` - the frequency you're about to
   dial in on the drive's pot. This just names the session; the logger still
   measures and records the real output frequency for every run.

All three go into the session's file name. After that, **every motor run is saved
automatically**. You don't need TEST or STOP.

```
                     hold UP; rows scroll while the motor runs; release
>> run 1 saved (5.8 s): 10.00 Hz, 0.660 A over 18 steady rows
>> type the SERVO RPM for run 1 (e.g. 291, or 291 down) ...
291 up               the RPM you read on the display (+ direction, optional)
>> run 1: RPM 291 up, slip 0.0300 -> added to ..._summary.xlsx
                     lower with DOWN: also saved as a run; type its RPM + "down",
                     or press Enter to skip it
WEIGHT 2.5           change the equipment weight before the next runs
QUIT                 exit
```

The averages for each run use only the steady rows at the highest speed held in that
run. Ramp-up, pauses while turning the pot, and slowing down are left out. Slip = (sync
RPM − typed RPM) / sync RPM. Anything else you type (`VFDCHECK`, `DEBUG 1`) goes to
the board. The board's own `TEST`/`STOP` commands still work, but you don't need them.

## Run time

The drum (`Drum_Rev_0`) is hourglass-shaped and the cable winds on its 15.9 mm
waist, so one turn moves about 1.96 in of cable. At 10 Hz (about 290 RPM, driven
directly) that's about 9.5 in/s, and 64 in lasts about 6.7 s. The effective diameter
grows as cable piles up, so plan on 4–5 s. The logger prints `run lasted X s` after
every run and beeps at `--max-run` (default 5.0 s). On the first no-load run, measure
how far the cable moved, then adjust: `run_logger.bat --max-run 4`.

## Output files

To skip the startup questions, run `run_logger.bat --equipment "Metal housing" --weight 1.46 --freq 6.36`.

Each session creates a folder named from your three answers, with just two files in it:

| File (example) | Contents |
|---|---|
| `data/2026-10-07_Metalhousing_baseweight-1.46kg_testedfreq-6.36hz/` | The session folder |
| `..._summary.xlsx` | One row per run: direction, weight, target frequency, steady averages, RPM, slip - a real Excel workbook, edit it directly |
| `..._session_log.txt` | Everything sent and received, with PC timestamps (plain text, for troubleshooting) |

There's no per-run file and no all-rows file - every row the board sends is still used
to compute each run's steady-state averages, it's just kept in memory rather than
written to disk, so a session folder stays to these two files. The `.xlsx` is the one
to plot (load vs. slip, load vs. current) and the one `tools/analysis/analyze.py` reads.

Running the same equipment at the same weight and frequency again the same day reopens
that session's workbook and appends to it, rather than starting a new file - handy for
resuming after a crash, but worth knowing if you meant to start a fresh session instead.

### Summary columns

| Column | Meaning |
|---|---|
| `run` | Run number within this session |
| `direction` | `up`, `down`, or blank if you skipped typing it |
| `weight_kg` | Equipment weight (same for the whole session unless changed with `WEIGHT`) |
| `target_freq_hz` | The frequency you told the logger you intended to test at |
| `start_time` | When this run actually started (PC clock, to the millisecond) |
| `run_s` | How long the run lasted |
| `steady_rows` | How many rows went into the averages below - a run with only 1–2 is a stutter, not a clean hold |
| `avg_freq_hz` / `avg_current_a` / `sync_rpm` | Measured, steady-state values for this run |
| `actual_rpm` | The SERVO RPM you typed in |
| `slip` | (sync_rpm − actual_rpm) / sync_rpm |
