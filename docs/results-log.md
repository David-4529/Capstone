# Results log

Hand-entered RPM (read off the ActiveServo display) matched to logger runs.
Slip = (sync RPM − shaft RPM) / sync RPM. Slip RPM = sync RPM − shaft RPM.

## 2026-10-05: no-load baseline (hook + scale only, baseline entered as 0 kg)

| Session | Run | Dir | Freq (Hz) | Sync RPM | Current (A) | Shaft RPM (display) | Slip RPM | Slip |
|---|---|---|---|---|---|---|---|---|
| 2129 | 1 | up | 1.39 | 41.8 | 0.476 | ~30.5 (29–32) | 11.3 | 0.27 |
| 2129 | 2 | down | 1.39 | 41.8 | 0.486 | ~38 | 3.8 | 0.09 |
| 2133 | 1 | up* | 2.45 | 73.4 | 0.463 | ~60 (59–61) | 13.4 | 0.18 |
| 2133 | 2 | down* | 2.44 | 73.3 | 0.434 | ~69.5 (69–70) | 3.8 | 0.05 |
| 2133 | 4 | up* | 2.44 | 73.3 | 0.469 | ~60 (59–61) | 13.3 | 0.18 |
| 2136 | 2 | up? | 4.85 | 145.4 | 0.814 | ~137.5 (137–138) | 7.9 | 0.054 |
| 2136 | — | down | ~4.85 | ~145.4 | — | ~141 | ~4.4 | ~0.030 |

The 2136 session logged only one full run (run 2, 9.7 s). The down reading has no
matching logged run, so its sync RPM is assumed equal to run 2's.

\* Direction inferred from run order and current (up runs draw more). To be confirmed.

Observations:
- Slip is strongly direction-dependent even with only the hook. Going up the motor
  lifts the hook and overcomes friction; going down gravity assists.
- Down slip RPM is steady across speed (3.8, 3.8, ~4.4 RPM at 1.4 / 2.4 / 4.9 Hz).
  Up slip RPM is not (~11, ~13, ~8 RPM). The ideal V/f "constant slip RPM per torque"
  only holds roughly. At very low frequency the stator resistance drop weakens the
  flux, so the motor slips more for the same torque. **Calibrate at one fixed
  frequency** rather than mixing speeds.
- No-load current rises with frequency (~0.47 A at 1.4–2.4 Hz, ~0.81 A at 4.9 Hz),
  because the drive applies more voltage (V/f) and so more magnetizing current.
- Current barely moves with direction (0.43–0.49 A). At no load it's dominated by
  magnetizing current. This supports slip over current as the load signal.
- Friction/gravity split (slip RPM): average of up and down ≈ friction (~8 RPM),
  half the difference ≈ the hook's weight (~4–5 RPM).
- The display jitters ±1–1.5 RPM. At 2.4 Hz that's about ±2% slip, at 1.4 Hz about ±3.6%.
