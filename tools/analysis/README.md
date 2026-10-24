# Analysis

`analyze.py` turns logger summaries plus hand-recorded shaft RPM into slip results and plots.
Reads both the current `*_summary_*.xlsx` workbooks (needs `pip install openpyxl`) and the
older `*_summary_*.csv` files from before the logger switched to Excel output.

1. Copy the session summary files into a results folder (e.g. `results/2026-10-05/`).
2. Add `rpm_notes.csv` there, one line per run you read the RPM for:
   `session,run,direction,rpm_low,rpm_high,direction_source,note`
   (`session` is the summary file's date-time prefix, e.g. `2026-10-05_2139`).
3. Run `python analyze.py ../../results/2026-10-05` (plots need `pip install matplotlib`).

Output goes to `<folder>/analysis/`: `runs.csv` (per run), `groups.csv` (mean ± std per
frequency, direction and load), and PNG plots. With only no-load data it plots slip and
current against frequency. Once there are several loads, it plots calibration curves
(slip RPM and current against load) for each test frequency.
