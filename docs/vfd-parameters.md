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
| P00.20 | Frequency reference source | **2** = external analog input (pot) — see "Potentiometer speed control" below |
| P00.21 | Run command source | 1 = external terminals (FWD/REV pushbuttons) |
| P02.00 | 2-/3-wire control mode | 1 (default — confirm, don't need to change) |
| P03.00 | AI1 function assignment | 1 (default = Frequency Command — confirm, don't need to change) |

⚠ Verify these against your drive's actual GSoft2 parameter list before entering —
this table is taken from documentation for this same motor/drive pairing but hasn't
been re-checked against your specific unit's firmware/parameter revision.

## Potentiometer speed control (current setup)

The motor's speed comes from the hand-turned pot, and the FWD/REV pushbuttons start
and stop it. **The STM32 firmware never commands the motor.** It only *reads* output
frequency/current over Modbus, so it can run alongside pot control without
interfering. If the motor runs by itself, or runs for a few seconds and stops, the
cause is a drive setting, not the firmware.

### Settings in GSoft2 (Parameters table, then write to the drive)

| Parameter | Must be | Why |
|---|---|---|
| P00.20 Frequency reference source | **2** (external analog input) | Speed follows the pot on +10V/AI1/ACM. Any other value (keypad, RS-485/GSoft2) makes the drive ignore the pot. |
| P00.21 Run command source | **1** (external terminals) | Start/stop from the FWD/REV pushbuttons. Keypad or RS-485 here lets GSoft2/keypad start the motor instead. |
| P03.00 AI1 function | **1** (frequency command) | Makes AI1 the speed input. |
| P01.00 Max output frequency | 60.00 Hz | Pot full-scale (10 V) = this frequency. |
| Accel / Decel time (P01 group) | ~2 s | Pot changes take effect quickly instead of a slow ramp. |

After writing the parameters, cycle drive power (or confirm in GSoft2 that the values
stuck). Then flash the updated firmware and open the serial terminal. It runs a
**`VFDCHECK`** at startup, which reads P00.20, P00.21, and P03.00 back over Modbus,
prints `OK` / `WRONG` for each, and shows the live pot setpoint (frequency command,
register 0x2102). Type `VFDCHECK` anytime to re-run it. The check is read-only, so it
never changes a setting.

### Test procedure

1. Turn the pot fully counter-clockwise (0 Hz).
2. Press FWD. The motor should stay still or barely turn.
3. Slowly turn the pot up. Speed should rise smoothly and hold wherever you leave it.
4. Release FWD (maintained contact: switch it off) to stop.

To hit an exact calibration setpoint (10.00 / 20.00 / 30.00 Hz), turn the pot while
watching `freq_hz` in the CSV stream or GSoft2's Output Frequency monitor. Wait for
the value to stop changing before you log, so the reading is at steady state.

### Troubleshooting: "motor runs automatically for a few seconds"

- **P00.20 / P00.21 not at 2 / 1.** If either one points to the keypad or RS-485, a
  GSoft2 or keypad setpoint and run command take over. Run `VFDCHECK` to see which.
- **GSoft2 Run/JOG test button used.** A test run from GSoft2 or a JOG command runs
  briefly at a fixed preset frequency. Use the pushbuttons instead.
- **Communication timeout.** If run/frequency was coming from RS-485 and the PC
  stopped talking, the drive stops after the P09 comm-timeout delay. Setting P00.20/
  P00.21 back to 2/1 removes this dependency.
- **Pot does nothing.** Check that the wiring matches `docs/wiring.md` §1 (Green →
  +10V, White → AI1, Red → ACM). Check that AI1 is in voltage mode (0–10 V) in the
  P03 group. Look at the `frequency command` line from `VFDCHECK` while turning the
  pot: it should change.
- **DI3 (preset-speed select) wired up later.** An active preset-speed input
  overrides the pot. Leave DI3 unassigned while using pot control.

If you'd rather type exact setpoints later instead of using the pot, switch P00.20 to
the drive's keypad/digital source. P00.21 can stay at 1, so the pushbuttons still
start and stop the motor.

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
