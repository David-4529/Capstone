# Load-estimation model

`fit_model.py` turns the calibration data under `results/*/analysis/groups.csv`
into a load-prediction model and regenerates two files in this folder:

- `load_model.json` - coefficients + fit stats, for reference / other tooling.
- `load_model.h` - the same thing as a small C lookup table, `#include`d directly
  by `firmware/stm32/Core/Src/main.c` to drive the live HMI display.

## Why current, not slip

Slip (sync RPM − shaft RPM) is the more load-sensitive signal and is what
`docs/results-log.md` uses to characterize this rig, but shaft RPM here is read by
eye off the ActiveServo display and typed in by hand - there's no shaft encoder, so
it isn't available once the rig is running on its own without a laptop. Output
current *is* already read automatically over Modbus every logger cycle, and across
all four calibration objects (no-load, 0.32 kg, 0.58 kg, 1.46 kg) it rises
monotonically with load at every test frequency in the up (lifting) direction. So
the standalone model fits `load_kg = slope * current_a + intercept`, one line per
test frequency (1.39 / 2.44 / 4.85 / 6.36 Hz - the nearest one is used for any
in-between operating frequency).

Current fits with r² > 0.98 at every frequency, not as tight as slip but tight
enough for a live estimate, and it needs no manual input.

## Known limitation: up-direction only

Down-direction (lowering) current does not track load at all - this is an
open-loop V/Hz drive with no dynamic braking resistor, so a descending load
overhauls the motor regardless of weight (see `docs/results-log.md`). The display
shows a predicted weight assuming the rig is lifting; there is currently no
automatic way to tell the firmware which direction is active (the FWD/REV buttons
wire straight to the VFD, not through the MCU), so the number should be ignored
while lowering. See `docs/open-items.md` for ideas on sensing direction.

## Regenerating after new calibration data

```
python tools/model/fit_model.py
```

Re-run this any time a new object is logged and backfilled into `results/`, then
copy/rebuild `firmware/stm32` so the new coefficients get flashed.

## Quick prediction check

```
python tools/model/fit_model.py --predict 4.85 0.90
```
