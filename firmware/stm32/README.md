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
