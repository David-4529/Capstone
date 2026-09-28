# Open Items

These block a fully-working, end-to-end firmware build. Each has a stub in
`firmware/src/main.cpp` marked `TODO(open-item-N)` so the code compiles and logs
partial data today, and can be completed in place once the item is resolved.

## 1. Motor shaft diameter
Needed to finalize the motor-side GT2 20T pulley bore. Not confirmed via coupling part
number, tooth count, Lucas Nülle frame class, or motor part number. **Next step:**
direct caliper measurement, or a Lucas Nülle support request.
Blocks mechanical BOM only — not firmware.

## 2. VFD output current Modbus register address
`docs/wiring.md` and `firmware/src/main.cpp` assume this lives in the same
Status Monitor register block (0x21xx range) as Output Frequency (confirmed at
`0x2103`), per the GS20/GS20X AC Drive User Manual, Chapter 5 (Serial
Communications). The exact address has not been located yet.

**Interim workaround in place:** a working USB connection to GSoft2 already exists,
and GSoft2's live monitor can display Output Current in real time (same monitor
function used in the lab manual's Section 8.5). `firmware/src/main.cpp` accepts this
via a `CURRENT <amps>` serial command, read off the GSoft2 monitor by eye. **Next
step, when convenient:** pull the register address from the Ch.5 table and set
`REG_OUTPUT_CURRENT` in `firmware/include/pins.h` for automatic logging; until then
this is not a hard blocker.

## 3. RS-485/Modbus terminal labels on the VFD control terminal block
The lab reference material for this drive family confirms the drive has a front-panel
USB port for GSoft2 configuration (which uses RS-485 internally), and gives the main
power/motor/control terminal labels (see README), but does not cover the terminal-block
labels for external RS-485 (commonly `SG-`/`SG+`, sometimes an RJ12 jack requiring an
adapter). Wiring doc marks this `TBD`.

**Interim workaround in place:** since GSoft2 is already connected over USB, its live
monitor can also display Output Frequency in real time, so the MAX485/RS-485 wiring to
the drive isn't actually needed yet to collect data. `firmware/src/main.cpp` accepts a
manually-read frequency via a `FREQ <hz>` serial command, and tags each logged row with
`freq_source` = `modbus` or `manual` so it's clear which path produced it. **Next step,
when convenient:** confirm the RS-485 terminal labels from the GS20/GS20X manual, Ch.5,
before wiring the MAX485 module to the drive for automatic logging; until then this is
not a hard blocker.

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
everything else in the wiring doc (ESP32/MAX485/sensor low-voltage side) is documented
now since it doesn't depend on that choice.
