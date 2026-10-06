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

```
TEST                 board asks: "what weight are you using?"
2.5                  weight in kg (0 = no load) -> TEST 1 START, new CSV opened
                     press FWD, turn the pot up, wait for freq_hz/current_a to settle
RPM 285              shaft RPM from the SERVO display -> slip column fills in
                     (type RPM again if the display changes)
STOP                 TEST 1 END + averages -> CSV closed, summary.csv updated
QUIT                 exit the logger
```

Other commands: `VFDCHECK`, `CANCEL` (at the weight prompt), `DEBUG 1`/`DEBUG 0`.

The averages printed at `STOP` (frequency, current, RPM, slip) only count rows logged
**after** you typed `RPM`. Type it once the readings have settled, so ramp-up rows
don't skew the averages.

## Output files

Each run creates `data/session_<date>_<time>/` next to the script:

| File | Contents |
|---|---|
| `test_01_2p5kg_<time>.csv` | Every row from one test (TEST → STOP) |
| `summary.csv` | One line per test: load, row counts, steady-state averages |
| `all_rows.csv` | Every row in the session, including idle time between tests |
| `session_log.txt` | Everything sent and received, with PC timestamps |

All CSVs open directly in Excel. `summary.csv` is the one to plot (load vs. slip,
load vs. current).
