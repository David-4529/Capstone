# STM32 Nucleo-F401RE Firmware

Single self-contained file: `Core/Src/main.c`. No other custom files needed — the
Modbus RTU logic is written directly into it, not a separate module.

## Starting a fresh CubeMX project

1. STM32CubeIDE → **File → New → STM32 Project → Board Selector** tab → search
   **NUCLEO-F401RE** → select it → Next → name it → Finish. Click **Yes** when asked
   to initialize peripherals with their default mode.
2. Pinout view:
   - Click **PA9** → `USART1_TX`. Click **PA10** → `USART1_RX`. (RS-485 link to the VFD.)
   - Click **PA8** → `GPIO_Output` → right-click → **Enter User Label** → `RS485_DE_RE`.
   - **Connectivity → USART1**: 9600 baud, 8 bits, **no parity, 2 stop bits** (8N2 —
     the VFD's P09.04 has no 8N1 option for RTU mode).
   - **Connectivity → USART2**: confirm it's enabled (Mode ≠ Disable) — this drives
     the ST-LINK virtual COM port used for the serial terminal. If it's not enabled,
     set PA2 → `USART2_TX`, PA3 → `USART2_RX` first.
   - **System Core → NVIC**: check **USART2 global interrupt** (needed to receive
     typed commands without blocking the main loop).
3. **Project → Generate Code.**
4. Open the generated `Core/Src/main.c` and copy this repo's code into the matching
   `/* USER CODE BEGIN ... */` / `/* USER CODE END ... */` sections (PD, PV, PFP, 2,
   WHILE, 4). Don't replace the whole file or project folder. CubeMX owns everything
   outside those markers, and your generated init code stays as it is.
5. Build and flash (Run button — the Nucleo's onboard ST-LINK handles this, no
   separate programmer needed).

## Verifying it works

Open a serial terminal to the Nucleo's ST-LINK VCP port at **115200 baud**. You
should see the CSV header lines, then a new row every 2 seconds. Typing
`FREQ 10`, `RPM 1650`, `CURRENT 1.2`, or `LOAD 0.5` (Enter after each) updates that
field — check for `manual` in the `freq_source`/`current_source` columns to confirm
the command was received.

## Verifying it reads from the VFD (`VFDCHECK`)

At startup, and whenever you type `VFDCHECK`, the firmware reads the drive's control
settings over Modbus. It reads only and never writes to the drive. With the drive
powered and RS-485 connected, you should see this (type `DEBUG 0` first to hide the
raw hex dumps):

```
# VFDCHECK P00.20 freq source = 2 OK (speed from pot on AI1)
# VFDCHECK P00.21 run source = 1 OK (start/stop from FWD/REV buttons)
# VFDCHECK P03.00 AI1 function = 1 OK (AI1 = frequency command)
# VFDCHECK P00.22 stop method = 0 OK (ramp to stop)
# VFDCHECK P01.10 freq upper limit = 10.00 Hz OK (safe max 10.00 Hz)
# VFDCHECK P01.12 accel time = 1.50 s (raw 150)
# VFDCHECK P01.13 decel time = 1.50 s (raw 150)
# VFDCHECK frequency command (pot setpoint) = 0.00 Hz
# VFDCHECK modbus link: 8/8 reads OK
# VFDCHECK: PASS - turn the pot to set speed, FWD/REV to run
```

Each value should match what GSoft2 shows for that parameter. If the accel/decel times
are 10x off (e.g. 15.00 s), the drive's time unit is 0.1 s and the raw value is still
correct. With the motor stopped, turn the pot and re-run `VFDCHECK`: the frequency
command line should change. Then run the motor and check that `freq_hz` and
`current_a` in the CSV rows match GSoft2's live monitor. The log prints a `WARNING` if
output frequency ever goes above `SAFE_MAX_FREQ_HZ` (10 Hz by default).

`0/8 reads OK` means no reply from the drive. Check the A/B wiring, 9600 baud 8N2, and
that the slave ID matches P09.00.

Once the RS-485 wiring to the VFD (see `docs/wiring.md` section 2.3) is connected,
those source columns should switch to `modbus` and populate on their own — cross-check
the values against GSoft2's live monitor at the same moment to confirm they're correct,
not just present.

If `RS485_DE_RE_Pin` / `RS485_DE_RE_GPIO_Port` don't resolve at build time, the user
label in step 2 wasn't set — go back to the Pinout view and add it to PA8, then
regenerate code (this won't touch your `main.c` edits, CubeMX only touches the parts
outside the `USER CODE` markers).

## Running tests

Use the PC logger in `tools/logger/` (see its README). It handles the commands below
and saves each test to its own CSV. Type `TEST` and the board asks for the weight in
kg. That starts a numbered test, and every row logged until `STOP` carries that test's
`test_id` in the last CSV column (0 = no test running). `STOP` prints the test's
steady-state averages, counting only rows logged after `RPM` was entered. Typed
commands need the **USART2 global interrupt** enabled in the `.ioc` (NVIC Settings).

## Status LED (no serial needed)

The green user LED (LD2) shows what the firmware is doing:

| LD2 | Meaning |
|---|---|
| Slow blink (1 s) | Running, and the VFD is answering Modbus reads |
| Fast blink (0.1 s) | Running, but the VFD isn't answering. Check RS-485 wiring, VFD power, slave ID |
| Solid on | Stopped in `Error_Handler` (a peripheral failed to start) |
| On for ~2 s after reset | Normal: the startup `VFDCHECK` is running |
| Off / never lights | New firmware isn't running. Check that the right build was flashed |

## Live data in the STM32CubeIDE console (no serial terminal needed)

Everything printed to the serial port is also sent over the debugger's SWO trace pin
(PB3, connected to the ST-LINK by default on the Nucleo-F401RE). It shows up in
STM32CubeIDE's **SWV ITM Data Console**. This view only displays output: commands
like `VFDCHECK` still need a serial terminal. Instead, press the Nucleo's reset
button to re-run the startup `VFDCHECK`. Raw Modbus hex dumps are off by default.

One-time setup:
1. **Run → Debug Configurations…** → your project → **Debugger** tab → under
   *Serial Wire Viewer (SWV)* tick **Enable**, set **Core Clock = 16.0 MHz** (this
   firmware runs on the 16 MHz HSI with no PLL) → Apply.
2. Start debugging (bug icon). When it stops at `main()`:
   **Window → Show View → SWV → SWV ITM Data Console**.
3. In that console click **Configure trace** (the wrench icon) → tick **ITM Stimulus
   Port 0** → OK.
4. Click **Start Trace** (the red record button), *then* press **Resume (F8)**.

The CSV header, the `VFDCHECK` result, and a new data row every 2 s should appear. If
it stays blank, check that Core Clock is 16.0 MHz and that trace was started before
resuming.

Alternatively, add these globals in the debugger's **Live Expressions** view (Window
→ Show View → Live Expressions) to watch values update while the program runs:
`liveFreqHz`, `liveCurrentA`, `livePotSetpointHz`, `liveModbusOkCount`,
`liveModbusFailCount`. `liveModbusOkCount` climbing while `liveModbusFailCount` stays
at 0 confirms the board is reading the VFD.
