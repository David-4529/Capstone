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

## 2026-10-07: pliers (0.58 kg), 1.39 Hz — first test with the rebuilt logger

First session run with the renamed/cleaned-up logger (`tools/logger/vfd_logger.py`):
equipment name, weight, and intended test frequency are now asked at startup and
named the session directly (`2026-10-07_Pliers_baseweight-0.58kg_testedfreq-1.39hz`).
The `weight_kg` and `target_freq_hz` columns came through correctly in the field.

Up read as **24–26 RPM**; down read as **39–40 RPM**. As in every prior
session, "down is tough" — most runs are 0–4-steady-row stutters from stopping to help
the descent. Four runs held long enough to trust (session
`2026-10-07_Pliers_baseweight-0.58kg_testedfreq-1.39hz`, see `results/2026-10-07_pliers/`).

Current didn't separate up from down as cleanly as at heavier loads (0.49–0.56 A across
all four, vs. a clear gap at 1.46 kg) — expected, since current is dominated by
magnetizing current at light load (see the 2026-10-05 no-load baseline notes). Direction
was assigned by current magnitude (higher = up, matching every prior session), picked to
match the established up > down ordering; the resulting slip values are not sensitive to
exactly which pair is which, since all four runs share the same sync RPM (~39.1):

| Run | Dir | Run time (s) | Current (A) | Sync RPM | RPM read | Slip |
|---|---|---|---|---|---|---|
| 5 | up | 27.5 | 0.564 | 39.1 | 24–26 | 0.335–0.386 |
| 11 | up | 34.1 | 0.521 | 39.1 | 24–26 | 0.335–0.386 |
| 2 | down | 33.0 | 0.518 | 39.1 | 39–40 | −0.023–0.003 |
| 9 | down | 11.4 | 0.494 | 39.1 | 39–40 | −0.023–0.003 |

**Consistency check**: down slip is again near zero (−2.3% to +0.3%), matching the
overhauling pattern seen at every frequency and every load so far. Up slip (33.5%–38.6%)
falls neatly between the no-load baseline at this frequency (27%) and the 1.46 kg box
(50%–57%) — exactly where a 0.58 kg load should sit between them, which is a good sign
the current-based direction split is correct despite the weak current separation.

## 2026-10-07: pliers (0.58 kg), 2.44 Hz

Up read as **50–56 RPM**; down read as **71–72 RPM** (session
`2026-10-07_Pliers_baseweight-0.58kg_testedfreq-2.44hz`). Five runs held long enough to
trust, and this time the current pattern itself — not just its average — splits them:
runs 4 and 7 settle into a tight, flat ~0.43–0.44 A plateau after their initial ramp,
while runs 2, 5, and 11 keep fluctuating between 0.44–0.93 A for their whole duration.
A flat low current all the way through fits a load overhauling the motor (gravity doing
the work, little corrective torque needed); sustained fluctuation fits active lifting
against load and friction. That assigns **up = runs 2, 5, 11** and **down = runs 4, 7**,
consistent with every prior session's up > down current ordering.

| Run | Dir | Run time (s) | Current (A) | Sync RPM | RPM read | Slip |
|---|---|---|---|---|---|---|
| 2 | up | 16.2 | 0.526 | 72.8 | 50–56 | 0.231–0.314 |
| 5 | up | 21.5 | 0.538 | 72.9 | 50–56 | 0.231–0.314 |
| 11 | up | 19.0 | 0.537 | 72.8 | 50–56 | 0.231–0.314 |
| 4 | down | 10.4 | 0.445 | 72.9 | 71–72 | 0.012–0.026 |
| 7 | down | 11.2 | 0.456 | 72.9 | 71–72 | 0.012–0.026 |

**Consistency check**: down slip is small and positive (1.2%–2.6%), in the same range as
the 6.36 Hz loaded-box result, not negative this time but still far below up slip. Up
slip (23.1%–31.4%) again falls between the no-load baseline at 2.44 Hz (18%) and the
1.46 kg box at 2.44 Hz (30.8%–32.2%) — exactly where 0.58 kg belongs, confirming the
current-pattern-based direction split.

## 2026-10-07: pliers (0.58 kg), 4.85 Hz

Up read as **136 RPM**; down read as **144 RPM** (session
`2026-10-07_Pliers_baseweight-0.58kg_testedfreq-4.85hz`). Seven runs held long enough to
trust, and this time current splits them cleanly into two tight clusters with a 0.1 A
gap between them — no ambiguity:

| Run | Dir | Run time (s) | Current (A) | Sync RPM | RPM read | Slip |
|---|---|---|---|---|---|---|
| 1 | up | 8.3 | 0.886 | 145.7 | 136 | 0.0659–0.0666 |
| 3 | up | 9.7 | 0.853 | 145.7 | 136 | 0.0659–0.0666 |
| 12 | up | 9.3 | 0.872 | 145.6 | 136 | 0.0659–0.0666 |
| 16 | up | 8.2 | 0.882 | 145.6 | 136 | 0.0659–0.0666 |
| 2 | down | 8.1 | 0.745 | 145.7 | 144 | 0.0110–0.0117 |
| 4 | down | 5.6 | 0.738 | 145.6 | 144 | 0.0110–0.0117 |
| 14 | down | 6.2 | 0.748 | 145.7 | 144 | 0.0110–0.0117 |

**Consistency check**: up slip (6.6%) sits between the no-load baseline at 4.85 Hz
(5.4%) and the 1.46 kg box at 4.85 Hz (10.0%–10.7%), exactly where 0.58 kg belongs. Down
slip also lines up across all three load levels at this frequency, decreasing
monotonically as load increases — no-load 3.0% → 0.58 kg pliers 1.1%–1.2% → 1.46 kg box
≈0% — consistent with heavier loads overhauling the motor more completely on the way
down.

## 2026-10-07: pliers (0.58 kg), 6.36 Hz — last of the four pliers frequencies

Up read as **178 RPM**; down read as **187–188 RPM** (session
`2026-10-07_Pliers_baseweight-0.58kg_testedfreq-6.36hz`). One run needed to be thrown out
for a reason not seen before: run 7 (11.1 s, 33 steady rows — looks clean by the numbers
alone) is actually two different runs stitched into one. Tracing the raw log, its current
holds a tight ~0.74 A plateau, then goes through several seconds of the pot dipping to
0.51–0.58 Hz without the frequency ever crossing the 0.5 Hz "stopped" threshold, then
settles onto a *second*, different ~0.65 A plateau for the rest of the run. Because our
continuity rule only checks whether frequency ever hits zero, this got treated as one
run with a blended, meaningless average (0.724 A) — excluded rather than guessed at.

The remaining five long runs still split cleanly on current, with a 0.072 A gap:

| Run | Dir | Run time (s) | Current (A) | Sync RPM | RPM read | Slip |
|---|---|---|---|---|---|---|
| 2 | up | 7.9 | 0.753 | 190.9 | 178 | 0.0671–0.0676 |
| 9 | up | 7.0 | 0.763 | 190.8 | 178 | 0.0671–0.0676 |
| 11 | up | 4.3 | 0.765 | 190.8 | 178 | 0.0671–0.0676 |
| 5 | down | 4.6 | 0.639 | 190.9 | 187–188 | 0.0152–0.0204 |
| 10 | down | 8.4 | 0.681 | 190.9 | 187–188 | 0.0152–0.0204 |

**Consistency check**: up slip (6.7%) again falls between the no-load baseline (5.5%–
5.6%) and the 1.46 kg box (8.7%–9.3%) at 6.36 Hz. Down slip extends the monotonic-
decrease-with-load pattern first seen at 4.85 Hz to a second frequency: no-load 2.5% →
0.58 kg pliers 1.5%–2.0% → 1.46 kg box 0.4%–0.8%.

This completes the pliers (0.58 kg) data set across all four test frequencies (1.39,
2.44, 4.85, 6.36 Hz), matching the four points already collected for the 1.46 kg box.

## 2026-10-07: plastic housing (0.32 kg) — third and final calibration object

Lightest object tested. The first up run had the hook catching before settling into
clean technique; several of the resulting short/high-current stutters are excluded
below the same way as every other session's stutters.

### 1.39 Hz

Up read as **28 RPM**; down read as **39–42 RPM**. All four runs were long and clean
(31.5–32.3 s for the longer two), but for the first time current gave no separation at
all between directions — all four fluctuate in the same 0.40–0.69 A band with averages
within 0.03 A of each other (0.497–0.529 A), the same breakdown seen with the pliers at
this frequency, just more complete. With no signal to split on, direction was confirmed
directly by the user, who ran a clean alternating up/down/up/down sequence:

| Run | Dir | Run time (s) | Current (A) | Sync RPM | Slip |
|---|---|---|---|---|---|
| 1 | up | 31.5 | 0.529 | 41.6 | 0.327 |
| 3 | up | 32.3 | 0.502 | 41.6 | 0.327 |
| 2 | down | 15.5 | 0.511 | 41.6 | −0.0096 to 0.0625 |
| 4 | down | 18.8 | 0.497 | 41.6 | −0.0096 to 0.0625 |

**Consistency check**: up slip (32.7%) sits just below the pliers (33.5%–38.6%) and
above the no-load baseline (27%) — correct ordering by load. Down slip (≈2.65%
midpoint) continues the monotonic decrease: no-load 9% → plastic housing ≈2.65% →
pliers ≈−1% → box ≈−3.3%.

### 2.44 Hz

Up read as **55–58 RPM**; down read as **70–72 RPM**. This session had a composite-run
problem of its own: run 12 (37.6 s) fluctuates 0.43–0.57 A for ~24 s (matching run 1's
pattern), dips near-stop without crossing the 0.5 Hz threshold, then switches to a tight
flat ~0.43–0.45 A plateau for another ~12 s (matching run 5's pattern) before truly
stopping — excluded. Run 13 genuinely stops at a real zero-frequency row partway through
what might look like one long run, then the motor runs again for ~12 s on a flat
~0.43–0.44 A plateau before a second real stop; that final ~12 s segment was never
captured as its own summary row and is not included here.

| Run | Dir | Run time (s) | Current (A) | Sync RPM | Slip |
|---|---|---|---|---|---|
| 1 | up | 20.3 | 0.514 | 73.1 | 0.207–0.248 |
| 13 | up | 17.0 | 0.512 | 73.1 | 0.207–0.248 |
| 5 | down | 13.6 | 0.454 | 73.1 | 0.015–0.042 |

**Consistency check**: up slip (20.7%–24.8%) sits between the no-load baseline (18%)
and the pliers (23.1%–31.4%). Down slip continues the monotonic trend: no-load 5% →
plastic housing ≈2.9% → pliers ≈1.9% → box ≈1%.

### 4.85 Hz

Up read as **135–136 RPM**; down read as **143 RPM**. Five long runs, splitting cleanly
into two tight current clusters (0.738–0.773 A vs. 0.821–0.854 A — note the up cluster
is higher here, opposite-looking numbers from a glance but consistent since these are
absolute, not relative, values):

| Run | Dir | Run time (s) | Current (A) | Sync RPM | Slip |
|---|---|---|---|---|---|
| 2 | up | 8.5 | 0.854 | 145.4 | 0.0646–0.0715 |
| 12 | up | 10.3 | 0.834 | 145.4 | 0.0646–0.0715 |
| 23 | up | 14.3 | 0.821 | 145.4 | 0.0646–0.0715 |
| 6 | down | 5.6 | 0.742 | 145.4 | 0.0165 |
| 13 | down | 5.6 | 0.773 | 145.4 | 0.0165 |

**Consistency check**: down slip (1.65%) fits neatly into the monotonic decrease:
no-load 3.0% → plastic housing 1.65% → pliers 1.1%–1.2% → box ≈0%. Up slip (6.46%–7.15%)
is essentially tied with the pliers (6.6%) rather than clearly below it — not a concern,
since 0.32 kg and 0.58 kg are the two lightest, closest-together loads tested, and the
ranges overlap within normal RPM-reading jitter.

### 6.36 Hz

Up read as **178 RPM**; down read as **187 RPM**. Six long runs split cleanly into two
clusters:

| Run | Dir | Run time (s) | Current (A) | Sync RPM | Slip |
|---|---|---|---|---|---|
| 10 | up | 6.2 | 0.743 | 191.0 | 0.0681–0.0686 |
| 14 | up | 6.0 | 0.750 | 191.0 | 0.0681–0.0686 |
| 16 | up | 6.8 | 0.739 | 191.1 | 0.0681–0.0686 |
| 8 | down | 8.2 | 0.632 | 191.1 | 0.0215 |
| 13 | down | 5.6 | 0.647 | 191.1 | 0.0215 |
| 15 | down | 5.4 | 0.659 | 191.1 | 0.0215 |

**Consistency check**: down slip (2.15%) continues the monotonic trend: no-load 2.5% →
plastic housing 2.15% → pliers ≈1.75% → box ≈0.6%. Up slip (6.81%–6.86%) is again
essentially tied with the pliers (6.7%), same reasoning as at 4.85 Hz.

This completes all three calibration objects (0.32 kg plastic housing, 0.58 kg pliers,
1.46 kg metal box) plus the no-load baseline across all four test frequencies — twelve
loaded data points per frequency direction, enough to fit a slip-vs-load calibration
curve at each of the four frequencies using up-direction data.

Also backfilled this session: the metal box's 2.44/4.85/6.36 Hz runs (first analyzed
and published above and in its Word doc, but never logged as structured data) are now
in `results/2026-10-07/` alongside the box's original 1.52 Hz down session, so all
three objects read uniformly from `results/*/analysis/groups.csv`.

## 2026-10-08: load-prediction model, fit from all four calibration points

With all three objects (plus no-load) logged at all four frequencies, built
`tools/model/fit_model.py` to fit an actual load-prediction model instead of just
characterizing slip. It reads every `results/*/analysis/groups.csv`, keeps only
up-direction (lifting) rows, and fits `load_kg = slope * current_a + intercept` by
least squares, one line per test frequency.

**Current, not slip, is the input** — a deliberate deviation from the finding above
that slip is the more load-sensitive signal. Shaft RPM (needed for slip) is read by
eye off the ActiveServo display and typed in by hand; there's no shaft encoder, so
it's only available during a manually-babysat calibration run, not once the rig is
meant to run standalone. Output current is already read automatically over Modbus
every cycle. Checking the up-direction current means across all four objects first:

| Freq (Hz) | No-load | 0.32 kg | 0.58 kg | 1.46 kg |
|---|---|---|---|---|
| 1.39 | 0.476 A | 0.516 A | 0.542 A | n/c |
| 2.44 | 0.466 A | 0.513 A | 0.534 A | 0.619 A |
| 4.85 | 0.814 A | 0.836 A | 0.873 A | 0.933 A |
| 6.36 | 0.704 A | 0.744 A | 0.760 A | 0.837 A |

Current rises monotonically with load at every frequency (the 1.39 Hz box point is
the single continuous run with no per-run current breakdown - excluded from that
fit, which still has 3 points). The resulting fits are tight: r² = 0.998, 0.991,
0.983, 0.992 at 1.39/2.44/4.85/6.36 Hz respectively - not as sensitive as slip, but
good enough for a live estimate with a signal that needs no manual input.

**This model is up-direction only.** Down-direction current doesn't track load at
any frequency (see the overhauling finding above - gravity sets the pace, not the
motor), so there is no equivalent down-direction fit, and the model should not be
trusted while lowering.

Coefficients are in `tools/model/load_model.json` / `load_model.h`, and the same
table is pasted into `firmware/stm32/Core/Src/main.c` to drive the new HMI display
(see `firmware/stm32/README.md`, "HMI: live load display") - a 16x2 I2C character
LCD showing live frequency/current and the predicted weight, wired on I2C1
(PB8/PB9, both previously unused). Re-run `tools/model/fit_model.py` and re-paste
the table after logging any new calibration object.
