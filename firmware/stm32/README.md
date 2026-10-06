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
4. Open the generated `Core/Src/main.c`, select all (Ctrl+A), and replace it entirely
   with the contents of this repo's `firmware/stm32/Core/Src/main.c`.
5. Build and flash (Run button — the Nucleo's onboard ST-LINK handles this, no
   separate programmer needed).

## Verifying it works

Open a serial terminal to the Nucleo's ST-LINK VCP port at **115200 baud**. You
should see the CSV header lines, then a new row every 2 seconds. Typing
`FREQ 10`, `RPM 1650`, `CURRENT 1.2`, or `LOAD 0.5` (Enter after each) updates that
field — check for `manual` in the `freq_source`/`current_source` columns to confirm
the command was received.

Once the RS-485 wiring to the VFD (see `docs/wiring.md` section 2.3) is connected,
those source columns should switch to `modbus` and populate on their own — cross-check
the values against GSoft2's live monitor at the same moment to confirm they're correct,
not just present.

If `RS485_DE_RE_Pin` / `RS485_DE_RE_GPIO_Port` don't resolve at build time, the user
label in step 2 wasn't set — go back to the Pinout view and add it to PA8, then
regenerate code (this won't touch your `main.c` edits, CubeMX only touches the parts
outside the `USER CODE` markers).

## Adding the HMI (16x2 I2C LCD + 4x4 keypad)

Lets you set `LOAD`/`RPM`/`FREQ`/`CURRENT` from the rig itself instead of needing a
laptop tethered over serial. Confirmed working design, not yet flashed/tested on real
hardware as of this writing.

**Pin plan** (all free pins, nothing conflicts with the RS-485/USART2 pins already in use):

| Signal | Chip pin | Board label |
|---|---|---|
| I2C1 SCL (LCD) | PB8 | D15 |
| I2C1 SDA (LCD) | PB9 | D14 |
| Keypad row 1 | PB3 | D3 |
| Keypad row 2 | PB5 | D4 |
| Keypad row 3 | PB4 | D5 |
| Keypad row 4 | PB10 | D6 |
| Keypad col 1 | PC7 | D9 |
| Keypad col 2 | PB6 | D10 |
| Keypad col 3 | PA7 | D11 |
| Keypad col 4 | PA6 | D12 |

**CubeMX steps** (do this before pasting the updated `main.c` — the pin user-labels
below need to exist in the generated `main.h` first):

1. Open `Capstone.ioc` (via STM32CubeMX if that's how your install opens it).
2. Click **PB8** → `I2C1_SCL`. Click **PB9** → `I2C1_SDA`. (Should auto-enable I2C1 in
   Connectivity; leave its settings at default, 100kHz Standard Mode is fine.)
3. For each of these 8 pins, click it → set to `GPIO_Output` (rows) or `GPIO_Input`
   (columns) → right-click → **Enter User Label** → type the exact name shown:
   - **Outputs (rows):** PB3→`KP_R1`, PB5→`KP_R2`, PB4→`KP_R3`, PB10→`KP_R4`
   - **Inputs (columns):** PC7→`KP_C1`, PB6→`KP_C2`, PA7→`KP_C3`, PA6→`KP_C4`
4. For each of the 4 column pins (`KP_C1`-`KP_C4`), also set **GPIO Pull-up/Pull-down**
   to **Pull-up** in that pin's configuration (System Core → GPIO, select the pin).
5. **Project → Generate Code.**
6. Select all in `Core/Src/main.c`, replace with the updated file (same file in this
   repo, now includes the LCD driver, keypad scanner, and HMI state machine).
7. Build and flash.

**Hardware:** a standard 16x2 character LCD with a PCF8574 I2C backpack (I2C address
0x27 — if the display stays blank, try changing `LCD_I2C_ADDR` in `main.c` to
`(0x3F << 1)`, the other common address for these backpacks), and a 4x4 matrix
membrane keypad.

**How it works:** the LCD normally shows live frequency/current (top line) and
whether an actual RPM/load have been entered (bottom line). Press **A** (Load),
**B** (RPM), **C** (Freq), or **D** (Current) to start entering a number for that
field; digits `0`-`9` type it in, `*` is the decimal point, `#` confirms. This
mirrors the `LOAD`/`RPM`/`FREQ`/`CURRENT` serial commands exactly — either input
method updates the same underlying values, and a confirmation line still prints
over USART2 either way.
