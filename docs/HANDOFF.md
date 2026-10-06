# Handoff: state of the project (from session of Oct 5–6, 2026)

Read this first in a new chat. All work is on branch `claude/elegant-noether-3qu8y3`.

## Rig and drive (working, verified on hardware)
- GS23-21P0 drive: pot speed on AI1 (P00.20=2), FWD/REV run (P00.21=1), AI1=freq (P03.00=1),
  **10 Hz cap (P01.10)**, accel/decel 1.5 s (P01.12/13), ramp stop (P00.22=0). Set because an
  uncapped run let the hoist reach the end of its 64 in cable before it could be stopped.
- Drum `Drum_Rev_0`: hourglass, 15.9 mm waist (≈1.96 in cable per turn). Direct drive assumed (unconfirmed).
- Shaft speed: Lucas-Nülle **ActiveServo SB2663-6U** (USB, driver not installed → RPM read by eye).
- No test weights yet: crane scale has no batteries. Plan: water by volume or labeled weights, ≤5 kg.

## Firmware `firmware/stm32/Core/Src/main.c` (860 lines, STM32CubeIDE, Nucleo-F401RE)
- Read-only Modbus: freq 0x2103, current 0x2104 (both ÷100), pot setpoint 0x2102; logs every 250 ms,
  CSV ends with `test_id,steady`. `VFDCHECK` reads the drive settings at boot. LD2 LED shows status.
- Output goes to the USART2 VCP **and** the SWV ITM console (SYS Debug = Trace Asynchronous Sw, ITM port 0).
- Latest version fixes commands running twice on \r\n. **User may not have flashed it yet.**
- User workflow: they replace whole files in the IDE; give them complete downloadable files.
  Pitfall seen: they once pasted main.c over `Drivers/.../stm32f4xx_hal.c` (restored from ST GitHub).

## PC logger `tools/logger/` (`run_logger.bat` → `vfd_logger.py`)
- Asks baseline weight + test weight, then **saves every motor run automatically** to
  `data/<date>_<time>_base<kg>/…_runNN_load<kg>_base<kg>.csv` + a summary CSV.
- After each run it asks for the SERVO RPM (`291 up` / Enter to skip). `WEIGHT <kg>` changes load.
- The user usually tells Claude the RPMs instead of typing them.

## Results so far: no-load baseline (`results/2026-10-05/`, `docs/results-log.md`)
| Hz | Up slip RPM | Down slip RPM | Up / down current (A) |
|---|---|---|---|
| 1.39 | 11.3 | 3.8 | 0.48 / 0.49 |
| 2.45 | 13.4 | 3.8 | 0.47 / 0.43 |
| 4.85 | 7.9 | 4.4 | 0.81 / — |
| 6.36 | 10.6 | 4.7 | 0.70 / 0.65 |
- Proposed fixed test speed: **6.36 Hz** (repeatable, ±0.26% slip reading error, 6–7 s runs).
- Slip RPM is not constant across speed → calibrate at one frequency.
- Prediction: ≈5.5–6.5 slip RPM per kg; half the up/down difference gives ≈0.5 kg for the hook.

## Documents
- `docs/Baseline_Test_Data_2026-10-05.docx`: every baseline run with its source file.
- `docs/Paper_Draft_Theory_and_Method.docx`: theory and method draft.
- `tools/analysis/analyze.py`: summaries + `rpm_notes.csv` → slip tables and plots.
- Status deck for the professor: https://claude.ai/artifact/UrpdUiizgDEeCjcSPYPxL2

## Next steps
1. Weighted tests at 6.36 Hz: up/down ×5 per mass; 10+10 no-load runs for uncertainty.
2. Add new RPMs to `rpm_notes.csv`, run the analysis, update the docs.
3. Install the ActiveServo driver/software for automatic RPM (and possibly torque loading).
4. Optional: log direction from the drive status register; end-of-travel limit switch on DI4.
