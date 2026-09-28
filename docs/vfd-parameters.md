# VFD Parameter Settings — DURApulse GS23-21P0

Confirmed from lab reference material for this exact motor/drive pairing (Lucas
Nülle SE2673-1K7 on a DURApulse GS23-21P0). Set these in GSoft2's Parameters table
over the drive's USB connection before running any test.

## Source data (motor nameplate)

| Field | Value |
|---|---|
| Voltage rating | 208V (delta) / 380V (wye) |
| Current rating | 1.8A (delta) / 1.05A (wye) |
| Power rating | 0.37 kW |
| Frequency | 60 Hz |
| Rated (full-load) speed | 1650 RPM |
| Poles | 4 (synchronous speed 1800 RPM @ 60Hz; matches nameplate slip ≈ 8.3%) |
| Winding used | **Delta** (208V) — matches this drive's 230V output class; wye (380V) would exceed it |
| Drive rated output current (VT) | 5.0 A |
| FLA % | 36% = (1.8A ÷ 5.0A) × 100 |

## Parameters to set

| Parameter | Description | Value |
|---|---|---|
| P01.01 | Motor base/rated frequency | 60.00 Hz |
| P01.02 | Motor rated voltage | 208.0 V |
| P00.16 | Load selection | 0 (Variable Torque) |
| P05.01 | Motor FLA, as % of drive rated current | 36% |
| P05.03 | Motor rated speed (from nameplate) | 1650 RPM |
| P05.04 | Number of motor poles | 4 |
| P01.00 | Maximum output frequency | 60.00 Hz |
| P00.20 | Frequency reference source | 2 = external analog input (pot) — see note below |
| P00.21 | Run command source | 1 = external terminals (FWD/REV pushbuttons) |
| P02.00 | 2-/3-wire control mode | 1 (default — confirm, don't need to change) |
| P03.00 | AI1 function assignment | 1 (default = Frequency Command — confirm, don't need to change) |

⚠ Verify these against your drive's actual GSoft2 parameter list before entering —
this table is taken from documentation for this same motor/drive pairing but hasn't
been re-checked against your specific unit's firmware/parameter revision.

## Note on P00.20 for calibration testing

Your test plan needs precise, repeatable frequency setpoints (10/20/30 Hz for the
baseline, more for the weighted tests) — that's hard to hit exactly with a hand-turned
pot. Two options:

- **Keep P00.20 = 2 (analog pot)**: turn the pot while watching GSoft2's live Output
  Frequency monitor, nudging until it reads exactly 10.00 Hz before logging. Works
  with zero changes, just fiddly.
- **Switch P00.20 to the drive's digital/keypad frequency source**: lets you type an
  exact number (10.00, 20.00, 30.00) instead of eyeballing a pot position. This is
  the more precise, more repeatable option for calibration data. I haven't confirmed
  the exact numeric value GS20-series uses for this source (the lab material only
  confirms `2 = external analog input`) — check the P00.20 dropdown in GSoft2, it
  should list something like "Keypad" or "Digital Setting."
  **P00.21 (run command source) can stay at 1** either way, so the physical FWD/REV
  pushbuttons keep working as your start/stop/direction control regardless of where
  frequency comes from — no need to give that up for precise frequency setting.

## Accel/Decel time — check before your first run

Not confirmed from the reference material (only base frequency, max frequency, and
rated voltage are covered in the P01 group above) — but the same P01 group in
GSoft2 will have Accel Time / Decel Time entries, labeled by description even if you
don't know their exact P-number. Worth checking before testing because:
- A long accel time delays reaching your target frequency, eating into your limited
  cable-travel time (see `docs/open-items.md` — safe run duration is still pending
  the drum diameter).
- The project's methodology assumes steady-state operation at each logged point —
  if you're still ramping when you log, the slip reading won't reflect true
  steady-state slip at that frequency.
Aim for a short accel/decel time (a couple seconds) given how low the test
frequencies are (10–30 Hz vs. a 60 Hz rated max).

## Safety (confirmed from lab reference material)

- ⚠ The drive stores lethal DC bus voltage in its capacitors even after being
  powered off and unplugged. Wait at least 5 minutes and verify zero energy with a
  multimeter before touching internal terminals — don't rely on the display going
  dark.
- ⚠ The Safe Torque Off (STO) jumper (factory-installed across `+24V`, `S1`, `S2`)
  must stay intact unless you're deliberately wiring an external safety circuit in
  its place.
- ⚠ Keep hands, tools, and loose clothing clear of the motor shaft/coupling any time
  the drive is powered.
- Do not press Forward and Reverse at the same time — both are maintained contacts
  with no built-in interlock; holding both sends conflicting direction commands.

## Wiring cross-reference

Terminal-level wiring for FWD/REV/pot is already landed and confirmed — see
`docs/wiring.md` section 1. Main power/motor terminals (`R/L1,S/L2,T/L3` /
`U/T1,V/T2,W/T3`) are also documented there.
