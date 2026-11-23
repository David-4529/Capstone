# Open Items

The data-acquisition and communication platform is built, wired, and validated
end-to-end on real hardware (STM32 Nucleo-F401RE, RS-485 transceiver to the VFD, PC-side
logger). What remains falls into two groups: things that need an action from David
(Section A) and lower-priority/deferred work (Section B). Every item below is cross-
checked against the current paper (`Sensorless_Load_Estimation_IEEE_Rev13.docx`) and
all four deliverables as of Rev 13 for the paper (the decks and lab manual are still at
their Rev 9/pre-review state - explicitly frozen until David reviews the paper). Note:
David has said the .docx is the only deliverable he wants going forward for the paper -
the PDF is still generated internally for page-render visual verification, but is no
longer something to send him.

## A. Needs an action from David

### A1. Rework Fig. 1 (system block diagram) together
Flagged in David's Rev 3 review comments: "This figure needs to be worked, let's work
together on what this should look like." Not auto-edited — the current version (drawn
by `tools/matlab/generate_figures.m`, boxes for PC Logger/STM32/MAX485/VFD/Motor+Load)
stays as a placeholder in the paper until we talk through what it should actually show.

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
- **No figure, table, or reference entry may span both IEEE columns.** Broadened in the
  Rev 11 review (comments 61-78 flagged nearly every figure/table from Section III-D
  onward) - every figure and table needs to fit within a single column's width, full
  stop. Root cause found in `build_paper.js`: large stretches of the document (the whole
  Results section, several figure/photo blocks in III-D/III-F) were deliberately
  switched to a one-column (full page width) section layout to dodge short, awkward
  two-column breaks - every figure/table inside one of those stretches is, by
  construction, full-page-width and therefore spans what should be both columns. The
  real fix is restoring genuine two-column layout for the whole paper and sizing every
  figure/table to the actual single-column width (`CONTENT_W` minus `COL_GAP`, halved -
  see `build_paper.js`'s own constants), handling any short-stretch pagination
  awkwardness with paragraph/row "keep together" properties instead of abandoning
  columns - not just shrinking individual images while leaving the section single-column.
- **Spell out every acronym's expansion in Title Case** (e.g. "Variable Frequency
  Drive," not "variable frequency drive") **before the parenthesized acronym.** New
  rule, Rev 11 review (comment 1): "All acrynms spelled out, the first letter needs to
  be captilzed."
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

### Rules pulled from the official IEEE conference Word template
David supplied the actual IEEE conference template (`conference-template-letter_2.docx`)
so our numbers stop being approximations. Real spec, US-letter page (612pt x 792pt):

- **Margins are not uniform**: top 54pt (0.75in), bottom 72pt (1.0in), left/right
  44.65pt (~0.62in) each - `build_paper.js` was using a uniform 54pt on all four sides
  (`MARGIN = 1080` twips everywhere), which is wrong on three of the four sides.
- **Two-column gutter is 18pt (360 twips)**, not the 14.4pt (288 twips) `build_paper.js`
  was using (`COL_GAP`).
- **Body font is Times New Roman, 10pt** - `build_paper.js` already had this right.
- **Figure labels/axis text: 8pt Times New Roman**, and use words with units in
  parentheses rather than bare symbols (e.g. "Magnetization (A/m)", not "A/m") - applies
  to `generate_figures.m`'s axis labels. Font FAMILY was fixed to Times New Roman in
  Rev 12; font SIZE was deliberately kept at 15pt rather than the literal 8pt - that
  spec is written for native Word text, not pixels baked into a raster PNG at a given
  DPI, and a literal 8pt here would be unreadable once the image is shrunk to single-
  column width (the exact problem a Rev 3 review comment already fixed by bumping these
  sizes up). Flagged for David to double check rather than silently decided on.
- **The template itself permits large figures/tables to span both columns** ("Large
  figures and tables may span across both columns") - this is normal, accepted IEEE
  practice in general. David's own rule above (no spanning, ever, for this paper) is a
  stricter personal preference that overrides the template's general allowance - noted
  here so the distinction isn't lost, not as a contradiction to resolve.
- **Figures/tables belong at the top or bottom of a column, not mid-column**, and should
  be inserted after the text that first cites them, not before.
- **Reference-list titles use sentence case** (capitalize only the first word and proper
  nouns), not Title Case - our current References list is Title Case and needs
  rechecking against this.
- **Acronyms/abbreviations must be defined at first use in the body text even if already
  defined in the Abstract** - the Abstract doesn't "cover" the body's first use.
  IEEE/SI/MKS/CGS/dc/rms/ac don't need defining.
- **Always use a leading zero before a decimal** ("0.25", not ".25").
- **"Data" is plural** ("the data are", not "the data is").
- Equation numbers sit flush-right in parentheses; italicize Roman variable symbols
  (not Greek symbols); use a true minus sign/long dash, not a hyphen, in equations.
- **No em dashes (—) in any deliverable** - paper, decks, lab manual, all of it. David's
  explicit new rule. Use a hyphen with spaces, a colon, or a comma/parenthetical instead,
  whichever reads most naturally in context. Applies going forward to all new/edited
  text; sweeping the existing deliverables for any already-present em dashes is part of
  what needs doing on the next revision pass (see A16), not a standalone fix right now.

### A16. Next paper revision: roll up pending fixes, wait for David's comments
David's call: hold off rebuilding until he gives more comments, then do the next
revision as one batch covering everything accumulated since Rev 14, at minimum:
A15 (BOM rod qty, 1 not 2) and the new no-em-dash rule (strip any that already exist
across the paper and sweep for them going forward). Add whatever else David's next
round of comments brings before rebuilding.

### A13. Fig. 5 (RS-485 transceiver signal-mapping diagram) needs David's own rework
Comment 254 on the Rev 9 markup: "Figure 4 needs work, I need to fix prior to you
updateing it here" (figure number has since shifted to Fig. 5 after the Rev 11
renumbering - it's the drawn VFD RJ45 -> transceiver -> STM32 pin-mapping diagram,
`tools/matlab/generate_figures.m`'s `fig2_rs485_wiring.png`). Left untouched beyond the
MAX485-label rename (Rev 11) since David said he'd provide the update himself.

### A15. Fix BOM qty for the linear rod: 1, not 2 (confirmed by David)
Surfaced by an external technical audit of Rev 12 (finding T13): Section III-C's prose
says "a single 8 mm hardened steel linear rod," but Table V (Bill of Materials) listed
"8 mm x 800 mm linear rod" at Qty Used = 2. David confirmed: 1 rod was actually used -
the prose is correct, the BOM's qty is wrong. His call: update on the next paper
revision rather than as an isolated edit now - fix Table V's Qty Used to 1 (and its
Cost, if priced per-unit rather than per-pack) at that point.

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

### B3. Mass-to-torque mechanical model - David asked whether this is really needed
Surfaced by an external technical audit of Rev 12 (finding T11): the paper reports
suspended mass (kg) as "load" but the motor physically responds to shaft torque.
David asked directly whether deriving a full mechanical model (cable radius, pulley
mechanical advantage, the 3:1 belt reduction, mechanical losses) is actually necessary.
My assessment: no, not for what this paper currently claims. The deployed estimator is
explicitly an empirical per-rig regression (current -> mass), not a physics-derived
torque model, so nothing downstream depends on a mass-to-torque conversion to be
valid. The torque-validation section already reports its "torque clusters tightly
across different masses" finding qualitatively (attributed to baseline friction/belt
tension dominating these specific small objects' torque differences) without claiming
a precise expected mass-torque proportionality to defend. A full derivation would be a
nice-to-have depth addition (or good future work/appendix material), not a gap in the
paper's actual argument. Cheaper alternative if any protection against this critique
is wanted at all: one sentence explicitly stating that a mass-torque proportionality
is not being claimed or tested here, scoping out the concern directly rather than
building the model. Staying logged here as optional, not blocking, pending David's
call on whether to do even the one-sentence version.

## Resolved

- **Rev 14: implemented the external technical/editorial audit's "comply" items** —
  David forwarded a detailed outside review of Rev 12 (30 numbered technical findings
  T01-T30, 15 editorial items E01-E15, plus a title/section-rename proposal) and asked
  me to either comply or defend. I gave David my verdict on each item directly in chat
  first (what I'd implement vs. push back on and why); this entry covers what was
  actually implemented in `build_paper.js`, producing Rev 14.
  - **Real table-numbering problem fixed (E02, independently confirmed by both this
    audit and a prior internal note)**: "Table II-B" and "Table I-C" appeared out of
    appearance order. All 15 tables renumbered I-XV in true first-appearance order
    (Equipment ID, Motor Nameplate Data, Operator Control Station Wiring, VFD
    Parameters, Bill of Materials, RS-485 Signal Mapping, Raw Logger Row Reference,
    Example Logger Summary, No-Load Baseline, Current-Based Model, Blind Validation
    Results, Measured Slip, Measured Torque, Slip vs. Original Calibration, LOOCV
    Error) and every cross-reference throughout the paper updated to match.
  - **"C1"/"C2" sub-subsection labels replaced with real IEEE "1)"/"2)" style** (E01) -
    added a new `h3()` heading helper; "V-C1"/"V-C2" cross-references throughout the
    paper now read "Section V-C, Part 1"/"Part 2".
  - **A real factual BOM error fixed, worse than what the audit itself flagged**: "Total
    out-of-pocket cost... was $385" was wrong - that sum includes the STM32 ($36.19)
    and VFD ($231.66), which the paper itself says were already in the lab and not
    purchased. True out-of-pocket is $117.14; $385 is the full replacement-cost figure.
    Fixed to state both numbers correctly and concisely. Also resolved the audit's
    separate Price-vs-Cost ambiguity (T12) by dropping the redundant Cost column -
    Table V's "Total Price" is now unambiguously the line's real paid total.
  - **T08 (slip "zero at no load")**: reworded against the no-load baseline table's own
    nonzero values (5.43%-27.0%) - now "near zero under an idealized no-load condition,"
    not "zero."
  - **T07 (dynamic-braking wording)**: "the motor lacks dynamic braking" → describes the
    actual rig configuration (no braking resistor installed/configured), not a
    categorical hardware limitation - the GS23-21P0 drive does support it generally.
  - **T06 (Abstract overclaimed validation scope)**: "further validation of the model
    was confirmed via an independent torque sensor" → "an independent torque sensor
    separately corroborated the underlying slip-load relationship," matching the scope
    the body text already correctly used.
  - **T17 (Abstract validation-count wording)**: "three independent, blind validation
    tests" → "three blinded objects at six operating points," matching Table XI.
  - **T03 (4.85 Hz baseline/model mismatch)**: verified by direct arithmetic
    (7.29×0.814−5.57 = +0.364 kg, not 0) and explained in-text as a direct consequence
    of that frequency's already-reported low r² (0.508), not a separate-session issue.
  - **T04 (current-span "0.1 A" claim)**: recomputed from the real per-frequency
    current ranges in `tools/model/load_model.json` (0.066-0.153 A across the four
    frequencies, not a single fixed 0.1 A) and restated with the real numbers.
  - **T09/T14 (Limitations, VI-B)**: added the real shaft-speed-resolution limit at
    1.39 Hz (±1 RPM ≈ ±2.4 slip percentage points, from Ns=41.7 RPM) and showed the
    64.0 in / 19.6 s cable-travel figure's derivation (~1.96 in effective drum
    circumference per revolution) explicitly instead of leaving it implicit.
  - **T10 (Discussion's "noise floor" closing sentence)**: softened from an assertion
    ("that noise floor is the binding constraint") to "is consistent with a binding
    signal-to-noise floor" - the rest of Section VI-A's hedged argument was left as-is.
  - **T16 (why n varies 3-5 across frequencies)** and **T18 (1.39 Hz pliers torque
    outlier)**: both given one-sentence explanations tied to already-documented causes
    (incomplete clean runs; the same belt-traction margin issue noted in Limitations).
  - **T19 ("independent sensor on different hardware" overstated)**: reworded toward
    corroboration on a shared mechanical path rather than implying full experimental
    independence, both where it's first stated and in the Conclusion.
  - **T02 (blind-prediction frequency selection, undocumented)** and **T30 (LOOCV model
    forms, undefined)**: both pulled from real source - `tools/model/fit_model.py`'s
    actual `predict()` function uses nearest-calibration-frequency selection (no
    interpolation/extrapolation) with the result floored at 0 kg; the four LOOCV model
    forms are now spelled out explicitly (linear; + interaction term; + quadratic freq;
    full). **T05 (MAPE's zero-load handling)**: the exact original LOOCV/MAPE
    computation isn't in any committed, rerunnable script (it was done ad hoc, same
    situation `generate_figures.m` already notes for Figs. 11/14) - rather than guess
    at an unverifiable implementation detail, the paper now states MAE (unambiguous, in
    kg) as the primary metric and flags MAPE's 0 kg blind spot generically.
  - **T20 (commanded vs. output frequency)**: confirmed from `firmware/stm32/Core/Src/
    main.c` that these are genuinely two different Modbus registers (0x2103 output
    frequency, which the firmware actually reads and logs; 0x2102 frequency command) -
    added one sentence clarifying "commanded frequency" means the former, which
    coincides with the latter at steady state under this drive's open-loop V/Hz control.
  - **T22 (which UART)**: the Modbus link is USART1, distinct from USART2 (PC console) -
    now named explicitly where first introduced, matching how USART2 is already named.
  - **T23/T24 (steady-state threshold, RPM-run pairing)**: pulled the real constants
    from firmware (`STEADY_MIN_FREQ_HZ=0.5`, `STEADY_FREQ_TOL_HZ=0.05`) and stated the
    real logger behavior (RPM is asked immediately after each run ends, before the
    next begins) instead of leaving both unspecified.
  - **T26 (P03.00's value)**: Table IV's P03.00 row now shows the real numeric value
    ("1 (frequency command)"), matching `docs/vfd-parameters.md` and the pattern every
    other row in that table already uses.
  - **Title (T01)**: minimally reworded - "Sensorless Load Estimation via Current and
    Slip..." - rather than adopting either of the audit's two longer proposed
    replacement titles, which conflicted with the standing "short and concise" wording
    instruction; the Abstract already distinguished deployed current from evaluated
    slip reasonably well, so no further rewrite was needed there.
  - **Explicitly not implemented, by design** (see the chat response to David for the
    full reasoning): no further section/heading renaming beyond the C1/C2 fix above - a
    dedicated fresh-eyes naming pass already happened one revision ago; the title
    wasn't replaced wholesale; references [2]/[9]-[13] remain deliberately uncited
    in-body; no limit-switch/e-stop content was added (this is a manually-operated
    bench rig, not a product).
  - **Logged rather than guessed on**: T13 (rod-count contradiction between prose and
    Table V) as new item A15, and T11 (a full mass-to-torque mechanical model) as new
    item B3 - both need real information (David's own knowledge of the rig, or new
    analysis) rather than a text edit.
  - Build/verify: 11 pages, clean page-by-page visual check, and the established
    programmatic column-width check re-run clean (0 violations across 15 images/15
    tables against the real 5047-twip single-column ceiling).
  - File: `scratchpad/paper_rev0/Sensorless_Load_Estimation_IEEE_Rev14.docx`.

- **Major presentation deck synced to Rev 13** — brought
  `Sensorless_Load_Estimation_Deck_RevN.pptx` (frozen at Rev 9) in line with the
  paper's Rev 10-13 changes:
  - **A real factual contradiction fixed**: the Reference Instrumentation slide claimed
    the SERVO Machine's torque sensor "was never used in this project... its driver
    software is incompatible," the same wrong claim comment 45 caught in the paper.
    Corrected to match: the torque sensor was used for independent validation, and is
    not part of the live, deployed (sensorless) platform.
  - **"MAX485" renamed to "RS-485 Transceiver"** throughout (slide titles, the native
    System Signal Path and RS-485 Wiring Diagram shape labels, the module-reference
    slide, and the BOM row) - including inside the native diagram shapes themselves,
    not just surrounding prose.
  - **BOM rebuilt** to match the paper's simplified Table II: one real Qty/Price per
    row (no more Used/Pack prorating), cable qty "whole roll" → "1", total cost as one
    flat "$385" instead of the old $99.34-out-of-pocket/~$367-reference split.
  - **Down-direction data removed from the No-Load Baseline slide** (table columns and
    figure) - the motor has no dynamic braking, so it was never physically meaningful;
    swapped in the paper's two newly-split figures (slip, current) in place of the old
    combined chart.
  - **Torque-validation results slide**: swapped in the paper's two split figures
    (slip, torque) in place of the old combined chart; torque values converted mN·m →
    N·m; dropped the "100-102%/raw-channel-correction" explanation from the setup
    slide to match the paper's own trim (David's call there: it's a distraction from
    the main point).
  - **New Fig. 7 slide added (A14)**: an STM32 firmware control-flow chart built from
    native PowerPoint shapes (boxes + arrows, including an elbow-routed loop-back
    connector), grounded in the real `firmware/stm32/Core/Src/main.c` loop - matches
    this deck's existing native-diagram convention (System Signal Path, RS-485 Wiring
    Diagram). Note: a separate pass also added this chart to the Diagrams & Tables
    Export deck (see below) - David's A14 comment pointed at "the powerpoint that has
    all the examples," which is that export deck, but it fits this deck's own
    firmware-explanation slide naturally too, so both now have it.
  - **5 new references added** ([9]-[13], same sources as the paper), and all 13
    references (including the original 8) converted to sentence case to match the
    paper's reference-list rule.
  - **Heading renames to match the paper's fresh-eyes section-naming pass**: "Is This a
    Modeling Problem?" → "Model Complexity Analysis"; "Why ~50% Error? A
    Signal-vs-Noise Budget" → "Signal-to-Noise Error Budget".
  - **PLA spelled out** (Polylactic Acid) on the Mechanical Build slide; dropped the
    "QWORK" brand name from prose per the existing don't-repeat-manufacturer-names rule.
  - **Open Items slide**: removed the closed A4 (RJ45 photo) bullet; added A5/A8 so the
    slide matches the current docs/open-items.md Section A instead of an older subset.
  - Stale "Section V-E"/"Table X/XI" paper cross-references updated to the current
    "Section V-C2"/"Table IX/X" throughout (Next Steps, Deliverables, and the
    torque-validation notes).
  - Verified by rebuilding, converting to PDF, and visually checking every changed
    slide, including iterating on the new flowchart slide twice to fix a subtitle
    text-overflow bug and a loop-back arrow that was routed to the wrong box.
  - File: `scratchpad/deck_rev1/Sensorless_Load_Estimation_Deck_Rev13.pptx` (42 slides).

- **Diagrams & Tables Export deck synced to Rev 13, new flowchart slide added** —
  brought `Sensorless_Load_Estimation_Diagrams_Tables_Export_RevN.pptx` (frozen at
  Rev 9) in line with the paper's Rev 10-13 changes, and resolved A14 (the STM32
  firmware flowchart needed to live in "the powerpoint that has all the examples which
  allows me to move the lines around" - this is that deck).
  - **New Fig. 7 slide**: the STM32 firmware control-flow chart, built entirely from
    native PowerPoint shapes (8 boxes, 6 down-arrows, 2 right-arrows, 4 connector
    lines - 36 separate `<p:sp>` shape elements confirmed directly in the saved
    pptx's XML, 0 `<p:pic>` image elements) so every box and arrow can be moved
    independently, per David's explicit request. Grounded in the real
    `firmware/stm32/Core/Src/main.c` loop. Caught and fixed a real layout bug during
    verification: the first draft routed the loop-back arrow straight through the
    "Every 250 ms" and "Compute sync RPM" boxes instead of around them - rerouted
    outside the box column entirely.
  - **Figures renumbered/added to match the paper exactly** (1-14, plus the unnumbered
    RS-485 vendor-reference image): Fig. 2 (full test rig photo) is newly numbered
    here to match the paper; the old combined no-load-baseline figure and the old
    combined torque-validation figure are each now two separate figures (Figs. 8/9
    and 12/13) per the paper's Rev 11 split; down-direction data dropped from the
    baseline figure entirely (no dynamic braking on this motor).
  - **Tables renumbered/added to match**: new Table I-B (Motor Nameplate Data); Table
    II (BOM) simplified to one real Price=Cost per row, $385 flat total (no more
    pack-price/prorated framing); Table III's "MAX485 Pin" header and slide title
    renamed to "Transceiver Pin"/"RS-485 Transceiver Signal Mapping"; the old combined
    7-column torque+slip table split into Table IX (slip only) and Table X (torque
    only, N·m not mN·m, "Corrected" dropped); Table XI's "Oct. 7 Calibration" column
    header shortened to "Calib." (no bare calendar date); Table I/Table II-B's VFD
    terminal column/header, Table I-C's caption ("Section III-E2" → "Section III-E"),
    and Table VIII's headers shortened to match the paper's current wording exactly.
  - Verified two ways: visual page-by-page PDF render (31 slides, clean) and a direct
    text sweep of the rendered PDF confirming zero remaining "MAX485" or "mN" instances
    anywhere in the deck.
  - This deck still carries no References slide (it never had one - out of scope to
    add now; it exports figures/tables only, not the paper's prose or citations).
  - File: `scratchpad/export_rev3/Sensorless_Load_Estimation_Diagrams_Tables_Export_
    Rev13.pptx`.

- **11/6 progress-meeting deck synced to Rev 13** — brought `Capstone_Progress_Update_
  11_6_26_RevN.pptx` (frozen at Rev 9) in line with everything the paper's Rev 10-13
  cycle changed. Kept the deck's own point-in-time content intact (it's a dated snapshot
  of the Nov 6 meeting - the completion percentages, "What's Left," and timeline slides
  stay as they were, since rewriting those would misrepresent what was actually known at
  that meeting); only fixed genuine factual/terminology drift against the now-current
  paper:
  - Down-direction (lowering) slip/current columns removed from the No-Load Baseline
    table (Slide 7) - the motor has no dynamic braking, so this was never physically
    meaningful and isn't reported anywhere in the paper anymore; the slide's framing
    sentence updated to match, and the explanatory bullet beneath it rewritten
    accordingly (previously claimed slip was "strongly direction-dependent," which no
    longer makes sense without a down-direction column to compare against).
  - "RS-485/MAX485" - "RS-485 transceiver" (Slide 5, matching the Rev 11 rename).
  - "Lucas Nülle SERVO Machine Test System (CO3636-6W7)" - "SERVO Machine Test System"
    and "QWORK digital scale" - "digital hanging scale" (Slide 3), matching the
    don't-repeat-part-numbers-in-prose rule; the VFD/motor make-and-model bullets on the
    same slide were left alone since they function as an equipment list, not prose.
  - "T-Block" - "T-housing" (Slides 5 and 10) - a stale name from before the paper's own
    Rev 3-6 rename; this deck had never picked it up.
  - Speaker note on Slide 6 updated from a stale "$99.34 purchased, by actual quantity
    used" BOM figure to the current flat "$385 total project cost."
  - No mN·m, torque-sensor-contradiction, or USART2/PLA content found in this deck -
    those don't appear here, nothing to fix on those fronts.
  - Verified via PDF page-by-page render - no overflow, tables fit their slides cleanly.
  - File: `scratchpad/deck_1106/Capstone_Progress_Update_11_6_26_Rev13.pptx` (not
    committed - deck sources and outputs live only in scratchpad, same as the paper's
    own build script).

- **Rev 13: fresh-eyes section-naming/flow review, 6 new comments, unit italics,
  5 new references, A4 closed** — David asked for a genuine fresh-eyes pass over every
  section/subsection name and the overall flow against real IEEE convention, plus 6 new
  comments on a Rev 12 markup (no tracked changes this round), then added three more
  decisions mid-pass: bring A11 (italicize units) into scope now, add 5 real references
  he'd found via web search, and close A4.
  - **Section/subsection naming fixed**: "E2. Software Tools: GSoft2 vs. STM32CubeIDE"
    was nested as a sub-subsection under "E. Human-Machine Interface," but it's about
    development/configuration tooling, not runtime operator interaction - a genuine
    topic mismatch, and almost certainly what comment 60 ("This section formatting is
    not right...") and David's separate chat note about sections "mixing in with each
    other" were both pointing at. Fixed: given its own top-level letter, moved right
    after "D. RS-485 Interface" (where GSoft2 is first introduced) and before HMI;
    Section III is now A-G (Overview, VFD/Motor/Instrumentation, Mechanical Build,
    RS-485 Interface, Software Tools, Human-Machine Interface, Data Logging), with every
    cross-reference to the old E2/E/F letters updated throughout the paper (III-E2 to
    III-E, III-F to III-G, in four separate places including one inside the Discussion
    section that a plain text search would have missed). Also renamed "D. Is This a
    Modeling Problem?" (Section V) to "D. Model Complexity Analysis" and "A. Why
    Approximately 50% Error: A Signal-vs-Noise Budget" (Section VI) to "A. Signal-to-
    Noise Error Budget" - both were stylistically inconsistent with the short noun-phrase
    heading convention used by every sibling heading in the paper (a question-style
    heading and a colon-subtitle heading, respectively). Considered but not changed:
    Section III-B ("VFD, Motor, and Reference Instrumentation") is broad - it also
    carries the operator control station wiring and VFD parameters table - but the
    content still coheres under one "describing the hardware" topic, so it was left
    alone rather than split without being asked. Also flagged, not fixed (out of scope
    for a section-naming pass): the table numbering sequence has a real ordering problem
    - "Table II-B" (Operator Control Station Wiring) appears in reading order *before*
    "Table I-C" (VFD Parameters), which reads as if it belongs to a later table "family"
    than one that actually follows it. Fixing this would mean renumbering nearly every
    table in the document and every in-text reference to it - flagged for a dedicated
    future pass rather than attempted here.
  - **6 new comments, all accounted for**: comment 0 (BOM's "single roll" to "1",
    matching every other row's numeric Qty Used convention); comment 1 (Polylactic Acid
    (PLA) spelled out at first use); comment 2 (every prose mention of the bare part
    number "CO3636-6W7" - three instances - changed to "SERVO Machine," matching how the
    standing rule already treats other part numbers; the part number itself stays in
    Table I, where it belongs); comment 3 (USART2 given its own first-use expansion,
    Universal Synchronous/Asynchronous Receiver-Transmitter, distinct from UART); comment
    4 (logged as new action item A14, not a Word-doc fix - see above); comment 5 (trimmed
    "both r² and n are dimensionless counts" as unnecessary).
  - **A11 resolved**: extended `body()`'s helper to support mixed plain/italic runs
    within one paragraph (a lightweight `*marker*` convention parsed into separate
    `TextRun`s, rather than rewriting every call site to pass an array), then ran a real
    pass over every running-text unit mention in the paper - kg, A, Hz, V, kW, mm, ms, s,
    HP, N·m - leaving table cells, equation text, and figure axis labels untouched since
    those were already correctly formatted and the rule is specifically about running
    prose. Caught and fixed two false positives from the first automated pass (motor
    terminal labels "V/T2" and "W/T3" were briefly, incorrectly italicized by a regex
    that mis-parsed "...T1, V/T2..." as a number-comma-unit sequence) before finalizing.
  - **A10 resolved**: 5 real references added as [9]-[13] (induction-motor sensorless
    load/torque estimation and nonintrusive motor-assessment literature), found via web
    search and verified against their actual DOIs rather than fabricated. Not cited
    in-body, consistent with the existing A10 precedent for refs [2]/[3]/[8].
  - **A4 closed**: David's call - "close this," no RJ45 close-up photo is coming. Moved
    to descoped below; no paper change needed since the inline action-item bracket for
    this was already removed in an earlier revision.
  - Verified: visual page-by-page PDF render (10 pages, unchanged from Rev 12 - the
    reference/heading/unit-italics work didn't add pages) plus the same programmatic
    column-width check as Rev 12 (every image/table width against the 5047-twip
    single-column ceiling) - 0 violations across 15 images and 15 tables, no regression
    from the section reordering.
  - Files: `scratchpad/paper_rev0/Sensorless_Load_Estimation_IEEE_Rev13.docx`.
- **A4. Photo of the VFD's RJ45 jack specifically** — descoped, David's call: "close
  this." No photo is coming; the paper's inline action-item bracket for this had already
  been removed in an earlier revision, so no further paper change was needed.
- **Rev 12: root-cause column-span fix, full Rev 11 comment audit, real IEEE template
  numbers, new firmware flowchart** — David marked up Rev 11 with 20 new comments (1,
  60-78) and 59 tracked changes, plus two things said directly in chat: a new rule that
  no figure/table/reference may ever span both columns, and that the paper is missing a
  software flow chart. Also supplied the real official IEEE conference Word template,
  showing `build_paper.js`'s layout constants were wrong approximations (see "Rules
  pulled from the official IEEE conference Word template" above).
  - **Root cause of nearly every figure/table rendering full-width** (comments 61-63,
    70-78): `build_paper.js` had deliberately switched the entire Results section, and
    several figure/photo blocks in Section III-D/III-F, to one-column (full page width)
    layout to dodge short, awkward two-column breaks - every figure/table inside one of
    those stretches was full-width by construction. Fixed structurally: restored genuine
    two-column layout for the whole paper (down from 9 section breaks to 5: one
    one-column title/abstract block plus four two-column sections) and resized every
    figure/table to the real single-column width. This also resolved comment 60 ("This
    section formatting is not right, its getting added before the previous section is
    done") and David's separate chat note that "sometimes [sections] look to be mixing
    in with each other" - both were symptoms of the same fragmented-section root cause.
    Verified two ways, not just by eye: visually, page by page, and programmatically (a
    script checking every `<wp:extent>` image width and `<w:tblW>` table width in the
    built docx's raw XML against the real single-column ceiling of 5047 twips - 0
    violations across 15 images and 15 tables).
  - **Real IEEE template margins/gutter applied**: top 1080 / bottom 1440 / left-right
    893 twips (was a uniform 1080 on all sides), column gutter 360 twips (was 288).
  - **New STM32 firmware control-flow chart added as Fig. 7** (David's explicit chat
    request, "a simple flow chart of the stm32 code") - grounded in
    `firmware/stm32/Core/Src/main.c`'s real init/poll/log/command-handle loop, drawn as
    a simple box-and-arrow chart via a new Octave script
    (`scratchpad/flowchart/gen_flowchart.m`, not part of `generate_figures.m` since it's
    a schematic, not a data plot) sized to fit single-column width (its height runs the
    full column, which is fine - the no-spanning rule is about width, not height).
  - **Comment 71 caught in the full audit, missed in the Rev 11 pass**: Fig. 8 (no-load
    baseline, slip + current in one two-panel figure) needed splitting into two
    standalone figures, same as the torque-validation figure already was in Rev 11.
    Done (`generate_figures.m` now emits `fig4_baseline_slip.png` /
    `fig4_baseline_current.png`), which bumped every subsequent figure number by one
    (Fig. 9 onward).
  - **A real LibreOffice table-rendering bug found and fixed during verification**: Table
    XII's data rows were silently dropped from the PDF render (confirmed present in the
    docx's raw XML the whole time) when the table sat right after its figure at a column
    break. Reordering it to table-then-figure, matching every other subsection's
    convention in Results, fixed it - a third LibreOffice-specific rendering quirk found
    this way, after the header-split and image-drop bugs Rev 11 already caught.
  - **BOM total simplified further**: per David's own tracked edit, the "$117.14
    out-of-pocket / ~$385 reference cost" two-number framing is now one flat sentence,
    "Total out-of-pocket cost for this project was $385."
  - **Acronym fixes**: Convolutional Neural Network (CNN), Leave-One-Out
    Cross-Validation (LOOCV), and Comma-Separated Value (CSV) capitalized correctly;
    VFD and RTU - previously defined only in the Abstract - now also spelled out at
    their first body-text use, per the new "define in the body even if already in the
    Abstract" template rule.
  - **References converted to sentence case** per the template rule (e.g. "Anti-sway
    crane motion controls," not "Anti-Sway Crane Motion Controls").
  - **"Down-direction data is" → "data are"** (data-is-plural template rule); no other
    singular/plural "data" issues or missing leading zeros found elsewhere in the paper.
  - **Equation numbering fixed to be genuinely flush-right** via a real tab stop instead
    of manual space-padding, which would have silently misaligned once the column width
    changed.
  - Smaller comment-driven fixes: "Calibration Procedure" → "Calibration and Baseline
    Testing" heading (comment 67); Table IV's "steady" field description shortened
    (comment 64); the Fig. 7 (old)/calibration-objects photo removed with its three
    objects and weights named directly in text instead (comment 69); the one-sentence
    "C. Validation" intro removed as adding nothing beyond its own C1/C2 subheadings
    (comment 74); "SERVO Machine" capitalization confirmed already consistent throughout
    (comments 65/66 - no change needed).
  - Not done: the full remaining IEEE-template rule list (figures/tables at top/bottom of
    a column specifically, not just inserted after first citation - current placement is
    reasonable but not rigorously re-checked against "top or bottom of column" placement)
    wasn't re-verified exhaustively; flagged for a future pass rather than claimed done.
  - Files: `scratchpad/paper_rev0/Sensorless_Load_Estimation_IEEE_Rev12.docx`. Figures
    regenerated in `tools/matlab/figures/` and copied into `paper_rev0/assets/`.

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
