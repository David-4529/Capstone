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
Register is `0x2104`, format `XXX.X A` (÷10 scaling — different from frequency's ÷100).
Cross-checked against two independent sources citing the GS20/GS20X manual's register
table (direct PDF fetch was blocked by network egress rules in this environment; the
manual itself is at `cdn.automationdirect.com/static/manuals/gs20m/ch5.pdf` if you want
to eyeball the raw table). `firmware/include/pins.h` and `main.cpp` are updated. The
`CURRENT <amps>` manual-entry command (read off GSoft2's monitor) stays available as a
fallback for whenever Modbus isn't wired up.

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

## 7. Controller changed from ESP32 to STM32 Nucleo-F401RE
Project moved to an STM32F401 Nucleo-64 board, developed in STM32CubeIDE using bare
HAL/LL C (no Arduino framework, so the `ModbusMaster` Arduino library and all of
`firmware/src/main.cpp` need to be rewritten — this isn't a small pin remap). Not yet
started. `docs/wiring.md` sections 2.1/2.2 (ESP32 GPIO numbers) are stale until this is
done; section 2.3 (RS-485 ↔ VFD pinout) is MCU-agnostic and unaffected.
