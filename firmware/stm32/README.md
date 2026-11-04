# STM32 Nucleo-F401RE Firmware

Single self-contained file: `Core/Src/main.c`. No other custom files needed — the
Modbus RTU logic is written directly into it, not a separate module.

## Two separate pieces of software — not to be confused

This project's software side splits cleanly into two unrelated tools:

- **GSoft2** (AutomationDirect's VFD configuration utility, Windows, direct USB to
  the drive) is how every drive parameter is set and checked — the P00/P01/P03/P05
  group settings in `docs/vfd-parameters.md` (frequency/run source, AI1 function,
  the P01.10 safety cap, motor nameplate values) are all entered here, not in
  firmware. GSoft2 also has a live monitor used throughout this project to
  cross-check Modbus-read values (e.g. confirming raw register 69 = 0.69 A).
  GSoft2 cannot reach the STM32 — it only ever talks to the VFD directly over USB,
  which is exactly why this project added its own RS-485/MAX485 link instead
  (Section III-D of the paper).
- **STM32CubeIDE** (ST's Eclipse-based IDE, bundling CubeMX) is where this
  project's own code is written, built, and flashed — `Core/Src/main.c` and the
  peripheral setup below. It has no knowledge of VFD parameters; it only reads
  them back over Modbus to display/verify them (the `VFDCHECK` command), it never
  writes them.

In short: **GSoft2 configures the drive, CubeIDE builds the controller that talks
to it** — two independent tools on two independent links (USB vs. RS-485) to the
same physical VFD.

## Starting a fresh CubeMX project

1. STM32CubeIDE → **File → New → STM32 Project → Board Selector** tab → search
   **NUCLEO-F401RE** → select it → Next → name it → Finish. Click **Yes** when asked
   to initialize peripherals with their default mode.
2. Pinout view:
   - Click **PA9** → `USART1_TX`. Click **PA10** → `USART1_RX`. (RS-485 link to the VFD.)
   - Click **PA8** → `GPIO_Output` → right-click → **Enter User Label** → `RS485_DE_RE`.
   - Click **PA5** → `GPIO_Output` (this is Nucleo's onboard green LED, LD2 — no
     wiring needed, it's already on the board). A user label isn't required; the
     firmware references it directly as `GPIOA`/`GPIO_PIN_5`.
   - **Connectivity → USART1**: 9600 baud, 8 bits, **no parity, 2 stop bits** (8N2 —
     the VFD's P09.04 has no 8N1 option for RTU mode).
   - **Connectivity → USART2**: confirm it's enabled (Mode ≠ Disable) — this drives
     the ST-LINK virtual COM port used for the serial terminal. If it's not enabled,
     set PA2 → `USART2_TX`, PA3 → `USART2_RX` first.
   - **System Core → NVIC**: check **USART2 global interrupt** (needed to receive
     typed commands without blocking the main loop).
   - **Trace and Debug → SYS**: set Debug to **Serial Wire** with **Trace Asynchronous
     Sw** enabled, so the SWV ITM Data Console in CubeIDE can mirror everything
     `UartPrint()` sends, with no serial terminal needed during debugging.
3. **Project → Generate Code.**
4. Open the generated `Core/Src/main.c`, select all (Ctrl+A), and replace it entirely
   with the contents of this repo's `firmware/stm32/Core/Src/main.c`.
5. Build and flash (Run button — the Nucleo's onboard ST-LINK handles this, no
   separate programmer needed).

## Verifying it works

Open a serial terminal to the Nucleo's ST-LINK VCP port at **115200 baud**. You
should see the CSV header lines, then a new row every 250 ms, and the onboard green
LED (LD2) should be blinking — slowly (1 s) once the VFD is answering Modbus reads,
rapidly (0.1 s) if it isn't (check the RS-485 wiring), solid on only if the firmware
has hit `Error_Handler`.

Send `VFDCHECK` to re-run the startup self-test, which reads back P00.20/P00.21/
P03.00/P00.22/P01.10/P01.12/P01.13 over Modbus and reports PASS/FAIL against the
expected pot/pushbutton-control configuration — this never writes to the drive, it
only flags what to go fix in GSoft2.

For an actual test: send `TEST`, enter the weight in kg when prompted, run the lift,
type `RPM <value>` with the shaft RPM read off the SERVO display, then send `STOP` —
this prints a one-line summary (steady-row count, averaged frequency/current/RPM/slip)
and every row logged during that window carries the test's own `test_id` (0 outside
any test). Typing `FREQ 10`, `RPM 1650`, `CURRENT 1.2`, or `LOAD 0.5` any time updates
that field manually — check for `manual` in the `freq_source`/`current_source` columns
to confirm the command was received.

Once the RS-485 wiring to the VFD (see `docs/wiring.md` section 2.3) is connected,
those source columns should switch to `modbus` and populate on their own — cross-check
the values against GSoft2's live monitor at the same moment to confirm they're correct,
not just present.

If `RS485_DE_RE_Pin` / `RS485_DE_RE_GPIO_Port` don't resolve at build time, the user
label in step 2 wasn't set — go back to the Pinout view and add it to PA8, then
regenerate code (this won't touch your `main.c` edits, CubeMX only touches the parts
outside the `USER CODE` markers).

## HMI

There is no onboard HMI hardware (LCD/keypad) in this project — descoped, not
implemented. The PC-side logger (`tools/logger/vfd_logger.py`) plus the STM32's own
serial output over USART2 is the actual, in-use interface: set `LOAD`/`RPM`/`FREQ`/
`CURRENT` via the same serial commands described above.
