// Generates docs/Paper_Draft_Theory_and_Method.docx (node, docx package).
const fs = require("fs");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell, WidthType,
  ShadingType, HeadingLevel, AlignmentType, BorderStyle, LevelFormat, Footer, PageNumber,
} = require("docx");

const OUT = process.argv[2] || "Paper_Draft_Theory_and_Method.docx";
const FONT = "Calibri";
const border = { style: BorderStyle.SINGLE, size: 4, color: "A6A6A6" };
const borders = { top: border, bottom: border, left: border, right: border };

const r = (text, o = {}) => new TextRun({ text, font: FONT, size: 22, ...o });
const p = (runs, opts = {}) => new Paragraph({ spacing: { after: 140, line: 300 }, ...opts,
  children: typeof runs === "string" ? [r(runs)] : runs });
const h1 = (t) => new Paragraph({ heading: HeadingLevel.HEADING_1, children: [new TextRun({ text: t, font: FONT })] });
const h2 = (t) => new Paragraph({ heading: HeadingLevel.HEADING_2, children: [new TextRun({ text: t, font: FONT })] });
const eq = (text, label) => new Paragraph({
  alignment: AlignmentType.CENTER, spacing: { before: 60, after: 160 },
  children: [new TextRun({ text, font: "Cambria Math", size: 24, italics: true }),
             new TextRun({ text: `\t(${label})`, font: FONT, size: 22 })],
  tabStops: [{ type: "right", position: 9360 }],
});
const bullet = (runs) => new Paragraph({ numbering: { reference: "bullets", level: 0 }, spacing: { after: 80, line: 290 },
  children: typeof runs === "string" ? [r(runs)] : runs });
const note = (text) => p([r(text, { italics: true, color: "7F6000" })], {
  shading: { fill: "FFF4CE", type: ShadingType.CLEAR, color: "auto" }, spacing: { before: 60, after: 160 } });

function table(widths, header, rows) {
  const cell = (text, w, head) => new TableCell({
    width: { size: w, type: WidthType.DXA }, borders,
    shading: { fill: head ? "D9E2F3" : "FFFFFF", type: ShadingType.CLEAR, color: "auto" },
    margins: { top: 40, bottom: 40, left: 80, right: 80 },
    children: [new Paragraph({ children: [new TextRun({ text: String(text), bold: head, font: FONT, size: 19 })] })],
  });
  return new Table({
    width: { size: widths.reduce((a, b) => a + b, 0), type: WidthType.DXA }, columnWidths: widths,
    rows: [new TableRow({ tableHeader: true, children: header.map((h, i) => cell(h, widths[i], true)) }),
           ...rows.map((row) => new TableRow({ children: row.map((c, i) => cell(c, widths[i], false)) }))],
  });
}

const c = [];
c.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 80 },
  children: [new TextRun({ text: "Paper Draft: Theory and Method", font: FONT, size: 40, bold: true })] }));
c.push(new Paragraph({ alignment: AlignmentType.CENTER, spacing: { after: 240 },
  children: [r("Sensorless Load Estimation Using Motor Slip on a Low-Cost VFD-Driven Induction Motor", { italics: true })] }));
c.push(note("Working draft. Yellow boxes mark assumptions to confirm or numbers to fill in before submission. Equation numbers are provisional."));

// ---------------- THEORY ----------------
c.push(h1("2. Theory"));
c.push(h2("2.1 Slip"));
c.push(p("An induction motor's rotor turns slightly slower than the rotating magnetic field set up by the stator. The field turns at the synchronous speed, which for a supply frequency f and P poles is:"));
c.push(eq("n_s = 120 f / P", "1"));
c.push(p("For the 4-pole test motor, n_s = 30 f RPM, e.g. 190.7 RPM at 6.36 Hz. The difference between synchronous speed and the measured shaft speed n is the slip, expressed either per unit or in RPM:"));
c.push(eq("s = (n_s − n) / n_s,        n_slip = n_s − n", "2"));
c.push(p("Slip is what lets the rotor conductors cut flux and carry current, so it rises with the torque the motor must deliver. That is the basis of this project: if slip can be measured, the load torque, and from it the hanging mass, can be estimated without a torque sensor."));

c.push(h2("2.2 Torque and slip under scalar V/f control"));
c.push(p("In the normal operating region (small slip), the equivalent circuit reduces to the rotor resistance R₂′ dominating the rotor branch, and electromagnetic torque is approximately proportional to slip:"));
c.push(eq("T ≈ (3 V² / (ω_s R₂′)) · s", "3"));
c.push(p("where V is the phase voltage and ω_s = 4πf/P is the synchronous speed in rad/s. A scalar V/f drive holds V/f roughly constant (V = k f), so the air-gap flux is roughly constant. Substituting V = k f and ω_s = 4πf/P into (3):"));
c.push(eq("T ≈ (3 P k² / (4π R₂′)) · (s f)  ∝  n_slip", "4"));
c.push(p("Because s·f is the rotor (slip) frequency, and n_slip = 60·s·f/(P/2), torque is ideally proportional to slip in RPM and independent of the drive frequency. Two practical effects break this ideal:"));
c.push(bullet([r("Stator resistance drop at low frequency. ", { bold: true }), r("At low f the stator voltage is small, and the I·R₁ drop takes a larger share of it. The air-gap flux falls, so the motor needs more slip for the same torque. The drive's low-frequency voltage boost only partly compensates.")]));
c.push(bullet([r("Rotor resistance with temperature. ", { bold: true }), r("R₂′ rises as the rotor warms, increasing slip for the same torque. Long test sessions should be checked for drift.")]));
c.push(p("The baseline data shows the first effect: down-direction slip was nearly constant across 1.4–6.4 Hz (3.8–4.7 RPM), but up-direction slip varied from 7.9 to 13.4 RPM. Calibration is therefore done at one fixed frequency."));

c.push(h2("2.3 Expected sensitivity"));
c.push(p("A rough sensitivity follows from the motor nameplate, assuming the linear relation (4) holds down to light load. Rated torque is T_r = P_out / ω = 370 W / (1650 RPM × 2π/60) = 2.14 N·m at a rated slip of 1800 − 1650 = 150 RPM. That gives:"));
c.push(eq("k_T ≈ 2.14 / 150 ≈ 0.0143 N·m per slip RPM", "5"));
c.push(p("A mass m hanging on a drum of effective radius r needs a torque m·g·r. For the drum's 15.9 mm waist (r ≈ 8–9.5 mm including the cable), each kilogram therefore adds about:"));
c.push(eq("Δn_slip / Δm = g · r / k_T ≈ 5.5 – 6.5 RPM per kg", "6"));
c.push(note("Assumes the drum is driven directly by the motor (no belt or pulley ratio; confirm) and ideal V/f behavior at the test frequency. Use the measured cable diameter for r. The weighted calibration tests check this prediction."));

c.push(h2("2.4 Separating load from friction using direction"));
c.push(p("On a hoist, gravity acts against the motor when lifting and with it when lowering, while friction (bearings, coupling, drum, cable) always opposes motion. With a friction torque T_f and a hanging mass m:"));
c.push(eq("T_up = T_f + m g r,        T_down = T_f − m g r", "7"));
c.push(p("so, with slip RPM proportional to torque, the friction and gravity parts separate:"));
c.push(eq("(n_up + n_down) / 2 ∝ T_f,        (n_up − n_down) / 2 ∝ m g r", "8"));
c.push(p("Half the up/down difference depends only on the hanging mass, which cancels friction without modeling it. In the no-load baseline at 6.36 Hz (up 10.6, down 4.7 slip RPM), half the difference is 3.0 RPM. With (6) this corresponds to roughly 0.5 kg, a plausible weight for the hook and crane scale that were on the cable. The friction share is about 7.7 RPM (≈0.11 N·m). A positive down-direction slip also shows that friction exceeded the hook's gravity torque, so the motor was still driving while lowering."));
c.push(note("Weigh the hook + crane scale to test the ≈0.5 kg estimate. This makes a useful first validation point."));

c.push(h2("2.5 Why current is a weaker load signal"));
c.push(p("Stator current is the vector sum of a magnetizing component I_m, set by the flux, and a torque-producing component I_T:"));
c.push(eq("I = √(I_m² + I_T²)", "9"));
c.push(p("At light load I_m dominates, so dI/dT ≈ I_T/I · dI_T/dT is close to zero: current barely changes until the load is a meaningful fraction of rated. Under V/f, I_m also rises with frequency because the drive raises the voltage, so current mixes load with speed. The baseline shows both effects. At 6.36 Hz, up versus down changed slip by a factor of 2.3 but current by only 8% (0.704 vs 0.651 A). No-load current also rose from about 0.47 A at 1.4–2.4 Hz to 0.70–0.81 A at 4.9–6.4 Hz. Current should become more useful at heavier loads, where I_T grows; comparing the two estimates across the load range is part of the evaluation."));

// ---------------- METHOD ----------------
c.push(h1("3. Method"));
c.push(h2("3.1 Test rig"));
c.push(table([2800, 6560], ["Component", "Details"], [
  ["Motor", "Lucas-Nülle SE2673-1K7: 0.37 kW, 4-pole, 1650 RPM at 60 Hz, delta-connected (208 V, 1.8 A)"],
  ["Drive", "AutomationDirect DURApulse GS23-21P0, scalar V/f, motor parameters entered per nameplate"],
  ["Load", "Hanging mass on a cable wound on drum Drum_Rev_0 (hourglass profile, 15.9 mm waist, 50.8 mm flanges), 64 in of usable travel"],
  ["Shaft speed", "Lucas-Nülle ActiveServo (SB2663-6U) display"],
  ["Data acquisition", "STM32 Nucleo-F401RE reading the drive over RS-485 Modbus RTU (9600 baud, 8N2) through a MAX485 transceiver; PC logger over USB"],
]));
c.push(p("", { spacing: { after: 60 } }));

c.push(h2("3.2 Drive configuration and safety"));
c.push(p("Speed is commanded by a potentiometer on analog input AI1 (P00.20 = 2), and runs are started from maintained FWD/REV switches (P00.21 = 1). Following an early run in which the hoist reached the end of its cable faster than it could be stopped, the output frequency was capped at 10 Hz (P01.10), acceleration and deceleration were set to 1.5 s (P01.12/P01.13), and a ramp stop was selected (P00.22 = 0). At power-up the acquisition firmware reads these settings back over Modbus and reports any mismatch (VFDCHECK). The firmware only reads from the drive and never commands it."));

c.push(h2("3.3 Measurements"));
c.push(bullet([r("Output frequency and current ", { bold: true }), r("are read from drive registers 0x2103 and 0x2104 (both scaled ÷100) every 250 ms. Current scaling was confirmed against the vendor's GSoft2 monitor (raw 69 = 0.69 A).")]));
c.push(bullet([r("Synchronous speed ", { bold: true }), r("is computed from the measured output frequency with (1), not from the commanded frequency.")]));
c.push(bullet([r("Shaft speed ", { bold: true }), r("is currently read by the operator from the ActiveServo display during the steady part of each run. Where the reading fluctuated, the range is recorded and its midpoint used.")]));
c.push(bullet([r("Test mass ", { bold: true }), r("is recorded per run, together with a baseline mass for the hook and scale.")]));
c.push(note("Planned improvement: log ActiveServo speed (and torque) automatically over its USB interface once the Lucas-Nülle driver is installed. Shaft RPM would then be available on every 250 ms row."));

c.push(h2("3.4 Procedure"));
c.push(bullet("Set the potentiometer to the test frequency (6.36 Hz) and leave it unchanged for the whole calibration."));
c.push(bullet("For each mass, run up, down, up, down, up, holding each run for its full travel (about 6–7 s at 6.36 Hz), and record the shaft RPM and direction."));
c.push(bullet("Repeat the no-load condition 10 times in each direction to quantify run-to-run scatter."));
c.push(bullet("The PC logger saves each motor run to its own CSV file and writes one summary line per run."));
c.push(note("Masses: [list the masses used, e.g. water by volume at 1.00 kg/L, or labeled weights], 0.5–5 kg, below [cable rating]."));

c.push(h2("3.5 Data processing"));
c.push(p("A logged row is steady when the output frequency is above 0.5 Hz and has changed by no more than 0.05 Hz since the previous row. For each run, only steady rows within 0.2 Hz of the highest steady frequency are averaged, which excludes ramp-up, ramp-down and pauses while setting the knob. Slip and slip RPM follow from (2) using the run's average synchronous speed and the recorded shaft RPM. Runs are grouped by test frequency, direction and total mass, and the group mean and standard deviation are reported (tools/analysis/analyze.py)."));

c.push(h2("3.6 Uncertainty"));
c.push(p("The dominant uncertainty is the shaft speed reading. The display resolves 1 RPM and fluctuated by up to ±1.5 RPM, so a reading carries at least ±0.5 RPM. As slip per unit this is ±0.5/n_s: ±1.2% at 1.39 Hz, ±0.7% at 2.45 Hz and ±0.26% at 6.36 Hz. This is one reason for testing at the higher frequency. In slip RPM, which is the quantity used for calibration, it is ±0.5 RPM, or about ±0.1 kg using (6). Frequency and current are read digitally from the drive at 0.01 Hz and 0.01 A resolution."));

c.push(h2("3.7 Baseline results to date"));
c.push(table([1300, 1300, 1500, 1500, 1880, 1880], ["Frequency", "Sync RPM", "Up slip RPM", "Down slip RPM", "Up current (A)", "Down current (A)"], [
  ["1.39 Hz", "41.8", "11.3", "3.8", "0.476", "0.486"],
  ["2.45 Hz", "73.4", "13.4", "3.8", "0.466", "0.434"],
  ["4.85 Hz", "145.4", "7.9", "4.4", "0.814", "—"],
  ["6.36 Hz", "190.7", "10.6", "4.7", "0.704", "0.651"],
]));
c.push(p([r("Full run-by-run data with source files: Baseline_Test_Data_2026-10-05.docx.", { italics: true })], { spacing: { before: 100 } }));

const doc = new Document({
  creator: "Capstone project", title: "Paper Draft: Theory and Method",
  styles: {
    default: { document: { run: { font: FONT, size: 22 } } },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 32, bold: true, color: "1F3864", font: FONT }, paragraph: { spacing: { before: 320, after: 140 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 26, bold: true, color: "2F5496", font: FONT }, paragraph: { spacing: { before: 240, after: 100 }, outlineLevel: 1 } },
    ],
  },
  numbering: { config: [{ reference: "bullets", levels: [{ level: 0, format: LevelFormat.BULLET, text: "•", alignment: AlignmentType.LEFT,
    style: { paragraph: { indent: { left: 540, hanging: 270 } } } }] }] },
  sections: [{
    properties: { page: { size: { width: 12240, height: 15840 }, margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 } } },
    footers: { default: new Footer({ children: [new Paragraph({ alignment: AlignmentType.CENTER, children: [
      new TextRun({ text: "Draft — Theory and Method — page ", font: FONT, size: 18, color: "808080" }),
      new TextRun({ children: [PageNumber.CURRENT], font: FONT, size: 18, color: "808080" })] })] }) },
    children: c,
  }],
});
Packer.toBuffer(doc).then((b) => { fs.writeFileSync(OUT, b); console.log("wrote", OUT); });
