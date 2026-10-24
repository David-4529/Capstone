const fs = require("fs");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell, WidthType,
  ShadingType, HeadingLevel, AlignmentType, BorderStyle, LevelFormat,
  Footer, PageNumber,
} = require("docx");

const OUT = process.argv[2];
const CONTENT_W = 9360; // US Letter, 1" margins
const FONT = "Calibri";
const MONO = "Consolas";
const HEAD_FILL = "D9E2F3";
const UP_FILL = "FFFFFF";
const border = { style: BorderStyle.SINGLE, size: 4, color: "A6A6A6" };
const borders = { top: border, bottom: border, left: border, right: border };

const p = (text, opts = {}) => new Paragraph({
  spacing: { after: 120 },
  ...opts,
  children: Array.isArray(text) ? text : [new TextRun({ text, font: FONT, size: 22 })],
});
const r = (text, o = {}) => new TextRun({ text, font: FONT, size: 22, ...o });
const h1 = (t) => new Paragraph({ heading: HeadingLevel.HEADING_1, spacing: { before: 280, after: 120 }, children: [new TextRun({ text: t, font: FONT })] });
const h2 = (t) => new Paragraph({ heading: HeadingLevel.HEADING_2, spacing: { before: 220, after: 100 }, children: [new TextRun({ text: t, font: FONT })] });
const bullet = (children) => new Paragraph({ numbering: { reference: "bullets", level: 0 }, spacing: { after: 60 }, children: typeof children === "string" ? [r(children)] : children });

function table(widths, header, rows, { mono = [], size = 18 } = {}) {
  const total = widths.reduce((a, b) => a + b, 0);
  const cell = (text, w, isHead, colIdx) => new TableCell({
    width: { size: w, type: WidthType.DXA },
    borders,
    shading: isHead ? { fill: HEAD_FILL, type: ShadingType.CLEAR, color: "auto" } : { fill: UP_FILL, type: ShadingType.CLEAR, color: "auto" },
    margins: { top: 40, bottom: 40, left: 80, right: 80 },
    children: [new Paragraph({
      children: [new TextRun({
        text: String(text), bold: isHead, size: isHead ? size : (mono.includes(colIdx) ? size - 2 : size),
        font: !isHead && mono.includes(colIdx) ? MONO : FONT,
      })],
    })],
  });
  return new Table({
    width: { size: total, type: WidthType.DXA },
    columnWidths: widths,
    rows: [
      new TableRow({ tableHeader: true, children: header.map((h, i) => cell(h, widths[i], true, i)) }),
      ...rows.map((row) => new TableRow({ children: row.map((c, i) => cell(c, widths[i], false, i)) })),
    ],
  });
}

const RUN_W = [560, 760, 820, 900, 980, 900, 1700, 1000, 1000, 740];
const RUN_H = ["Run", "Dir", "Freq (Hz)", "Sync RPM", "Current (A)", "Steady rows", "Shaft RPM (display)", "Slip RPM", "Slip", "Run time (s)"];
const FILE_W = [700, 8660];
const spacer = () => new Paragraph({ spacing: { after: 60 }, children: [] });

const children = [];

// ---------- Title ----------
children.push(new Paragraph({
  alignment: AlignmentType.CENTER, spacing: { after: 80 },
  children: [new TextRun({ text: "No-Load Baseline Test Data", font: FONT, size: 40, bold: true })],
}));
children.push(new Paragraph({
  alignment: AlignmentType.CENTER, spacing: { after: 80 },
  children: [r("Sensorless Load Estimation Using Motor Slip on a VFD-Driven Induction Motor", { italics: true })],
}));
children.push(new Paragraph({
  alignment: AlignmentType.CENTER, spacing: { after: 280 },
  children: [r("Test date: October 5, 2026   |   Data sessions 20:57 – 21:40")],
}));

// ---------- 1. Purpose ----------
children.push(h1("1. Purpose"));
children.push(p("This document records every no-load baseline run from the first data-collection day. It gives the shaft RPM observed going up and going down, the slip worked out from it, the motor current, and the exact logger file each number came from, so results can be traced back to raw data when writing the paper. \"No load\" means only the hook and crane scale hang on the cable; no test weight is attached."));

// ---------- 2. Test setup ----------
children.push(h1("2. Test Setup"));
children.push(table([3000, 6360], ["Item", "Setting / value"], [
  ["Motor", "Lucas-Nülle SE2673-1K7, 0.37 kW, 4-pole, 1650 RPM @ 60 Hz, delta (208 V)"],
  ["Drive (VFD)", "AutomationDirect DURApulse GS23-21P0, scalar V/f"],
  ["Speed command", "Hand potentiometer on AI1 (P00.20 = 2); run from FWD/REV buttons (P00.21 = 1)"],
  ["Speed cap", "P01.10 output frequency upper limit = 10 Hz"],
  ["Accel / decel", "P01.12 = P01.13 = 1.5 s; stop method ramp (P00.22 = 0)"],
  ["Drive settings check", "Confirmed by the firmware's VFDCHECK over Modbus: 8/8 reads OK, PASS"],
  ["Load path", "Drum Drum_Rev_0: hourglass profile, 15.9 mm waist (≈1.96 in of cable per turn); 64 in usable cable travel"],
  ["Shaft speed reference", "Lucas-Nülle ActiveServo (SB2663-6U) display, read by eye"],
  ["Frequency & current", "Read from the drive over RS-485 Modbus RTU: registers 0x2103 (output frequency) and 0x2104 (output current), both ÷100"],
  ["Logging", "STM32 Nucleo-F401RE, one row every 250 ms; PC logger (vfd_logger.py) saves each motor run to its own CSV"],
  ["Baseline weight entered", "0 kg (hook + scale not weighed separately; see Limitations)"],
], { size: 18 }));
children.push(spacer());

// ---------- 3. Method ----------
children.push(h1("3. Method and Definitions"));
children.push(bullet([r("Synchronous speed: ", { bold: true }), r("sync RPM = 120 × f / poles = 30 × f for this 4-pole motor, using the drive's measured output frequency f.")]));
children.push(bullet([r("Slip RPM: ", { bold: true }), r("sync RPM − shaft RPM.")]));
children.push(bullet([r("Slip (per unit): ", { bold: true }), r("(sync RPM − shaft RPM) / sync RPM.")]));
children.push(bullet([r("Steady rows: ", { bold: true }), r("logged rows where output frequency was above 0.5 Hz and changed by no more than 0.05 Hz since the previous row. Each run's averages use only the steady rows at the highest speed held (within 0.2 Hz), so ramp-up, ramp-down and knob adjustments are excluded.")]));
children.push(bullet([r("Shaft RPM: ", { bold: true }), r("read by eye from the ActiveServo display while the motor ran. Where the reading moved, the range is given and its midpoint is used.")]));
children.push(bullet([r("Direction: ", { bold: true }), r("Up = lifting the hook, Down = lowering. Marked * where it is inferred from run order and current (up runs draw more current) rather than recorded.")]));

// ---------- 4. Summary ----------
children.push(h1("4. Results Summary"));
children.push(p("Values below are per direction at each test speed. Detailed per-run data and source files follow in Section 5."));
children.push(table(
  [880, 800, 1160, 800, 820, 1160, 800, 820, 1060, 1060],
  ["Freq (Hz)", "Sync RPM", "Up RPM", "Up slip RPM", "Up slip", "Down RPM", "Down slip RPM", "Down slip", "Up current (A)", "Down current (A)"],
  [
    ["1.39", "41.8", "30.5 (29–32)", "11.3", "0.270", "38", "3.8", "0.091", "0.476", "0.486"],
    ["2.44", "73.3", "60 (59–61)", "13.3", "0.182", "69.5 (69–70)", "3.8", "0.052", "0.466", "0.434"],
    ["4.85", "145.4", "137.5 (137–138)", "7.9", "0.054", "141", "4.4", "0.030", "0.814", "—"],
    ["6.36", "190.7", "180", "10.7", "0.056", "186", "4.7", "0.025", "0.704", "0.651"],
  ], { size: 17 }));
children.push(p([r("Earlier runs at 9–10 Hz were logged without an RPM reading: steady no-load current 0.65–0.67 A (Section 5.1).", { italics: true, size: 20 })], { spacing: { before: 80, after: 120 } }));

children.push(h2("Key observations"));
children.push(bullet("Slip depends strongly on direction even with no test weight. Going up, the motor lifts the hook and overcomes friction; going down, gravity helps. Up slip RPM was 1.8–3.5× the down slip RPM at every speed."));
children.push(bullet("Results repeat well at a fixed speed: at 6.36 Hz three up runs gave 0.055–0.056 slip and two down runs gave 0.025."));
children.push(bullet("Current is a much weaker signal than slip. At 6.36 Hz up vs down changed current by about 8% (0.704 vs 0.651 A) while slip changed by about 120%. At low frequency the current is mostly magnetizing current."));
children.push(bullet("No-load current rises with frequency (≈0.47 A at 1.4–2.4 Hz, ≈0.70–0.81 A at 4.9–6.4 Hz) because the drive applies more voltage under V/f."));
children.push(bullet("Down slip RPM stayed nearly constant across speed (3.8–4.7 RPM), but up slip RPM did not (7.9–13.3 RPM). At very low frequency the motor is weaker (stator resistance drop) and slips more for the same torque. Calibration should therefore be done at one fixed frequency."));
children.push(bullet("Friction vs gravity: the average of up and down slip RPM reflects friction, and half their difference reflects the hanging hook's weight. At 6.36 Hz that's ≈7.7 RPM friction and ≈3.0 RPM hook."));
children.push(bullet("Display resolution limits low-speed accuracy: ±1 RPM is ±2.4% slip at 1.39 Hz but only ±0.5% at 6.36 Hz. 5–6.4 Hz gives the best balance of accuracy and run time (6–10 s of travel)."));

// ---------- 5. Detailed data ----------
children.push(h1("5. Detailed Run Data and Source Files"));
children.push(p("All folders are under the logger's data folder (Documents\\VFD Logger\\data\\). Each session folder holds one CSV per run, a summary CSV, an all-rows CSV, and a session log."));

// 5.1 exploratory
children.push(h2("5.1 Exploratory sessions (no RPM recorded)"));
children.push(p("These sessions were used to learn the rig and logger. No shaft RPM was entered, so slip cannot be calculated, but frequency and current are valid."));
children.push(table([2400, 6960], ["Session / file", "What it contains"], [
  ["data\\session_20261005_205746\\all_rows.csv", "Four up/down runs at 10.00 Hz (4.4 s, 1.1 s, 5.2 s, 5.6 s). Steady no-load current 0.65–0.67 A. Logged before the TEST/auto-run features were in use (test_id = 0)."],
  ["Session started ≈21:01 (older logger), session_log.txt", "Runs at 10.00 Hz (≈0.66 A) and at 6.86 Hz (one run ≈0.65 A, another ≈0.70 A, likely down vs up)."],
  ["data\\2026-10-05_2114_base0kg\\ 2026-10-05_2114_summary_base0kg.csv", "16 automatically saved runs while practicing (knob moved during several runs). Best steady runs: run 13 at 9.08 Hz, 0.655 A (11 steady rows); run 9 at 3.72 Hz, 0.588 A (19 steady rows). Slow positioning moves at ≈1.25 Hz drew 0.54–0.58 A."],
], { size: 17, mono: [0] }));
children.push(spacer());

function session(title, folder, summary, note, runRows, files) {
  children.push(h2(title));
  children.push(p([r("Folder: ", { bold: true }), new TextRun({ text: folder, font: MONO, size: 18 })], { spacing: { after: 40 } }));
  children.push(p([r("Summary file: ", { bold: true }), new TextRun({ text: summary, font: MONO, size: 18 })], { spacing: { after: 100 } }));
  if (note) children.push(p([r(note, { italics: true, size: 20 })]));
  children.push(table(RUN_W, RUN_H, runRows, { size: 17 }));
  children.push(spacer());
  children.push(table(FILE_W, ["Run", "Run data file"], files, { size: 17, mono: [1] }));
  children.push(spacer());
}

session("5.2 Session 21:29 — 1.39 Hz", "data\\2026-10-05_2129_base0kg\\", "2026-10-05_2129_summary_base0kg.csv",
  "One up and one down run. The RPM \"30\" was typed after run 2, so in the summary file it is attached to run 2. It actually belongs to run 1 (up). The values below are corrected.",
  [
    ["1", "Up", "1.39", "41.8", "0.476", "140", "30.5 (29–32)", "11.3", "0.270", "35.4"],
    ["2", "Down", "1.39", "41.8", "0.486", "103", "38", "3.8", "0.091", "25.9"],
  ],
  [["1", "2026-10-05_212942_run01_load0kg_base0kg.csv"], ["2", "2026-10-05_213019_run02_load0kg_base0kg.csv"]]);

session("5.3 Session 21:33 — 2.44 Hz", "data\\2026-10-05_2133_base0kg\\", "2026-10-05_2133_summary_base0kg.csv",
  "Runs 3 and 5 were short repositioning bumps (no steady rows) and are excluded. Directions inferred*.",
  [
    ["1", "Up*", "2.45", "73.4", "0.463", "72", "60 (59–61)", "13.4", "0.182", "18.4"],
    ["2", "Down*", "2.44", "73.3", "0.434", "68", "69.5 (69–70)", "3.8", "0.052", "17.8"],
    ["4", "Up*", "2.44", "73.3", "0.469", "72", "60 (59–61)", "13.3", "0.181", "18.6"],
  ],
  [["1", "2026-10-05_213332_run01_load0kg_base0kg.csv"], ["2", "2026-10-05_213355_run02_load0kg_base0kg.csv"],
   ["3", "2026-10-05_213413_run03_load0kg_base0kg.csv  (bump, excluded)"], ["4", "2026-10-05_213414_run04_load0kg_base0kg.csv"],
   ["5", "2026-10-05_213433_run05_load0kg_base0kg.csv  (bump, excluded)"]]);

session("5.4 Session 21:36 — 4.85 Hz", "data\\2026-10-05_2136_base0kg\\", "2026-10-05_2136_summary_base0kg.csv",
  "Only one full run was logged (run 2). The down reading of 141 RPM has no matching logged run; its sync RPM is assumed equal to run 2's. Run 1 was a 1 s bump.",
  [
    ["2", "Up*", "4.85", "145.4", "0.814", "36", "137.5 (137–138)", "7.9", "0.054", "9.7"],
    ["—", "Down", "≈4.85", "≈145.4", "—", "—", "141", "4.4", "0.030", "—"],
  ],
  [["1", "2026-10-05_213701_run01_load0kg_base0kg.csv  (bump, excluded)"], ["2", "2026-10-05_213703_run02_load0kg_base0kg.csv"]]);

session("5.5 Session 21:39 — 6.36 Hz (recommended baseline)", "data\\2026-10-05_2139_base0kg\\", "2026-10-05_2139_summary_base0kg.csv",
  "Five back-to-back runs alternating up/down*. One RPM reading per direction was given for the session (up 180, down 186).",
  [
    ["1", "Up*", "6.36", "190.7", "0.700", "25", "180", "10.7", "0.056", "7.4"],
    ["2", "Down*", "6.36", "190.7", "0.660", "23", "186", "4.7", "0.025", "6.4"],
    ["3", "Up*", "6.35", "190.5", "0.692", "24", "180", "10.5", "0.055", "6.7"],
    ["4", "Down*", "6.36", "190.7", "0.641", "22", "186", "4.7", "0.025", "6.2"],
    ["5", "Up*", "6.35", "190.6", "0.720", "21", "180", "10.6", "0.056", "6.0"],
  ],
  [["1", "2026-10-05_213912_run01_load0kg_base0kg.csv"], ["2", "2026-10-05_213920_run02_load0kg_base0kg.csv"],
   ["3", "2026-10-05_213928_run03_load0kg_base0kg.csv"], ["4", "2026-10-05_213935_run04_load0kg_base0kg.csv"],
   ["5", "2026-10-05_213942_run05_load0kg_base0kg.csv"]]);

// ---------- 6. Limitations ----------
children.push(h1("6. Limitations and Notes for the Paper"));
children.push(bullet("Shaft RPM was read by eye from the ActiveServo display, which jittered by ±0.5 to ±1.5 RPM. This is the largest source of slip uncertainty, especially at low frequency."));
children.push(bullet("Directions marked * were inferred from run order and current, not recorded at the time."));
children.push(bullet("The hook + scale weight was entered as 0 kg rather than weighed, so \"no load\" includes the hook's own weight. That weight shows up as the up/down slip difference."));
children.push(bullet("The test frequency changed between sessions (1.39, 2.44, 4.85, 6.36 Hz) while a working speed was chosen. Results should only be compared within the same frequency."));
children.push(bullet("The ActiveServo's USB interface (USB\\VID_16C1&PID_2663) needs its Lucas-Nülle driver installed before it can log RPM automatically. Until then, RPM is entered by hand."));
children.push(bullet("Whether the drum is driven directly or through a belt/pulley ratio has not been confirmed. This affects cable speed and travel-time estimates, but not slip, which is measured at the motor shaft."));

// ---------- 7. Next steps ----------
children.push(h1("7. Next Steps"));
children.push(bullet("Keep the speed fixed at about 6.36 Hz (knob untouched) for all loaded tests."));
children.push(bullet("For each test weight: run up, down, up, down, up in one logger session (WEIGHT <kg> between weights), and note one RPM per direction."));
children.push(bullet("Weigh the hook + scale and enter it as the baseline weight."));
children.push(bullet("Plot up and down slip RPM against total weight to build the calibration curves; compare against current."));

const doc = new Document({
  creator: "Capstone project",
  title: "No-Load Baseline Test Data",
  styles: {
    default: { document: { run: { font: FONT, size: 22 } } },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 30, bold: true, color: "1F3864", font: FONT }, paragraph: { spacing: { before: 280, after: 120 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 25, bold: true, color: "2F5496", font: FONT }, paragraph: { spacing: { before: 220, after: 100 }, outlineLevel: 1 } },
    ],
  },
  numbering: { config: [{ reference: "bullets", levels: [{ level: 0, format: LevelFormat.BULLET, text: "•", alignment: AlignmentType.LEFT,
    style: { paragraph: { indent: { left: 540, hanging: 270 } } } }] }] },
  sections: [{
    properties: { page: { size: { width: 12240, height: 15840 }, margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 } } },
    footers: { default: new Footer({ children: [new Paragraph({ alignment: AlignmentType.CENTER,
      children: [new TextRun({ text: "No-Load Baseline Test Data — page ", font: FONT, size: 18, color: "808080" }),
                 new TextRun({ children: [PageNumber.CURRENT], font: FONT, size: 18, color: "808080" })] })] }) },
    children,
  }],
});

Packer.toBuffer(doc).then((buf) => { fs.writeFileSync(OUT, buf); console.log("wrote", OUT); });
