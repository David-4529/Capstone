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

## 6. AC/mains wiring color convention — RESOLVED
Confirmed: R/L1=black, S/L2=grey, T/L3=brown on the incoming 3-phase side, carried
through to U2/V2/W2 on the motor side. Documented in `docs/wiring.md` section 1b.
Everything else in the wiring doc (MAX485/sensor low-voltage side) is documented too
since it doesn't depend on that choice.

## 7. Controller changed from ESP32 to STM32 Nucleo-F401RE — RESOLVED
Project moved to an STM32F401 Nucleo-64 board, developed in STM32CubeIDE using bare
HAL/LL C (no Arduino framework). Firmware is written (`firmware/stm32/Core/Src/main.c`,
self-contained, no separate Modbus library needed) and **confirmed working end-to-end
on real hardware**: RS-485 wired to the VFD via the MAX485 module, live Modbus reads of
both Output Frequency and Output Current succeed with correct CRCs, and the CSV output
matches GSoft2's own readings. `docs/wiring.md` sections 2.1/2.2 have since been updated
to the real STM32 pinout (PA8/PA9/PA10, see `firmware/stm32/README.md`); section 2.3
(RS-485 ↔ VFD pinout) was MCU-agnostic and needed no changes.

## 9. No-load baseline: missing down-direction run at 4.85 Hz
`results/2026-10-05/analysis/groups.csv` (the no-load baseline session) has no
down-direction row at 4.85 Hz, searched across every `results/*/analysis/groups.csv`
and `runs.csv` in the repo. Table VI of the paper nonetheless reports values for it
(slip 0.030, 0.761 A) that don't trace back to any logged session found. Found while
regenerating Fig. 4 from real data in `tools/matlab/generate_figures.m` — the figure
honestly shows this as a gap rather than reproducing those two numbers. Either locate
the original source run for that Table VI entry, or re-run a 4.85 Hz no-load
down-direction test so Table VI and Fig. 4 agree.

## 10. MATLAB/Octave figure script and diagrams/tables export — DONE
`tools/matlab/generate_figures.m` regenerates every figure used in the paper and both
decks (system block diagram, RS-485/MAX485 wiring, no-load baseline, calibration fit,
blind validation, LOOCV comparison) directly from `results/*/analysis/groups.csv` where
possible. A standalone, fully-editable `Sensorless_Load_Estimation_Diagrams_Tables_Export`
deck (same rev number as the paper) carries every figure and table as native PowerPoint
content for reuse outside the paper itself.

## 8. Future enhancements (not started)
Ideas for after real calibration/validation data is collected. A local HMI/keypad was
considered and descoped (decision made, not a future item) - the PC logger plus the
STM32's serial output is the project's final, only interface.
- **Simple web dashboard** showing live/logged load readings and the model's estimated
  weight (slip-based and current-based side by side), likely reading off the same CSV
  stream the STM32 already produces. Would need a way to get that serial stream onto
  something that can host a web page - options to consider later: a small script on
  the same PC that already runs the serial monitor, or adding networking to the
  controller itself (out of scope for the STM32 Nucleo without an added Ethernet/WiFi
  module - worth deciding whether that's a hardware addition or handled entirely on
  the PC side).
