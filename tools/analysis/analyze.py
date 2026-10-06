"""Slip / current analysis for the load-estimation rig.

Reads a results folder containing:
  *_summary_*.csv   summary files written by tools/logger/vfd_logger.py (one per session)
  rpm_notes.csv     shaft RPM read off the ActiveServo display, per run:
                    session,run,direction,rpm_low,rpm_high,direction_source,note

and writes to <folder>/analysis/:
  runs.csv          every run that has an RPM note, with slip and slip RPM
  groups.csv        mean / std / n per (frequency, direction, total load)
  *.png             plots (needs matplotlib: pip install matplotlib)

Usage:
  python analyze.py ../../results/2026-10-05
"""

import argparse
import csv
import glob
import math
import os
import statistics
import sys
from collections import defaultdict

UP_COLOR = "#2a78d6"     # categorical slot 1
DOWN_COLOR = "#eb6834"   # categorical slot 2
INK = "#0b0b0b"
INK_2 = "#52514e"
GRID = "#e4e3df"
SURFACE = "#fcfcfb"
# The display shows whole RPM, so a single reading is uncertain by at least +/-0.5.
MIN_RPM_HALF_RANGE = 0.5
# Runs whose frequencies are within this of each other are one test speed (Hz).
FREQ_CLUSTER_HZ = 0.2


def fnum(text):
    try:
        return float(text)
    except (TypeError, ValueError):
        return None


def session_of(path):
    # "2026-10-05_2139_summary_base0kg.csv" -> "2026-10-05_2139"
    name = os.path.basename(path)
    return name.split("_summary_")[0]


def load_summaries(folder):
    runs = {}
    for path in sorted(glob.glob(os.path.join(folder, "*_summary_*.csv"))):
        session = session_of(path)
        with open(path, newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                runs[(session, row["run"])] = dict(row, session=session, summary_file=os.path.basename(path))
    return runs


def load_notes(folder):
    path = os.path.join(folder, "rpm_notes.csv")
    if not os.path.exists(path):
        sys.exit(f"missing {path}")
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def build_runs(summaries, notes):
    out = []
    for n in notes:
        key = (n["session"], n["run"])
        s = summaries.get(key)
        if s is None:
            print(f"!! {key}: in rpm_notes.csv but not in any summary file - skipped")
            continue
        sync = fnum(s["sync_rpm"])
        lo, hi = fnum(n["rpm_low"]), fnum(n["rpm_high"])
        if sync is None or lo is None or hi is None:
            print(f"!! {key}: missing sync RPM or RPM reading - skipped")
            continue
        rpm = (lo + hi) / 2
        half = max((hi - lo) / 2, MIN_RPM_HALF_RANGE)
        load = fnum(s.get("load_kg")) or 0.0
        base = fnum(s.get("baseline_kg")) or 0.0
        out.append({
            "session": n["session"], "run": n["run"], "direction": n["direction"],
            "direction_source": n.get("direction_source", ""),
            "freq_hz": fnum(s["avg_freq_hz"]), "sync_rpm": sync,
            "current_a": fnum(s["avg_current_a"]), "steady_rows": s["steady_rows"],
            "load_kg": load, "baseline_kg": base, "total_kg": load + base,
            "rpm": rpm, "rpm_unc": half,
            "slip_rpm": sync - rpm, "slip": (sync - rpm) / sync, "slip_unc": half / sync,
            "file": s["file"], "note": n.get("note", ""),
        })
    return out


def freq_clusters(freqs):
    # Group nearby frequencies (e.g. 6.35 and 6.36 Hz) into one test speed, labelled
    # by their mean, so rounding never splits a single knob setting in two.
    labels = {}
    cluster = []
    for f in sorted(set(freqs)) + [math.inf]:
        if cluster and f - cluster[-1] > FREQ_CLUSTER_HZ:
            mean = round(statistics.fmean(cluster), 2)
            labels.update({c: mean for c in cluster})
            cluster = []
        cluster.append(f)
    return labels


def group(runs):
    labels = freq_clusters([r["freq_hz"] for r in runs])
    groups = defaultdict(list)
    for r in runs:
        groups[(labels[r["freq_hz"]], r["direction"], r["total_kg"])].append(r)
    rows = []
    for (fbin, direction, total), rs in sorted(groups.items()):
        def stats(key):
            vals = [r[key] for r in rs if r[key] is not None]
            mean = statistics.fmean(vals)
            std = statistics.stdev(vals) if len(vals) > 1 else 0.0
            return mean, std
        slip_m, slip_s = stats("slip")
        srpm_m, srpm_s = stats("slip_rpm")
        cur_m, cur_s = stats("current_a")
        rows.append({
            "freq_hz": fbin, "direction": direction, "total_kg": total, "n": len(rs),
            "slip_mean": slip_m, "slip_std": slip_s,
            "slip_rpm_mean": srpm_m, "slip_rpm_std": srpm_s,
            "current_mean": cur_m, "current_std": cur_s,
            # reading uncertainty of the display, as slip (largest in the group)
            "slip_reading_unc": max(r["slip_unc"] for r in rs),
        })
    return rows


def write_csv(path, rows, fmt):
    if not rows:
        return
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(rows[0].keys())
        for r in rows:
            w.writerow([fmt(v) for v in r.values()])


def fmt(v):
    if isinstance(v, float):
        return f"{v:.4f}" if abs(v) < 10 else f"{v:.2f}"
    return v


# --- plots ---------------------------------------------------------------------
def style_axes(ax):
    ax.set_facecolor(SURFACE)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for side in ("left", "bottom"):
        ax.spines[side].set_color(INK_2)
    ax.tick_params(colors=INK_2, labelsize=9)
    ax.grid(True, color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)


def plot_vs(groups_rows, x_key, y_key, y_err_key, xlabel, ylabel, title, path, plt):
    fig, ax = plt.subplots(figsize=(6.4, 3.8), dpi=150)
    fig.patch.set_facecolor(SURFACE)
    style_axes(ax)
    for direction, color, label in (("up", UP_COLOR, "Up (lifting)"), ("down", DOWN_COLOR, "Down (lowering)")):
        pts = sorted((g[x_key], g[y_key], g[y_err_key]) for g in groups_rows if g["direction"] == direction)
        if not pts:
            continue
        xs, ys, es = zip(*pts)
        ax.errorbar(xs, ys, yerr=es, color=color, linewidth=2, marker="o", markersize=7,
                    markeredgecolor=SURFACE, markeredgewidth=2, capsize=3, label=label, zorder=3)
        ax.annotate(label.split(" ")[0], (xs[-1], ys[-1]), xytext=(8, 0), textcoords="offset points",
                    color=INK_2, fontsize=9, va="center")
    ax.set_xlabel(xlabel, color=INK, fontsize=10)
    ax.set_ylabel(ylabel, color=INK, fontsize=10)
    ax.set_title(title, color=INK, fontsize=11, loc="left")
    ax.legend(frameon=False, fontsize=9, labelcolor=INK_2)
    ax.set_ylim(bottom=0)
    fig.tight_layout()
    fig.savefig(path, facecolor=SURFACE)
    plt.close(fig)
    print(f"   {path}")


def make_plots(groups_rows, out_dir):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print(">> matplotlib not installed - skipping plots (pip install matplotlib)")
        return
    loads = sorted({g["total_kg"] for g in groups_rows})
    print(">> plots:")
    if len(loads) == 1:
        # Baseline only: show how slip and current behave across test frequency.
        sub = [dict(g, slip_err=max(g["slip_std"], g["slip_reading_unc"])) for g in groups_rows]
        for g in sub:
            g["slip_rpm_err"] = max(g["slip_rpm_std"], g["slip_reading_unc"] * g["freq_hz"] * 30)
        plot_vs(sub, "freq_hz", "slip_rpm_mean", "slip_rpm_err", "Drive output frequency (Hz)",
                "Slip (RPM)", f"No-load slip RPM vs frequency (total load {loads[0]:g} kg)",
                os.path.join(out_dir, "baseline_slip_rpm_vs_freq.png"), plt)
        plot_vs(sub, "freq_hz", "slip_mean", "slip_err", "Drive output frequency (Hz)",
                "Slip (per unit)", "No-load slip vs frequency",
                os.path.join(out_dir, "baseline_slip_vs_freq.png"), plt)
        plot_vs(sub, "freq_hz", "current_mean", "current_std", "Drive output frequency (Hz)",
                "Output current (A)", "No-load current vs frequency",
                os.path.join(out_dir, "baseline_current_vs_freq.png"), plt)
        return
    # Loaded data: one calibration plot per test frequency.
    for fbin in sorted({g["freq_hz"] for g in groups_rows}):
        sub = [dict(g, slip_rpm_err=max(g["slip_rpm_std"], g["slip_reading_unc"] * g["freq_hz"] * 30))
               for g in groups_rows if g["freq_hz"] == fbin]
        if len({g["total_kg"] for g in sub}) < 2:
            continue
        tag = f"{fbin:g}Hz".replace(".", "p")
        plot_vs(sub, "total_kg", "slip_rpm_mean", "slip_rpm_err", "Total hanging load (kg)",
                "Slip (RPM)", f"Slip RPM vs load at {fbin:g} Hz",
                os.path.join(out_dir, f"calibration_slip_rpm_{tag}.png"), plt)
        plot_vs(sub, "total_kg", "current_mean", "current_std", "Total hanging load (kg)",
                "Output current (A)", f"Current vs load at {fbin:g} Hz",
                os.path.join(out_dir, f"calibration_current_{tag}.png"), plt)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("folder", help="results folder with *_summary_*.csv and rpm_notes.csv")
    args = ap.parse_args()

    summaries = load_summaries(args.folder)
    if not summaries:
        sys.exit(f"no *_summary_*.csv files in {args.folder}")
    runs = build_runs(summaries, load_notes(args.folder))
    groups_rows = group(runs)
    out_dir = os.path.join(args.folder, "analysis")
    os.makedirs(out_dir, exist_ok=True)
    write_csv(os.path.join(out_dir, "runs.csv"), runs, fmt)
    write_csv(os.path.join(out_dir, "groups.csv"), groups_rows, fmt)

    print(f">> {len(runs)} runs with RPM, {len(groups_rows)} groups -> {out_dir}")
    print(f"{'Hz':>6} {'dir':>5} {'kg':>5} {'n':>3} {'slip':>14} {'slip RPM':>14} {'current A':>14}")
    for g in groups_rows:
        print(f"{g['freq_hz']:6.2f} {g['direction']:>5} {g['total_kg']:5g} {g['n']:3d} "
              f"{g['slip_mean']:7.4f}±{max(g['slip_std'], g['slip_reading_unc']):.4f} "
              f"{g['slip_rpm_mean']:8.2f}±{g['slip_rpm_std']:.2f}   "
              f"{g['current_mean']:6.3f}±{g['current_std']:.3f}")
    make_plots(groups_rows, out_dir)


if __name__ == "__main__":
    main()
