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
| FLA % (informational only) | 36% = (1.8A ÷ 5.0A) × 100 — the lab reference material enters this as a %, but your GSoft2 build takes P05.01 directly in amps, so use 1.8A below, not this percentage |

## Parameters to set

| Parameter | Description | Value |
|---|---|---|
| P01.01 | Motor base/rated frequency | 60.00 Hz |
| P01.02 | Motor rated voltage | 208.0 V |
| P00.16 | Load selection | 0 (Variable Torque) |
| P05.01 | Motor Full-Load Amps | **1.8 A** (motor nameplate FLA, delta winding — enter directly in amps; confirmed your GSoft2 shows this parameter in A, not %) |
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

## Actual settings (confirmed from GSoft2 export, `VFDParameters.xlsx`)

This is the real parameter dump read back off the drive, not the lab-manual-derived
table above. Where the two disagree, this section is the one that reflects the
physical unit.

| Pr. No | Description | Data (set value) | Default | Min | Max |
|---|---|---|---|---|---|
| 00.16 | Duty Selection | 0 | 1 | 0 | 1 |
| 00.20 | Source of FREQ | 2 (external analog/pot) | 0 | 0 | 9 |
| 00.21 | Source of OPER | 1 (external terminals) | 0 | 0 | 5 |
| 01.02 | MAX Out-VOLT 1 | 208.0 V | 220.0 | 0.0 | 255.0 |
| **01.10** | **Upper Bound FREQ** | **10.00 Hz** | 599.00 | 0.00 | 599.00 |
| 01.12 | 1st ACC Time | 1.50 sec | 10.00 | 0.00 | 600.00 |
| 01.13 | 1st DEC Time | 1.50 sec | 10.00 | 0.00 | 600.00 |
| 05.01 | Motor1 Rated A | 1.80 A | 0.00 | 0.50 | 6.00 |
| 05.02 | Motor1 Rated P | 0.73 kW | 0.00 | 0.00 | 655.35 |
| 05.03 | Motor1 Rated S | 1650 rpm | 1710 | 0 | 65535 |
| 05.05 | Motor1 No-Load | 0.72 A | 0.00 | 0.00 | 1.80 |
| 05.06 | Motor1 Rs | 5.156 ohm | 0.000 | 0.000 | 65.535 |
| 06.03 | OCA Level | 120% | 180 | 0 | 150 |
| 06.04 | OCN Level | 120% | 180 | 0 | 150 |
| 06.15 | OH Warn Level | 100.0 °C | 105.0 | 0.0 | 110.0 |
| 09.04 | MODBUS Protocol | 13 | 15 | 1 | 17 |
| 10.52 | INJ Magnitude | 15.0 | 30.0 | 0.0 | 100.0 |

**P01.10 is the cable-length-driven safety cap**, not P01.00. P01.00 (max output
frequency, 60.00 Hz) is left at the motor's rating; P01.10 separately clamps the
*commanded* frequency to 10.00 Hz regardless of pot position. With the confirmed
20T/60T (3:1) belt reduction between motor and drum, this caps drum speed at 100 RPM,
giving roughly 19.6 s of the 64 in usable cable travel before running out — a
comfortable margin for logging a settled plateau. See `docs/open-items.md` item 9 for
the underlying cable-travel math.

**Resolved:** 0.37 kW (the motor's own nameplate, SE2673-1K7) is correct. P05.02 on
the drive currently reads 0.73 kW, exactly double - this is a wrong value entered on
the physical drive, not a real property of the motor, and should be corrected in
GSoft2 (set P05.02 to 0.73 → 0.37 kW) before the next test session.

P09.04 = 13 is inferred to correspond to 9600 baud / 8N2 (no parity, 2 stop bits),
since that is the configuration already confirmed working end-to-end against real
hardware (see `docs/open-items.md` item 3) — not independently decoded from the GS20
manual's P09.04 code table, so treat this mapping as a reasonable inference rather
than a verified fact if it ever needs to be cited precisely.

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
