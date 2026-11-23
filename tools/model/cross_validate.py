"""Leave-one-out cross-validation (LOOCV) of load-estimation model forms, for
Table XV / Fig. 14 of the IEEE paper.

Compares the already-deployed per-frequency model (one independent current-vs-load
line per test frequency, as fit_model.py builds and firmware/the PC script actually
use) against four single global regressions of increasing complexity that combine
current and the RAW (unbucketed) commanded frequency as continuous regressors
instead of splitting the data by test frequency:

  deployed  load = a_f*current + b_f          (one (a_f, b_f) per nearest test freq.)
  model 1   load = a*current + b*freq + c
  model 2   load = a*current + b*freq + c*(current*freq) + d
  model 3   load = a*current + b*freq + c*freq^2 + d
  model 4   load = a*current + b*freq + c*(current*freq) + d*freq^2 + e

Every prediction is floored at 0 kg, matching fit_model.py's predict(). MAPE is
computed only over the folds whose true load is nonzero (4 of 17 points are a
0 kg baseline reading, at which percentage error is undefined).

Usage:
  python tools/model/cross_validate.py
"""

import json
import os

from fit_model import TARGET_FREQS_HZ, load_points, nearest_target, fit_line

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def load_raw_points():
    """Same calibration rows as fit_model.load_points(), but keeping each point's
    own measured frequency (not bucketed to the nearest target) for the global
    model forms, which take frequency as a continuous input."""
    import csv
    import glob
    rows = []
    for path in sorted(glob.glob(os.path.join(ROOT, "results", "*", "analysis", "groups.csv"))):
        with open(path, newline="", encoding="utf-8") as fh:
            for row in csv.DictReader(fh):
                if row["direction"] != "up":
                    continue
                current = row.get("current_mean", "")
                if current in ("", None):
                    continue
                rows.append((float(row["freq_hz"]), float(current), float(row["total_kg"])))
    return rows


def design_row(freq, current, terms):
    values = {"1": 1.0, "c": current, "f": freq, "cf": current * freq, "f2": freq * freq}
    return [values[t] for t in terms]


def solve_least_squares(X, y):
    """Normal-equations solve of (X^T X) beta = X^T y via Gaussian elimination."""
    m = len(X[0])
    XtX = [[sum(X[k][i] * X[k][j] for k in range(len(X))) for j in range(m)] for i in range(m)]
    Xty = [sum(X[k][i] * y[k] for k in range(len(X))) for i in range(m)]
    A = [row[:] + [Xty[i]] for i, row in enumerate(XtX)]
    for col in range(m):
        pivot_row = max(range(col, m), key=lambda r: abs(A[r][col]))
        A[col], A[pivot_row] = A[pivot_row], A[col]
        pivot = A[col][col]
        for j in range(col, m + 1):
            A[col][j] /= pivot
        for r in range(m):
            if r != col:
                factor = A[r][col]
                for j in range(col, m + 1):
                    A[r][j] -= factor * A[col][j]
    return [A[i][m] for i in range(m)]


def mae_mape(abs_errors, true_values):
    mae = sum(abs_errors) / len(abs_errors)
    nonzero = [(e, t) for e, t in zip(abs_errors, true_values) if t != 0]
    mape = sum(e / t for e, t in nonzero) / len(nonzero) * 100
    return round(mae, 3), round(mape, 1), len(nonzero)


def loocv_deployed_model(bucketed_points):
    """LOOCV of the real per-frequency model: for each point, refit only its own
    frequency's line on the other points in that bucket, matching what
    fit_model.build_model()/predict() do."""
    abs_errors, true_values = [], []
    for freq in TARGET_FREQS_HZ:
        bucket = bucketed_points[freq]
        for i in range(len(bucket)):
            train = [p for j, p in enumerate(bucket) if j != i]
            test_current, test_load = bucket[i]
            if len(train) < 2:
                continue
            a, b, _ = fit_line(train)
            prediction = max(a * test_current + b, 0.0)
            abs_errors.append(abs(prediction - test_load))
            true_values.append(test_load)
    return mae_mape(abs_errors, true_values)


def loocv_global_model(raw_points, terms):
    """LOOCV of a single global regression over all 17 points at once, using
    terms from design_row (frequency taken as-measured, not bucketed)."""
    abs_errors, true_values = [], []
    for i in range(len(raw_points)):
        train = [raw_points[j] for j in range(len(raw_points)) if j != i]
        test_freq, test_current, test_load = raw_points[i]
        X = [design_row(f, c, terms) for f, c, l in train]
        y = [l for f, c, l in train]
        beta = solve_least_squares(X, y)
        prediction = max(sum(b * x for b, x in zip(beta, design_row(test_freq, test_current, terms))), 0.0)
        abs_errors.append(abs(prediction - test_load))
        true_values.append(test_load)
    return mae_mape(abs_errors, true_values)


MODEL_FORMS = [
    ("deployed", "Per-frequency (deployed, Table X)", None),
    ("model1", "current + freq (linear)", ["1", "c", "f"]),
    ("model2", "+ interaction term", ["1", "c", "f", "cf"]),
    ("model3", "+ quadratic freq", ["1", "c", "f", "f2"]),
    ("model4", "full (interaction + quadratic)", ["1", "c", "f", "cf", "f2"]),
]


def main():
    bucketed_points = load_points()
    raw_points = load_raw_points()
    n_total = sum(len(v) for v in bucketed_points.values())

    results = []
    for key, label, terms in MODEL_FORMS:
        if terms is None:
            mae, mape, n_nonzero = loocv_deployed_model(bucketed_points)
        else:
            mae, mape, n_nonzero = loocv_global_model(raw_points, terms)
        results.append({"key": key, "label": label, "mae_kg": mae, "mape_pct": mape})
        print(f"{label:38s}  MAE={mae:.3f} kg  MAPE={mape:.1f}%  (n={n_total}, nonzero n={n_nonzero})")

    out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "cross_validation.json")
    with open(out_path, "w", encoding="utf-8") as fh:
        json.dump({
            "description": "LOOCV MAE/MAPE across all 17 real calibration points, deployed "
                            "per-frequency model vs. four global current+frequency regressions. "
                            "See cross_validate.py docstring for exact model forms. MAPE excludes "
                            "the 4 of 17 points whose true load is 0 kg.",
            "n_points": n_total,
            "results": results,
        }, fh, indent=2)
        fh.write("\n")
    print(f"\nwrote {out_path}")


if __name__ == "__main__":
    main()
