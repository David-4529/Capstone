# Open Items

The data-acquisition and communication platform is built, wired, and validated
end-to-end on real hardware (STM32 Nucleo-F401RE, RS-485 transceiver to the VFD, PC-side
logger). What remains falls into two groups: things that need an action from David
(Section A) and lower-priority/deferred work (Section B). Every item below is cross-
checked against the current paper (`Sensorless_Load_Estimation_IEEE_Rev11.docx`) and
all four deliverables as of Rev 11 for the paper (the decks and lab manual are still at
their Rev 9/pre-review state - explicitly frozen until David reviews the paper).

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

### A8. Real motor torque/breakdown-torque curves from Lucas Nülle
Surfaced in the Rev 9 Word review (comment on Section III-B): David says he has his own
action to pull real torque-speed curve data for the SE2673-1K7 (not from a published
datasheet, which doesn't exist - see A7 Resolved below). Once he has it, the paper's
"no published spec" framing in Section III-B needs updating to cite/plot the real curve
instead.

### A9. Confirm the PPT limitations slide covers the cable-travel/time discussion
David's call: the cable-length/19.6 s safety-margin discussion is being pulled out of
the paper's VFD-parameters prose (it reads as a design footnote there, not a result) and
instead needs to live in a Limitations discussion - David believes this is already in
his PowerPoint slides somewhere and wants that confirmed rather than rewritten from
scratch in the paper.

### A10. Add ~5 more applicable IEEE reference entries
David's call: the References list should carry roughly 5 more applicable sources beyond
the current 8 - they don't need to be cited in-body, just listed as references. Needs
real candidate sources (not fabricated) - likely more induction-motor/VFD/sensorless-
estimation literature in the same vein as refs [2]/[3]/[8].

### A11. Italicize every unit in running text (Rev 10 follow-up)
Flagged while building Rev 10: the "every unit mentioned in running text must be
italicized" standing rule (below) was not applied in that pass. `build_paper.js`'s
`body()` helper only supports single-run paragraphs, so retrofitting ~100 paragraphs'
worth of mixed-run text (plain prose with italic units spliced in mid-sentence) needs a
small helper rewrite first, not a one-off text edit - doing it piecemeal just on the
sentences touched in Rev 10 would have looked like an inconsistency rather than a rule.
David's call: hold this for the next paper revision rather than block Rev 10 on it.

## Standing rules (style/formatting, apply going forward)

Surfaced in the Rev 9 Word review (comments + tracked changes); apply to all future
edits of the paper, not just the passages they were originally flagged on.

- **Say "VFD," not "drive."** Applies throughout the paper (and by extension the other
  deliverables) wherever the drive itself is meant, not a disk/mechanism sense of "drive."
- **Don't repeat manufacturer/model names in prose.** Keep body text high-level ("the
  custom hoist," "the motor," "the generator") and let the equipment table (Table I)
  carry specific make/model/part-number detail - don't restate it inline every time a
  component is mentioned.
- **Every unit mentioned in running text must be italicized** (e.g. *kg*, *A*, *Hz*),
  not just units inside tables/equations.
- **No table may span both IEEE columns.** Every table needs to fit within a single
  column's width - reformat (narrower columns, smaller font, or split tables) rather
  than let a wide table bridge both columns.
- **Don't call out specific calendar dates in body text** (e.g. "Oct. 24, 2026") -
  describe sessions by what they were, not when they happened.
- **Torque is reported in N·m, not mN·m.** Reverses the Rev 9 engineering-notation
  decision specifically per David's instruction - applies to every mN·m instance
  (Table X/XI headers and values, Fig. 10 axis label, in-text mentions) across the
  paper, decks, and `generate_figures.m`.
- **Every figure and table kept in the paper must be referenced by number somewhere in
  the body text** - before cutting or keeping a figure/table, confirm there's a sentence
  that actually points to it (e.g. "(Fig. 8)" / "(Table VII)"); orphaned figures/tables
  get removed or given a proper callout.
- **Spell out every acronym in full at first use, with the acronym in parentheses
  immediately after** - reinforces the existing standing rule; this round caught CNN,
  RTU, and UART missing their expansion despite being used as bare acronyms later.

### A13. Fig. 5 (RS-485 transceiver signal-mapping diagram) needs David's own rework
Comment 254 on the Rev 9 markup: "Figure 4 needs work, I need to fix prior to you
updateing it here" (figure number has since shifted to Fig. 5 after the Rev 11
renumbering - it's the drawn VFD RJ45 -> transceiver -> STM32 pin-mapping diagram,
`tools/matlab/generate_figures.m`'s `fig2_rs485_wiring.png`). Left untouched beyond the
MAX485-label rename (Rev 11) since David said he'd provide the update himself.

## Standing rules (process)

- **Any tracked-changes/comments review must be a complete, full audit - never a
  selective pass.** David's instruction after catching that comment 284 slipped through
  the first Rev 9 review pass: "add a rule to always do a complete and full aduit of
  provided document of track changes and comments." Concretely: when reviewing a marked-
  up document, go through every single comment and every tracked-change operation
  individually and account for it (resolved / not yet resolved / superseded and why) -
  don't rely on skimming for the ones that look most significant, and don't report a
  review as "done" without having actually checked each item against the current state
  of the document. See A12 for the audit this rule itself came from.

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

- **A12. Full Rev 9 comment-by-comment audit, Rev 11** — David caught that several Rev 9
  comments were never actually incorporated into Rev 10 and asked for a real, systematic
  audit rather than trusting the earlier selective pass. Went through all 89 original
  Rev 9 comment IDs individually against the Rev 10 text (not just a resolved_text.txt
  diff, since some comments - like 284 - have no accompanying tracked change at all) and
  fixed every one found not actually resolved, producing Rev 11:
  - **A real contradiction**: Section III-B claimed "a torque sensor... was not used in
    this project... no torque data was recorded" while Section V-C2 (the whole
    Independent Torque-Based Validation section) exists specifically because that torque
    sensor *was* used - comment 45's "Wrong, I was able to get the data from it" was
    correct and had never been applied. Fixed.
  - **Photos/content marked "Remove this" that were still present**: the duplicate
    "Photographed nameplates confirming Table I" photo block (comments 43/44), the
    GT2/drum/QWORK close-up photo block (comment 198), the blind-validation-objects
    photo block (comment 319, attached directly to the image with no text anchor - easy
    to miss without checking the raw XML), and both inline `[ACTION ITEM: ...]` brackets
    in the paper body itself (comment 212 plus the same logic applied to Fig. 1's
    matching bracket) - all removed; the underlying action items stay tracked here
    instead (A1, A4).
  - **"MAX485" renamed to "RS-485 transceiver" throughout** (comments 231/237, repeated
    twice) - prose, section/table titles, Table III's column header, and both MATLAB-
    drawn diagrams (Fig. 1's block label, Fig. 5's own diagram text) that still had the
    old chip name baked into the image itself.
  - **A systematic figure/table reference audit** (comments 196, 292, 315, 318, plus the
    existing standing rule): 8 of the paper's 10 figures had no in-text pointer beyond
    their own caption, only caught by actually counting every "Fig. N" occurrence rather
    than spot-checking. Added a real pointer sentence for every figure and for Table VII.
    Two photos (the full test rig, the three calibration objects) were promoted from
    unnumbered images to properly numbered, referenced figures per comments 196/292.
  - **Table IX (torque+slip combined, 7 columns) split into two 4-column tables**
    (Slip, Torque) paired with two split figures, per comment 354's explicit ask to make
    these "two separate figures" - also resolves comment 348's column-span complaint,
    which a 7-column table could not have met by reformatting alone.
  - **New Table I-B (Motor Nameplate Data)** with the motor's real rated voltage/current/
    power/speed/poles, addressing comments 17 ("doesn't list the nameplate data but
    rather calls out the make/models") and 38 (table request) together - Table I is
    renamed "Equipment Identification" to match what it actually contains.
  - Smaller fixes: P01.02 forward-reference reworded (comment 49); Table IV's verbose
    "Meaning" cells shortened (comments 272/275-280); the down-direction aside trimmed
    from the Table V discussion (comment 281); a one-sentence 0 kg lower-bound rationale
    added before the 1.46 kg upper-bound paragraph (comment 291); the generator/load unit
    named explicitly as where the SERVO's torque sensor sits (comment 343); the dense
    VI-A closing paragraph tightened without losing its technical content (comment 363);
    the two overlapping MAX485/RS-485 explanation paragraphs merged into one (comments
    238/256); the MAX485 reference photo moved to right after its first mention and
    resized down (comments 234/255).
  - Comments confirmed already correctly resolved on inspection (no further change
    needed): 5, 12, 13, 14, 15, 16, 18, 46, 50-55, 61, 62, 194, 195, 197 (partially - see
    above), 208, 210, 232, 233, 235, 252, 253, 257, 263 (deferred per David's own
    standing instruction, not missed), 265, 266, 283, 285, 294-297, 299, 300, 320, 337,
    342, 349, 352, 355, 356, 358, 361. Comments 311 (unit italics) and 373 (more
    references) remain intentionally open - see A11 and A10.
  - Also fixed along the way: a LibreOffice-specific PDF-export bug where the single
    structurally-last image in a continuous section would silently fail to render
    (reproduced independent of file content/format - confirmed by swapping in a known-
    good image at that position and watching it also disappear); worked around by never
    ending a one-column section on an image when a table can trail it instead. And a
    table-header-split-across-a-page-break defect (same family as the one already noted
    below from Rev 6), fixed by setting `cantSplit` on every table row.
- **Down-direction (lowering) slip/current data removed from Table VI, Fig. 6, and the
  model entirely (Rev 10)** — three of David's own Rev 9 comments on Table VI/Fig. 6
  contradicted each other about whether down-direction data should be included. David's
  resolution: remove it entirely, since the motor has no dynamic braking and
  down-direction current/slip readings were never physically meaningful or actually used
  in the paper's current-based model. Table VI is now up-direction only (Freq/Slip/
  Current), Fig. 6 regenerated as a single-series two-panel plot, and
  `generate_figures.m` updated to match.
- **MRAS (Model Reference Adaptive System) discussion removed from Related Work
  (Rev 10)** — David's comment ("Need to expand on what these are or remove them if not
  needed") resolved as: remove. The Utrata and Rolek [2] MRAS-speed-estimator sentence
  was cut from Section II; reference [2] stays in the References list uncited,
  consistent with the A10 precedent that extra references don't need an in-body
  citation.
- **Blind Validation and Independent Torque-Based Validation merged under one
  Validation section (Rev 10)** — David's comment on the torque-validation heading
  asked whether it could be grouped with Blind Validation "within the same chapter."
  Resolved as: Section V-C ("Validation") now contains two subsections, V-C1 (Blind
  Validation) and V-C2 (Independent Torque-Based Validation); the "Is This a Modeling
  Problem?" (LOOCV) section moved after both, keeping its own letter (V-D) but its
  table/figure renumbered (Table IX→XI, Fig. 9→10) to stay in document order. All
  cross-references to the old V-C/V-D/V-E throughout the paper were updated to match.
- **Bill of Materials (Table II) simplified to a single real Price/Cost per row
  (Rev 10)** — the prorated-cost framework (pack price vs. fraction-of-pack-used cost)
  was being removed per David's tracked edits, but left two rows (timing belt, RS-485
  module) with a blank Price cell and a stale "$99.34 vs. $117.14" comparison sentence.
  David's call: show one real price per row (what was actually paid for that purchase,
  not prorated). New total out-of-pocket cost is $117.14 (every component at its actual
  purchase price); reference cost including the lab-provided STM32/VFD at list price is
  roughly $385.
- **Section IV-B shaft-RPM-reading scope and validation-narrative trims (Rev 10)** —
  three follow-up fixes from David after the main Rev 9 review pass: (1) Section IV-B's
  "only ever taken by hand" claim about the shaft RPM reading now explicitly scopes to
  this project's own calibration/deployment workflow and cross-references the
  independent SERVO-based validation (Section V-C2), which obtained slip electronically
  instead, with no hand reading involved; (2) the "100-102%"/raw-channel-correction
  explanation was removed from Section V-C2 entirely ("takes away from the main purpose
  of the paper," David's words) — the section now presents Table IX directly without
  narrating why the SERVO's own slip readout needed correction, and "Corrected" was
  dropped from Table X's header/title for the same reason; (3) the Conclusion
  (Section VII) was expanded to explicitly state what the independent torque-based
  validation confirmed (the integrity of the slip-load relationship), not just the
  one-clause mention it had before.
- **3-sig-fig / units / engineering-notation pass (Rev 9)** — every measured or
  derived number in the paper and all three decks (Tables VI, VII, VIII, IX, X, XI)
  standardized to 3 significant figures using the most precise available source data
  (raw `groups.csv` files, the regression fit, and the `.acts` torque captures), not
  just re-padded display strings; a stale, previously-corrected data error (the 4.85 Hz
  down-direction current in the 11/6 deck's baseline table, "0.761 A") was caught and
  fixed to match the rest of the project in the process. Every table header now states
  its unit except genuinely unitless quantities (r², n, counts). Torque is now reported
  in engineering-notation mN·m instead of N·m, since the raw values (0.0098–0.0291 N·m)
  were awkward with leading zeros; this cleaned up Table X/XI and Fig. 10 in the paper,
  both decks, and the export deck. Exact configured VFD parameters (e.g. 208.0 V,
  10.00 Hz) and nominal catalog/CAD dimensions were deliberately left at their own
  genuine resolution rather than forced to 3 figures, since that would either discard
  real information or fabricate precision that isn't there.
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
