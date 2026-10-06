# Open Items

These block a fully-working, end-to-end firmware build. Each has a stub in
`firmware/src/main.cpp` marked `TODO(open-item-N)` so the code compiles and logs
partial data today, and can be completed in place once the item is resolved.

## 1. Motor shaft diameter
Needed to finalize the motor-side GT2 20T pulley bore. Not confirmed via coupling part
number, tooth count, Lucas Nülle frame class, or motor part number. **Next step:**
direct caliper measurement, or a Lucas Nülle support request.
Blocks mechanical BOM only — not firmware.

## 2. VFD output current Modbus register address — RESOLVED
Register is `0x2104`, scaling is **÷100** (same as frequency). The manual-summary-derived
guess of ÷10 was wrong by 10x — corrected after real hardware testing: raw register
value 69 matched GSoft2's own live monitor showing "Output Current = 0.69 A" exactly.
Lesson learned: trust real hardware cross-checks over manual/documentation summaries
when the two disagree. `firmware/include/pins.h`, the STM32 `main.c`, and the ESP32
`main.cpp` are all updated. The `CURRENT <amps>` manual-entry command (read off
GSoft2's monitor) stays available as a fallback for whenever Modbus isn't wired up.

## 3. RS-485/Modbus terminal labels on the VFD control terminal block — RESOLVED
This drive exposes RS-485 both via terminal-block `SG+`/`SG-` and via a front/side
**RJ45 jack** that shares the same signals: pin 5 = SG+ (A), pin 4 = SG- (B), pins 3/7 =
SGND, pins 1/2/6 reserved, pin 8 = +10V for an optional accessory keypad (don't connect
that one). Full pinout and wiring is now in `docs/wiring.md` section 2.3. Also resolved:
the drive's P09.04 has no 8N1 framing option for RTU mode, only 8N2/8E1/8O1 — use 8N2 if
running without parity, on both the drive and the MCU UART.
The `FREQ <hz>` manual-entry command stays available as a fallback for whenever Modbus
isn't wired up.

## 4. Lucas Nülle SERVO Machine Test System's data interface
Electrical interface (analog voltage / digital pulse / serial) still not confirmed —
its manual or physical outputs need to be checked before the ESP32 can read it
automatically.

**Interim workaround in place:** the SERVO system's own display shows a readable
numeric RPM value, so `firmware/src/main.cpp` accepts it via a `RPM <value>` serial
command (same pattern as the crane-scale `LOAD <kg>` command below) — the operator
reads the display by eye and types it in. This is enough to collect real slip data
(including the no-load baseline sweep) today. **Next step, when convenient:** confirm
the interface for automatic logging; until then this is not a hard blocker.

## 5. Crane scale (QWORK) data interface
No confirmed data output port. Treated as **manual entry**: the known/applied test
load is typed over USB serial before each test point (see `firmware/src/main.cpp`,
`handleSerialCommands()`). If automatic logging is wanted later, this specific scale
would need to be replaced with one that has a serial/Bluetooth output.

## 6. AC/mains wiring color convention
User has not yet specified US/NEC vs. IEC color coding for the VFD's 3-phase input
wiring. `docs/wiring.md` marks all AC/mains conductor colors `TBD` pending that input;
everything else in the wiring doc (MAX485/sensor low-voltage side) is documented now
since it doesn't depend on that choice.

## 7. Controller changed from ESP32 to STM32 Nucleo-F401RE — RESOLVED
Project moved to an STM32F401 Nucleo-64 board, developed in STM32CubeIDE using bare
HAL/LL C (no Arduino framework). Firmware is written (`firmware/stm32/Core/Src/main.c`,
self-contained, no separate Modbus library needed) and **confirmed working end-to-end
on real hardware**: RS-485 wired to the VFD via the MAX485 module, live Modbus reads of
both Output Frequency and Output Current succeed with correct CRCs, and the CSV output
matches GSoft2's own readings. `docs/wiring.md` sections 2.1/2.2 (ESP32 GPIO numbers)
are still stale/superseded by the STM32 pinout (PA8/PA9/PA10, see `firmware/stm32/README.md`);
section 2.3 (RS-485 ↔ VFD pinout) was MCU-agnostic and needed no changes.

## 8. Manual pot speed control with speed cap — RESOLVED
Motor speed now comes from the hand pot (P00.20 = 2) and start/stop from the FWD/REV
pushbuttons (P00.21 = 1). Speed is capped at 10 Hz (P01.10) with 1.5 s accel/decel
and ramp-to-stop, after an uncapped pot run exhausted the cable travel. **Confirmed on
hardware:** the startup `VFDCHECK` read all 8 registers back correctly over Modbus.
With the pot at 11.88 Hz, output held at exactly 10.00 Hz, so the cap works. Below
the cap, output followed the pot (ramped 0 → 9.4 Hz), and no-load current was about
0.65 A. Live data is visible in STM32CubeIDE's SWV ITM Data Console (SYS Debug =
Trace Asynchronous Sw, ITM port 0 enabled).
Still recommended: an end-of-travel limit switch on the spare DI4 pair as a hardware
stop.

## 9. Future enhancements (not started)
Ideas for after real calibration/validation data is collected:
- **Local HMI/keypad** on the rig itself (standalone display + button input), so the
  station doesn't need a laptop tethered via serial to run a test or see live values.
- **Simple web dashboard** showing live/logged load readings and the model's estimated
  weight (slip-based and current-based side by side), likely reading off the same CSV
  stream the STM32 already produces. Would need a way to get that serial stream onto
  something that can host a web page - options to consider later: a small script on
  the same PC that already runs the serial monitor, or adding networking to the
  controller itself (out of scope for the STM32 Nucleo without an added Ethernet/WiFi
  module - worth deciding whether that's a hardware addition or handled entirely on
  the PC side).
