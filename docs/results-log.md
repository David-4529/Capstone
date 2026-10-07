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

| 2139 | 1 | up* | 6.36 | 190.7 | 0.700 | ~180 | 10.7 | 0.056 |
| 2139 | 2 | down* | 6.36 | 190.7 | 0.660 | ~186 | 4.7 | 0.025 |
| 2139 | 3 | up* | 6.35 | 190.5 | 0.692 | ~180 | 10.5 | 0.055 |
| 2139 | 4 | down* | 6.36 | 190.7 | 0.641 | ~186 | 4.7 | 0.025 |
| 2139 | 5 | up* | 6.35 | 190.6 | 0.720 | ~180 | 10.6 | 0.056 |

2139: one RPM reading per direction was given for the whole session. Up/down were
inferred from the alternating current (odd runs higher). Up current averages
0.704 A, down 0.651 A.

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

## 2026-10-07: first loaded test (1.46 kg metal box, baseline correctly entered as 0 kg)

Up direction (lifting) ran at 1.39 Hz, same as the no-load baseline. The down direction
(lowering) was run at a different pot setting, ~1.52 Hz — hand pot control, not matched
to the up frequency on purpose.

**Up**: shaft RPM read as 18–21 (same range as the unloaded hook in the first,
mislabeled attempt at this weight — see note below). Logged as a single ~39 s
continuous run; no separate summary row captured for it this session.

**Down**: four runs held long enough with no stop/restart to trust (session
`2026-10-07_1903`, see `results/2026-10-07/`). Shaft RPM read as 46–48 for all four.

| Run | Run time (s) | Freq (Hz) | Sync RPM | Current (A) | Slip RPM (46–48 RPM) | Slip |
|---|---|---|---|---|---|---|
| 8 | 17.4 | 1.51 | 45.2 | 0.570 | −0.8 to −2.8 | −0.018 to −0.062 |
| 11 | 27.1 | 1.52 | 45.7 | 0.610 | −0.3 to −2.3 | −0.007 to −0.050 |
| 15 | 10.5 | 1.53 | 45.8 | 0.470 | −0.2 to −2.2 | −0.004 to −0.048 |
| 16 | 22.2 | 1.52 | 45.7 | 0.573 | −0.3 to −2.3 | −0.007 to −0.050 |

Runs 11 and 16 (27.1 s and 22.2 s) were the longest, cleanest holds; 8 and 15 were also
unbroken but shorter. The other down-direction runs that session (9, 10, 12–14, 17, 18)
were 1–3 s stutters from stopping to help the descent along and are excluded.

**Key finding — slip is negative on every down run.** The measured shaft speed
(46–48 RPM) meets or exceeds the synchronous speed implied by the output frequency
(45.2–45.8 RPM) in every case. This means the descending load is overhauling the motor,
not being speed-controlled by it: with this drive's open-loop V/Hz control and no
dynamic braking resistor, there is nothing to absorb the regenerative energy of a load
that wants to fall faster than the commanded frequency, so during descent the frequency
reading doesn't reflect real speed control — gravity sets the pace. This is consistent
with the free-fall-on-stop behavior already seen in the first (unloaded hook) session.

**Implication for the load model**: up-direction slip remains a valid motoring-condition
signal and is usable for the slip-vs-load calibration. Down-direction data under load is
not comparable to it (different physical regime — overhauling vs. motoring) and should
be reported as a separate limitation/finding, not combined into the same calibration
curve as the up-direction points.

Note: in this project's very first weighted attempt (same 1.46 kg box, earlier that same
day), the box's weight was entered as the *baseline* rather than the *load*, and no RPM
was typed into the logger for any run — see the session's own commit history. This
2026-10-07_1903 session repeats it correctly (load_kg=1.46, baseline_kg=0) and is the
one to use going forward.
