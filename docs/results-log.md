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

\* Direction inferred from run order and current (up runs draw more). To be confirmed.

Observations:
- Slip is strongly direction-dependent even with only the hook. Going up the motor
  lifts the hook and overcomes friction; going down gravity assists.
- Slip in **RPM** is about the same at 1.4 Hz and 2.4 Hz for the same direction
  (up ~11–13 RPM, down ~3.8 RPM). That's what scalar V/f theory predicts: slip RPM
  tracks torque roughly independent of frequency. So slip RPM may be the better load
  metric than fractional slip when comparing across speeds.
- Current barely moves with direction (0.43–0.49 A). At no load it's dominated by
  magnetizing current. This supports slip over current as the load signal.
- Friction/gravity split (slip RPM): average of up and down ≈ friction (~8 RPM),
  half the difference ≈ the hook's weight (~4–5 RPM).
- The display jitters ±1–1.5 RPM. At 2.4 Hz that's about ±2% slip, at 1.4 Hz about ±3.6%.
