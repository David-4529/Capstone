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
**Next step:** pull the address from that chapter's register table and set
`REG_OUTPUT_CURRENT` in `firmware/include/pins.h`.

## 3. RS-485/Modbus terminal labels on the VFD control terminal block
The lab reference material for this drive family confirms the drive has a front-panel
USB port for GSoft2 configuration (which uses RS-485 internally), and gives the main
power/motor/control terminal labels (see README), but does not cover the terminal-block
labels for external RS-485 (commonly `SG-`/`SG+`, sometimes an RJ12 jack requiring an
adapter). **Next step:** confirm from the GS20/GS20X manual, Chapter 5, before wiring
the MAX485 module to the drive. Wiring doc marks this `TBD`.

## 4. Lucas Nülle SERVO Machine Test System's data interface
Biggest firmware blocker. The system provides real shaft RPM, but nothing so far
establishes whether the ESP32 reads it as an analog voltage (e.g. 0–10V tachometer
output), a digital pulse train (encoder-style, needs a GPIO interrupt), or a serial link.
`firmware/src/main.cpp` stubs `readActualRpm()` with instructions for whichever it turns
out to be. **Next step:** check the SERVO system's own manual, or inspect its physical
outputs.

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
