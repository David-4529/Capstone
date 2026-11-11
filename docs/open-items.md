# Open Items

The data-acquisition and communication platform is built, wired, and validated
end-to-end on real hardware (STM32 Nucleo-F401RE, RS-485/MAX485 to the VFD, PC-side
logger). What remains falls into two groups: things that need an action from David
(Section A) and lower-priority/deferred work (Section B). Every item below is cross-
checked against the current paper (`Sensorless_Load_Estimation_IEEE_Rev8.docx`) and
all four deliverables as of Rev 8.

## A. Needs an action from David

### A1. Rework Fig. 1 (system block diagram) together
Flagged in David's Rev 3 review comments: "This figure needs to be worked, let's work
together on what this should look like." Not auto-edited — the current version (drawn
by `tools/matlab/generate_figures.m`, boxes for PC Logger/STM32/MAX485/VFD/Motor+Load)
stays as a placeholder in the paper until we talk through what it should actually show.

### A3. Source two stock photos for the major deck
Two slides in `Sensorless_Load_Estimation_Deck_Rev6.pptx` still show a dashed
placeholder box instead of a real image, since neither is a photo of this project's
own hardware:
- **"Our Approach" slide:** a generic photo of a dedicated torque transducer / load
  cell, for cost contrast against this project's sensorless approach.
- **"Our Approach" slide:** a photo or logo of PAR Systems' anti-sway crane product
  (the commercial telemetry-first precedent cited as reference [1]).
Any representative stock photo or the vendor's own product image works for both.

### A4. Photo of the VFD's RJ45 jack specifically
Requested in David's Rev 3 review comments. The paper has a wide shot of the whole
wired VFD (Fig. 3) but not a close-up of the RJ45 jack itself where the RS-485 link
taps in. Flagged with a red placeholder in the paper, Section III-C.

### A5. Dedicated VFD and controller photos for the major deck's Hardware Platform table
Surfaced during the Rev 7 deck redesign. `Sensorless_Load_Estimation_Deck_Rev7.pptx`,
"Hardware Platform" slide (Table I), still shows red `[photo needed]` placeholders for
the VFD and STM32 Nucleo-F401RE rows - only the Motor row has a photo. The deck does
have a wide shot of the fully-wired VFD (`vfd_wired.jpg`, used as Fig. 3 elsewhere),
but it's a multi-component shot (VFD + motor + control station), not a clean
per-item identification photo, so it wasn't reused here rather than mislabel it.
Distinct from A4 (RJ45 jack close-up specifically).

## B. Lower priority / deferred

### B1. Lab manual (instructor + student editions)
Explicitly deprioritized — lowest priority in the current backlog. Needs to match the
Summer 2026 VFD lab manual's format and document the STM32 GPIO pinout.

### B2. Simple web dashboard
Idea for after validation data is in a steadier state: show live/logged load readings
and the model's estimated weight (slip-based and current-based side by side), reading
off the same CSV stream the STM32 already produces. Would need a way to get that
serial stream onto something that can host a web page — a small script on the PC
already running the serial monitor, or networking added to the controller itself (out
of scope for the bare Nucleo without an added Ethernet/WiFi module).

## Resolved

- **A2. Higher-frequency validation tests (6.36–10.00 Hz)** — descoped, David's call:
  "don't need this anymore, the torque converter was used to validate the numbers."
  Flagged at the time and recorded here so the distinction isn't lost: the independent
  torque-sensor validation that replaces it (Section V-E, Tables X/XI, Fig. 10)
  technically answers a different question than A2 did — it validates the slip-load
  relationship and the integrity of the original calibration data *at the frequencies
  already tested*, via a second independent sensor, not the deployed current-based
  model's accuracy, and it does not extend validation into the untested 6.36–10.00 Hz
  range. This is a project-scope decision, not a technical substitution, and both the
  paper and both decks say so explicitly.
- **A6. Torque readings from the SERVO Machine Test System** — fully resolved. 12
  sessions recorded (3 objects × 4 frequencies) and parsed directly from the machine's
  raw `.acts` capture files. Along the way, found and corrected a real instrumentation
  bug: the SERVO system's own live slip readout has no link to the VFD's commanded
  frequency and reads ~100–102% regardless of actual rotation; slip is instead
  recomputed from the sensor's raw speed channel using this project's own known
  Ns = 120f/P. Corrected slip agrees with the original Oct. 7 calibration slip to
  within ~1.9 percentage points on average across all 12 points (Table XI).
- **A7. Motor torque characteristics for the paper** — resolved via measured data, not
  a datasheet: confirmed (repo search plus a background web search) that no rated-
  torque, breakdown-torque, or torque-speed curve for the Lucas Nülle SE2673-1K7 is
  published anywhere, including Lucas Nülle's own site. The measured SERVO torque/slip
  data (Table X, Fig. 10) serves as the real, measured torque characterization instead,
  noted explicitly in the paper (Section III-B) rather than left silent.
- **IEEE paper converted to strict black-and-white formatting** — all colored text
  removed (the red STOP banner, red/gray table cells, blue Heading 1/2 styling left
  over from an earlier color redesign); every MATLAB/Octave-generated figure redrawn in
  black/grayscale, with datasets distinguished by marker shape and line style instead
  of color. The MAX485 vendor reference photo and real equipment photographs are kept
  in color as a deliberate, documented exception (not practical or useful to desaturate
  a vendor product photo or a real photograph of the rig). The major/11-6/export decks
  are explicitly exempt from this rule and keep the UNCC green template.
- **Paper acronym audit** — RTOS, HAL/LL, RPM, CSV, OLS, LOOCV, MAE, and MAPE were all
  either missing their first-use expansion or used only as bare table-header
  abbreviations with no spelled-out definition anywhere in the body text; all now
  expanded at first use.
- **Motor shaft / motor-side pulley bore** — resolved by the actual build: the
  purchased and installed GT2 pulley set (Table II) is 5 mm bore on both the 20T
  (motor) and 60T (drum) sides, confirmed by the completed, tested mechanical rig.
- **VFD output current Modbus register address** — `0x2104`, scaling ÷100, confirmed
  against GSoft2's live monitor.
- **RS-485/Modbus terminal labels on the VFD control terminal block** — full pinout
  in `docs/wiring.md` section 2.3; drive uses 8N2 framing (no 8N1 option).
- **Crane scale (QWORK) data interface** — no confirmed digital output; treated as
  manual entry (`LOAD <kg>` command), which is how every real session has been run.
  Revisit only if automatic logging is wanted later.
- **AC/mains wiring color convention** — confirmed (R/L1=black, S/L2=grey, T/L3=brown,
  carried through to U2/V2/W2 on the motor). `docs/wiring.md` section 1b.
- **Controller changed from ESP32 to STM32 Nucleo-F401RE** — firmware confirmed
  working end-to-end on real hardware; `docs/wiring.md` updated to the real STM32
  pinout throughout.
- **Local HMI/keypad** — considered and descoped (decision made, not a future item).
  The PC logger plus the STM32's own serial output is the project's final interface.
- **MATLAB/Octave figure script and diagrams/tables export** — `tools/matlab/
  generate_figures.m` regenerates every figure used in the paper and both decks
  directly from `results/*/analysis/groups.csv` where possible. A standalone,
  fully-editable `Sensorless_Load_Estimation_Diagrams_Tables_Export` deck (same rev
  number as the paper) carries every figure and table as native PowerPoint content.
- **Steel cable cost** — priced at the full roll ($19.99), David's call rather than
  prorating by measured length. No longer a placeholder/upper bound.
- **Style reference deck reviewed** — `Salisbury_David_Proposal_Presentation.pptx`
  reviewed; its clean, minimal header (logo-only bar, centered title below) informed
  the Rev 4 visual refresh.
- **SERVO Machine Test System automated data interface** — David's call: not pursuing.
  The existing manual `RPM <value>` entry workflow stays as the permanent method.
- **Torque sensor driver/Windows incompatibility** — David's call: not pursuing
  *automated* logging via the sensor's software. Stays noted as a future-work idea in
  the paper (Section VII). A manual torque-reading workaround is now tracked
  separately as A6.
- **Calibration weight ceiling (1.46 kg) explained** — David identified that heavier
  loads slip the hand-tensioned belt rather than drive the drum, which is why 0.32,
  0.58, and 1.46 kg were the weights used. Added to the paper (Section IV-C) and the
  major deck's Calibration Procedure slide.
- **4.85 Hz no-load down-direction data gap** — fully resolved by
  `Baseline_Test_Data_2026-10-05.docx` (Section 5.4), which David provided. The slip
  value in Table VI (0.030) is real: a shaft RPM of 141 read by eye for a down run
  too brief to produce a steady-state current row. The current value previously shown
  (0.761 A) had no source anywhere and has been corrected to "not recorded." Fig. 4
  now plots the documented down-direction slip at 4.85 Hz (panel a) while correctly
  leaving a gap in current (panel b), sourced directly in
  `tools/matlab/generate_figures.m` with a comment citing the originating document.
- **43 Rev 3 Word review comments** — David attached a Rev 3 copy of the paper with 43
  tracked comments that had never reached this session before 2026-11-05; all but the
  two items above (Fig. 1 redesign, RJ45 photo) are resolved in Rev 6: acronyms spelled
  out on first use (VFD, GPIO, MRAS), Human-Machine Interface capitalized, the
  self-referential "this paper differs from a typical capstone report" paragraph and
  the "Offset Mistake"/"Other Contributing Factors" sections removed, wire colors
  dropped from every table and from Fig. 4's regenerated diagram (signal names only),
  Table I's Notes column and Table II-B's Wire Color column removed, BOM headers
  reworded, a new VFD-parameters table (Table I-C) added introducing GSoft2, the VFD
  photo and raw CSV screenshot promoted to numbered figures (Fig. 3/Fig. 5, with every
  later figure renumbered to match), "T-Block" renamed to "T-housing" everywhere, a
  slip-physics explanation added to the paper (previously only in the deck), an
  OLS-fitting explanation added to Table VII, units clarified on r²/n, a mismatched
  logger-row example corrected (910626 instead of 911375, matching the screenshot),
  and all five result/diagram figures enlarged to full page width after testing showed
  they were unreadable at IEEE two-column width - which also surfaced and fixed a
  LibreOffice rendering defect where a short two-column section before a one-column
  figure could split a table's header from its body across the column break. The
  entire Results section (V) is single-column throughout as a result.
