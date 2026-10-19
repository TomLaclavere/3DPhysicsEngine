# Performance Benchmark Analysis
#
# Data sources:
#   results_scalar/benchmark.csv, results/benchmark.csv - one row per (solver, dt, n_objects),
#   from scripts/benchmark.sh. Hardware counters and profiling reports are plotted separately
#   by profiling_analysis.py.
#
# Plots produced (each metric gets a vs-dt view & a vs-n_objects view):
#   wall_time, step_time, step_cv, speedup - × 2 views

from pathlib import Path

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.lines import Line2D

plt.rcParams.update({
    'figure.dpi': 130,
    'axes.spines.top': False,
    'axes.spines.right': False,
    'axes.grid': True,
    'grid.alpha': 0.25,
    'font.size': 11,
})

SOLVER_STYLE = {
    'Euler':  {'color': '#E24B4A', 'marker': 'o'},
    'Verlet': {'color': '#378ADD', 'marker': 's'},
    'RK4':    {'color': '#1D9E75', 'marker': '^'},
}
PROFILE_LS    = {'scalar': '--', 'optimised': '-'}
PROFILE_ALPHA = {'scalar': 0.6,  'optimised': 1.0}

REPO      = Path(__file__).resolve().parent
RESULTS   = REPO / 'results'
RESULTS_S = REPO / 'results_scalar'

# -----------------------------------------------------------------------------
# Data loading
# -----------------------------------------------------------------------------
def load_profile(directory: Path, label: str) -> pd.DataFrame | None:
    path = directory / 'benchmark.csv'
    if not path.exists():
        print(f'  [skip] {path}')
        return None
    d = pd.read_csv(path)
    d.insert(0, 'profile', label)
    d['wall_time_ms'] = d['wall_time_s'] * 1000
    d['cv_noisy']     = d['cv_percent'] > 10
    return d


frames = [f for f in [
    load_profile(RESULTS_S, 'scalar'),
    load_profile(RESULTS,   'optimised'),
] if f is not None]

if not frames:
    raise FileNotFoundError('No results found. Run ./scripts/benchmark.sh first.')

df = pd.concat(frames, ignore_index=True)

# Backward compatibility: old CSV files without n_objects column
if 'n_objects' not in df.columns:
    df['n_objects'] = 1

solvers       = sorted(df['solver'].unique(),   key=lambda s: ['Euler', 'Verlet', 'RK4'].index(s)   if s in ['Euler', 'Verlet', 'RK4']   else 99)
profiles      = sorted(df['profile'].unique(),  key=lambda p: ['scalar', 'optimised'].index(p)       if p in ['scalar', 'optimised']       else 99)
n_objs_list   = sorted(df['n_objects'].unique())
have_both     = {'scalar', 'optimised'}.issubset(set(profiles))
HAS_STEP      = 'step_us_mean' in df.columns

STEP_COL  = 'step_us_mean'    if HAS_STEP else 'time_per_step_us'
STEP_UNIT = 'µs  (profiling)' if HAS_STEP else 'µs  (derived)'

print(f'Loaded {len(df)} rows: {len(profiles)} profile(s) × {len(solvers)} solver(s) '
      f'× {df["dt"].nunique()} dt × {len(n_objs_list)} n_objects')
print(f'  n_objects: {n_objs_list}')
print(f'  step-level columns: {HAS_STEP}')


# -----------------------------------------------------------------------------
# Legend helpers
# -----------------------------------------------------------------------------
def solver_legend_handles(present=None):
    sols = present or list(SOLVER_STYLE)
    return [Line2D([0],[0], color=SOLVER_STYLE[s]['color'], marker=SOLVER_STYLE[s]['marker'],
                   ls='-', label=s, markersize=6) for s in sols if s in SOLVER_STYLE]

def profile_legend_handles():
    return [Line2D([0],[0], color='#555', ls=PROFILE_LS[p], alpha=PROFILE_ALPHA[p],
                   label=p, lw=1.8) for p in profiles]


# -----------------------------------------------------------------------------
# Generic plot primitives
# -----------------------------------------------------------------------------
def _lines_vs_dt(ax, col: str, sub: pd.DataFrame, log_y: bool = False) -> None:
    for profile in profiles:
        for solver in solvers:
            grp = sub[(sub['profile'] == profile) & (sub['solver'] == solver)].sort_values('dt')
            if grp.empty or col not in grp.columns:
                continue
            s      = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
            plot_f = ax.loglog if log_y else ax.semilogx
            plot_f(grp['dt'], grp[col],
                   color=s['color'], marker=s['marker'],
                   ls=PROFILE_LS.get(profile, '-'),
                   alpha=PROFILE_ALPHA.get(profile, 1.0),
                   markersize=4, lw=1.6)


def _plot_vs_dt(col: str, ylabel: str, title: str, out_name: str, log_y: bool = False,
                extra_handles: list | None = None) -> Path:
    """One subplot per n_objects, lines per (solver, profile), x-axis = dt."""
    n     = len(n_objs_list)
    fig, axes = plt.subplots(1, n, figsize=(max(8, 5 * n), 5), sharey=True)
    if n == 1:
        axes = [axes]
    for ax, nobj in zip(axes, n_objs_list):
        _lines_vs_dt(ax, col, df[df['n_objects'] == nobj], log_y=log_y)
        ax.set_xlabel('dt  (s)')
        ax.set_title(f'N = {nobj} objects', fontsize=10)
    axes[0].set_ylabel(ylabel)
    handles = solver_legend_handles(solvers) + profile_legend_handles() + (extra_handles or [])
    axes[-1].legend(handles=handles, fontsize=8, framealpha=0.5, ncol=2)
    fig.suptitle(title, y=1.02)
    plt.tight_layout()
    out = RESULTS / out_name
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


def _plot_vs_nobjs(col: str, ylabel: str, title: str, out_name: str,
                   extra_handles: list | None = None, thresholds: list | None = None) -> Path:
    """Lines per (solver, profile), x-axis = n_objects, fixed at finest dt."""
    rep_dt = float(df['dt'].min())
    sub    = df[np.isclose(df['dt'], rep_dt)]
    _, ax  = plt.subplots(figsize=(9, 6))
    for profile in profiles:
        for solver in solvers:
            grp = sub[(sub['profile'] == profile) & (sub['solver'] == solver)].sort_values('n_objects')
            if grp.empty or col not in grp.columns:
                continue
            s = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
            ax.plot(grp['n_objects'], grp[col],
                    color=s['color'], marker=s['marker'],
                    ls=PROFILE_LS.get(profile, '-'),
                    alpha=PROFILE_ALPHA.get(profile, 1.0),
                    markersize=5, lw=1.6)
    for val, color, lbl in (thresholds or []):
        ax.axhline(val, color=color, ls='--', lw=1.2, label=lbl)
    ax.set_xlabel('Number of objects')
    ax.set_ylabel(ylabel)
    ax.set_title(f'{title}\n(dt = {rep_dt:.2e} s)')
    handles = solver_legend_handles(solvers) + profile_legend_handles() + (extra_handles or [])
    if thresholds:
        handles += [Line2D([0],[0], color=c, ls='--', lw=1.2, label=lbl) for _, c, lbl in thresholds]
    ax.legend(handles=handles, fontsize=9, framealpha=0.5, ncol=2)
    plt.tight_layout()
    out = RESULTS / out_name
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# -----------------------------------------------------------------------------
# Plot 1 - Wall time
# -----------------------------------------------------------------------------
def plot_wall_time_vs_dt() -> Path:
    return _plot_vs_dt('wall_time_s', 'Wall time  (s)',
                       'Wall time vs dt  -  O(1/dt) scaling expected',
                       'plot_wall_time_vs_dt.png', log_y=True)

def plot_wall_time_vs_nobjs() -> Path:
    return _plot_vs_nobjs('wall_time_s', 'Wall time  (s)',
                          'Wall-time scaling with object count  -  O(N²) collision expected',
                          'plot_wall_time_vs_nobjs.png')


# -----------------------------------------------------------------------------
# Plot 2 - Step time
# -----------------------------------------------------------------------------
def plot_step_time_vs_dt() -> Path:
    return _plot_vs_dt(STEP_COL, f'Step time  ({STEP_UNIT})',
                       'Step time vs dt\n(plateau at fine dt = true per-step cost)',
                       'plot_step_time_vs_dt.png')

def plot_step_time_vs_nobjs() -> Path:
    return _plot_vs_nobjs(STEP_COL, f'Step time  ({STEP_UNIT})',
                          'Step-time scaling with object count',
                          'plot_step_time_vs_nobjs.png')


# -----------------------------------------------------------------------------
# Plot 3 - Step CV% (jitter)
# -----------------------------------------------------------------------------
def plot_step_cv_vs_dt() -> Path | None:
    if not HAS_STEP:
        return None
    thresholds = [(5, 'orange', '5%'), (10, 'red', '10%  (high jitter)')]
    extra = [Line2D([0],[0], color=c, ls='--', lw=1.2, label=lbl) for _, c, lbl in thresholds]
    return _plot_vs_dt('step_us_cv', 'Step CV%',
                       'Per-step jitter vs dt  (low CV → regular cost)',
                       'plot_step_cv_vs_dt.png', extra_handles=extra)

def plot_step_cv_vs_nobjs() -> Path | None:
    if not HAS_STEP:
        return None
    return _plot_vs_nobjs('step_us_cv', 'Step CV%',
                           'Per-step jitter vs object count',
                           'plot_step_cv_vs_nobjs.png',
                           thresholds=[(5, 'orange', '5%'), (10, 'red', '10%')])


# -----------------------------------------------------------------------------
# Plot 4 - Speedup
# -----------------------------------------------------------------------------
def _speedup_df() -> pd.DataFrame | None:
    if not have_both:
        return None
    scl = df[df['profile'] == 'scalar']
    opt = df[df['profile'] == 'optimised']
    m   = scl.merge(opt, on=['solver', 'dt', 'n_objects'], suffixes=('_scl', '_opt'))
    m['speedup_wall'] = m['wall_time_s_scl'] / m['wall_time_s_opt']
    if f'{STEP_COL}_opt' in m.columns:
        m['speedup_step'] = m[f'{STEP_COL}_scl'] / m[f'{STEP_COL}_opt'].replace(0, np.nan)
    return m


def plot_speedup_vs_dt() -> Path | None:
    spd = _speedup_df()
    if spd is None:
        print('[skip] Speedup vs dt requires both profiles.')
        return None
    n     = len(n_objs_list)
    fig, axes = plt.subplots(1, n, figsize=(max(8, 5 * n), 5), sharey=True)
    if n == 1:
        axes = [axes]
    for ax, nobj in zip(axes, n_objs_list):
        sub = spd[spd['n_objects'] == nobj]
        for solver in solvers:
            grp = sub[sub['solver'] == solver].sort_values('dt')
            if grp.empty:
                continue
            s = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
            ax.semilogx(grp['dt'], grp['speedup_wall'],
                        color=s['color'], marker=s['marker'], ls='-', markersize=4, lw=1.6)
        ax.axhline(1.0, color='#555', ls='--', lw=1.2)
        ax.set_xlabel('dt  (s)')
        ax.set_title(f'N = {nobj}', fontsize=10)
    axes[0].set_ylabel('Wall-time speedup  (scalar / optimised)')
    axes[-1].legend(handles=solver_legend_handles(solvers), fontsize=8, framealpha=0.5)
    fig.suptitle('Scalar → Optimised wall-time speedup vs dt  (> 1 = optimised wins)', y=1.02)
    plt.tight_layout()
    out = RESULTS / 'plot_speedup_vs_dt.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


def plot_speedup_vs_nobjs() -> Path | None:
    spd = _speedup_df()
    if spd is None:
        print('[skip] Speedup vs n_objects requires both profiles.')
        return None
    rep_dt = float(df['dt'].min())
    sub    = spd[np.isclose(spd['dt'], rep_dt)]
    _, ax  = plt.subplots(figsize=(9, 6))
    for solver in solvers:
        grp = sub[sub['solver'] == solver].sort_values('n_objects')
        if grp.empty:
            continue
        s = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
        ax.plot(grp['n_objects'], grp['speedup_wall'],
                color=s['color'], marker=s['marker'], ls='-', markersize=5, lw=1.6)
        if 'speedup_step' in grp.columns:
            ax.plot(grp['n_objects'], grp['speedup_step'].fillna(0),
                    color=s['color'], marker=s['marker'], ls='--', markersize=4, lw=1.2, alpha=0.7)
    ax.axhline(1.0, color='#555', ls='--', lw=1.2)
    ax.set_xlabel('Number of objects')
    ax.set_ylabel('Speedup  (scalar / optimised)')
    ax.set_title(f'Speedup vs object count  (dt = {rep_dt:.2e} s)\n'
                 'solid = wall-time  ·  dashed = step-time')
    ax.legend(handles=solver_legend_handles(solvers), fontsize=9, framealpha=0.5)
    plt.tight_layout()
    out = RESULTS / 'plot_speedup_vs_nobjs.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
def print_summary() -> None:
    finest = df.loc[df.groupby(['profile', 'solver', 'n_objects'])['dt'].idxmin()]
    rows = []
    for _, row in finest.sort_values(['profile', 'solver', 'n_objects']).iterrows():
        p, s, nobj = row['profile'], row['solver'], int(row['n_objects'])
        r = {
            'Profile':        p,
            'Solver':         s,
            'N objects':      nobj,
            'dt':             f"{row['dt']:.2e}",
            'Wall (ms)':      f"{row['wall_time_ms']:.2f}",
            STEP_COL:         f"{row[STEP_COL]:.4f}" if STEP_COL in row.index else 'n/a',
            'Steps/s':        f"{row['steps_per_second']:.0f}",
            'GFLOP/s':        f"{row['gflops_per_second']:.4f}",
            'Run CV%':        f"{row['cv_percent']:.1f}" + (' ⚠' if row['cv_noisy'] else ''),
        }
        if HAS_STEP:
            r['Step CV%'] = f"{row['step_us_cv']:.1f}" + (' ⚠' if row['step_us_cv'] > 10 else '')
        rows.append(r)

    print('\n── Summary (finest dt per solver × n_objects) ──────────────────────')
    print(pd.DataFrame(rows).set_index(['Profile', 'Solver', 'N objects']).to_string())

    if have_both:
        spd = _speedup_df()
        if spd is not None:
            finest_spd = spd.loc[spd.groupby(['solver', 'n_objects'])['dt'].idxmin()]
            print('\n── Speedup at finest dt per (solver, n_objects) ────────────────────')
            for _, row in finest_spd.sort_values(['solver', 'n_objects']).iterrows():
                wall = row['wall_time_s_scl'] / row['wall_time_s_opt']
                denom = row.get(f'{STEP_COL}_opt', 0)
                step  = f"{row[f'{STEP_COL}_scl'] / denom:.2f}×" if denom > 0 else 'n/a'
                print(f"  {row['solver']:<8} N={int(row['n_objects']):<3}  wall={wall:.2f}×  step={step}")

    if HAS_STEP:
        jittery = df[df['step_us_cv'] > 10]
        if not jittery.empty:
            print('\n── High step jitter (step CV > 10%) ───────────────────────────────')
            for _, row in jittery.iterrows():
                print(f"  {row['solver']:<8} {row['profile']:<12} N={int(row['n_objects']):<3} "
                      f"dt={row['dt']:.2e}  step_cv={row['step_us_cv']:.1f}%")


# -----------------------------------------------------------------------------
# PDF export
# -----------------------------------------------------------------------------
def export_pdf(plot_paths: list[Path | None]) -> Path:
    pdf_path = RESULTS / 'benchmark_report.pdf'
    with PdfPages(pdf_path) as pdf:
        for path in plot_paths:
            if not path or not path.exists():
                continue
            img = plt.imread(str(path))
            fig, ax = plt.subplots(figsize=(14, 8))  # fig passed to pdf.savefig
            ax.imshow(img)
            ax.axis('off')
            pdf.savefig(fig, bbox_inches='tight')
            plt.close(fig)
    return pdf_path


# -----------------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------------
RESULTS.mkdir(parents=True, exist_ok=True)

plot_paths: list[Path | None] = [
    # Wall time: vs dt (faceted by N) and vs N (at finest dt)
    plot_wall_time_vs_dt(),
    plot_wall_time_vs_nobjs(),
    # Step time
    plot_step_time_vs_dt(),
    plot_step_time_vs_nobjs(),
    # Step CV% (jitter)
    plot_step_cv_vs_dt(),
    plot_step_cv_vs_nobjs(),
    # Speedup
    plot_speedup_vs_dt(),
    plot_speedup_vs_nobjs(),
]

print_summary()

pdf = export_pdf(plot_paths)
print(f'\nPDF saved → {pdf}')
