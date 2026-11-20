"""Plotting helpers shared by the physics benchmark notebooks (benchmarks/physics/*/benchmark_analysis.ipynb).

Every value drawn comes from the CSV files or from theory: nothing is written from a past run.
"""

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_pdf import PdfPages

SOLVERS = ["Euler", "Verlet", "RK4"]
# RK4 is drawn last with hollow, larger markers so it stays visible where it overlaps Verlet
STYLE = {
    "Euler": dict(color="#E24B4A", marker="o", ls="-", markersize=4),
    "Verlet": dict(color="#378ADD", marker="s", ls="--", markersize=4),
    "RK4": dict(color="#1D9E75", marker="^", ls="-.", markersize=7, mfc="none"),
}
RESULTS = Path("results")
_saved = []

plt.rcParams.update(
    {
        "figure.dpi": 110,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.grid": True,
        "grid.alpha": 0.25,
        "font.size": 10,
    }
)


def plot_solvers(ax, df, x, y, logx=True, logy=True):
    """One line per solver: df[y] against df[x]. Non-positive values are skipped on a log axis."""
    for solver in SOLVERS:
        grp = df[df["solver"] == solver].sort_values(x)
        if logy:
            grp = grp[grp[y] > 0]
        ax.plot(grp[x], grp[y], label=solver, lw=1.4, **STYLE[solver])
    ax.set_xscale("log" if logx else "linear")
    ax.set_yscale("log" if logy else "linear")


def expected_orders(ax, df, x, y, orders):
    """Draw y ∝ x^p for each solver's expected order p (dotted, solver colour).

    The line passes through the median of the solver's own data (in log x and log y), so only its
    slope is a prediction: compare slopes, not levels.
    """
    for solver, p in orders.items():
        grp = df[(df["solver"] == solver) & (df[y] > 0)].sort_values(x)
        if len(grp) < 2:
            continue
        lx, ly = np.log(grp[x].to_numpy()), np.log(grp[y].to_numpy())
        lx0, ly0 = np.median(lx), np.median(ly)
        xs = np.array([grp[x].min(), grp[x].max()])
        ax.plot(xs, np.exp(ly0) * (xs / np.exp(lx0)) ** p, color=STYLE[solver]["color"], ls=":", lw=1.2,
                label=f"{solver}: slope {p}")


def shade_from(ax, x_start, label):
    """Grey out the x range from x_start to the right edge of the axis (results not trusted there)."""
    if x_start is None or not np.isfinite(x_start):
        return
    xlim = ax.get_xlim()
    ax.axvspan(x_start, xlim[1], color="grey", alpha=0.12, lw=0, label=label)
    ax.set_xlim(xlim)


def time_series(ede, value="energy_drift", ylabel="|E(t) - E₀| / E₀"):
    """One panel per recorded dt: value against time, all solvers, shared legend. Returns the figure."""
    dts = sorted(ede["dt"].unique())
    fig, axes = plt.subplots(len(dts), 1, figsize=(10, 2.3 * len(dts)), sharex=True, squeeze=False)
    for ax, dt in zip(axes[:, 0], dts):
        sub = ede[ede["dt"] == dt]
        for solver in SOLVERS:
            grp = sub[(sub["solver"] == solver) & (sub[value] > 0)].sort_values("time")
            ax.semilogy(grp["time"], grp[value], color=STYLE[solver]["color"], ls=STYLE[solver]["ls"],
                        lw=1.2, label=solver)
        ax.set_title(f"dt = {dt:.2e} s", fontsize=9, loc="left")
        ax.set_ylabel(ylabel, fontsize=8)
    axes[-1, 0].set_xlabel("Time (s)")
    axes[0, 0].legend(ncol=3, fontsize=8, loc="lower right")
    fig.tight_layout()
    return fig


def save(fig, name):
    """Save the figure as results/<name>.png and remember it for the PDF report."""
    path = RESULTS / f"{name}.png"
    fig.savefig(path, dpi=150, bbox_inches="tight")
    _saved.append(path)
    plt.show()


def export_pdf():
    """Write every figure saved so far to results/benchmark_report.pdf, one per page."""
    with PdfPages(RESULTS / "benchmark_report.pdf") as pdf:
        for path in _saved:
            fig, ax = plt.subplots(figsize=(11, 7))
            ax.imshow(plt.imread(path))
            ax.axis("off")
            pdf.savefig(fig, bbox_inches="tight")
            plt.close(fig)
    print(f"{len(_saved)} figures -> {RESULTS / 'benchmark_report.pdf'}")
