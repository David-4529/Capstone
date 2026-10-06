# VFD test logger (PC side)

Logs the STM32's data stream over the Nucleo's USB port (ST-LINK virtual COM port),
lets you type commands to the board, and saves each test to its own CSV.

## One-time setup

1. Install **Python 3** from python.org. On the first installer screen, tick
   **"Add python.exe to PATH"**.
2. In the `.ioc`: **Connectivity → USART2 → NVIC Settings** → tick **USART2 global
   interrupt** → save → generate code → rebuild → flash. Without this, the board
   prints data but ignores anything you type.

## Running it

Double-click **`run_logger.bat`**. The first run installs `pyserial` automatically.
It finds the ST-LINK COM port by itself. If it can't, run it from a command prompt as
`run_logger.bat --port COM5`, using the number shown in Device Manager → Ports.

Only one program can use the COM port at a time, so close PuTTY/Tera Term first. The
STM32CubeIDE debugger and SWV console can stay open: they use a different connection.

## Running a test

Cable travel is only 64 in, so each run lasts a few seconds (see "Run time" below).
The board logs a row every 0.25 s and marks rows where the speed is constant
(`steady` = 1). You can type RPM **after** releasing the button.

```
TEST                 board asks: "what weight are you using?"
0                    weight in kg (0 = no load) -> TEST 1 START, new CSV opened
                     hold UP; read the RPM on the SERVO display; release before the
                     cable runs out (the logger beeps at --max-run seconds)
RPM 290              the RPM you read on the display
STOP                 TEST 1 END + averages -> CSV closed, summary.csv updated
                     lower the cable back down (not part of the test), repeat
QUIT                 exit the logger
```

Data rows only scroll on screen while the motor is running, so the window stays still
while you type (everything is still saved). Use `--show-all` to see every row.

Other commands: `VFDCHECK`, `CANCEL` (at the weight prompt), `DEBUG 1`/`DEBUG 0`.

The averages printed at `STOP` use only the steady rows at the highest speed held
during the test. Rows from the ramp-up, pauses while turning the pot, and slowing down
are left out. Slip in the summary = (average synchronous RPM − typed RPM) / average
synchronous RPM.

## Run time

The drum (`Drum_Rev_0`) is hourglass-shaped and the cable winds on its 15.9 mm
waist, so one turn moves about 1.96 in of cable. At 10 Hz (about 290 RPM, driven
directly) that's about 9.5 in/s, and 64 in lasts about 6.7 s. The effective diameter
grows as cable piles up, so plan on 4–5 s. The logger prints `run lasted X s` after
every run and beeps at `--max-run` (default 3.0 s). On the first no-load run, measure
how far the cable moved, then adjust: `run_logger.bat --max-run 4`.

## Output files

At startup the logger asks for the **baseline weight**: what always hangs on the
cable with no test weight (hook + scale), in kg. Press Enter for 0. To skip the
question, run `run_logger.bat --baseline 0.35`. The baseline goes into every file
name and into the summary (`baseline_kg`, and `total_kg` = load + baseline).

Each run creates a folder named after the date, time and baseline:

| File (example) | Contents |
|---|---|
| `data/2026-10-05_2106_base0.35kg/` | The session folder |
| `2026-10-05_210712_test01_load2.5kg_base0.35kg.csv` | Every row from one test (TEST → STOP) |
| `2026-10-05_2106_summary_base0.35kg.csv` | One line per test: load, baseline, total, steady-state averages |
| `2026-10-05_2106_all_rows_base0.35kg.csv` | Every row in the session, including idle time between tests |
| `2026-10-05_2106_session_log.txt` | Everything sent and received, with PC timestamps |

All CSVs open directly in Excel. The summary file is the one to plot (load vs. slip,
load vs. current).
