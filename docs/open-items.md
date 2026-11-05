# Open Items

The data-acquisition and communication platform is built, wired, and validated
end-to-end on real hardware (STM32 Nucleo-F401RE, RS-485/MAX485 to the VFD, PC-side
logger). What remains falls into two groups: things that need an action from David
(Section A) and lower-priority/deferred work (Section B). Every item below is cross-
checked against the current paper (`Sensorless_Load_Estimation_IEEE_Rev5.docx`) and
all three decks as of Rev 5.

## A. Needs an action from David

### A1. Run the higher-frequency validation tests (6.36–10.00 Hz)
All calibration and blind-validation data tops out at 6.36 Hz; the drive's confirmed
safety cap is P01.10 = 10.00 Hz. Section V-E of the paper and Table X are reserved and
flagged in red, waiting for this data — same procedure as the existing blind-validation
runs (Section V-C): run a commanded frequency in that range, predict the load live
with the calibrated model, then reveal and log the actual weight.

### A2. Source two stock photos for the major deck
Two slides in `Sensorless_Load_Estimation_Deck_Rev5.pptx` still show a dashed
placeholder box instead of a real image, since neither is a photo of this project's
own hardware:
- **"Our Approach" slide:** a generic photo of a dedicated torque transducer / load
  cell, for cost contrast against this project's sensorless approach.
- **"Our Approach" slide:** a photo or logo of PAR Systems' anti-sway crane product
  (the commercial telemetry-first precedent cited as reference [1]).
Any representative stock photo or the vendor's own product image works for both.

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
- **Torque sensor driver/Windows incompatibility** — David's call: not pursuing.
  Stays noted as a future-work idea in the paper (Section VII) only, not an open item.
- **4.85 Hz no-load down-direction data gap** — fully resolved by
  `Baseline_Test_Data_2026-10-05.docx` (Section 5.4), which David provided. The slip
  value in Table VI (0.030) is real: a shaft RPM of 141 read by eye for a down run
  too brief to produce a steady-state current row. The current value previously shown
  (0.761 A) had no source anywhere and has been corrected to "not recorded." Fig. 4
  now plots the documented down-direction slip at 4.85 Hz (panel a) while correctly
  leaving a gap in current (panel b), sourced directly in
  `tools/matlab/generate_figures.m` with a comment citing the originating document.
