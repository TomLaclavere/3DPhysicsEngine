# Performance Benchmark Analysis
#
# Data sources (produced by scripts/benchmark.sh):
#   results[_scalar]/benchmark.csv    — one row per (solver, dt) combination
#   results/perf_stat_<profile>_<solver>.txt  — perf stat per solver at finest dt
#
# Plots produced:
#   1. Wall time vs dt         — log-log, one line per (solver, profile)
#   2. Time/step vs dt         — same
#   3. Speedup vs dt           — scalar/optimised ratio per (solver, dt)
#   4. Hardware counters       — grouped bars per solver (scalar vs optimised)
#   5. IPC + stall breakdown
#   6. Cache & branch miss rates

import re
from pathlib import Path

import matplotlib
matplotlib.use('Agg')  # non-interactive backend — no display required
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
PROFILE_LS = {'scalar': '--', 'optimised': '-'}
PROFILE_ALPHA = {'scalar': 0.6, 'optimised': 1.0}

REPO      = Path('benchmarks/performance')
RESULTS   = REPO / 'results'
RESULTS_S = REPO / 'results_scalar'


# ─────────────────────────────────────────────────────────────────────────────
# Data loading
# ─────────────────────────────────────────────────────────────────────────────
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

solvers  = sorted(df['solver'].unique(),  key=lambda s: ['Euler', 'Verlet', 'RK4'].index(s) if s in ['Euler', 'Verlet', 'RK4'] else 99)
profiles = sorted(df['profile'].unique(), key=lambda p: ['scalar', 'optimised'].index(p) if p in ['scalar', 'optimised'] else 99)
have_both = {'scalar', 'optimised'}.issubset(set(profiles))

print(f'Loaded {len(df)} rows: {len(profiles)} profile(s) × {len(solvers)} solver(s) × {df["dt"].nunique()} dt value(s)')


# ─────────────────────────────────────────────────────────────────────────────
# perf stat loading — glob per (profile, solver)
# ─────────────────────────────────────────────────────────────────────────────
def parse_perf_stat(path: Path) -> dict[str, int]:
    """Parse perf stat output. Handles hybrid CPU (cpu_atom/event/u) and classic formats.
    Thousands separators stripped regardless of character (space, comma, U+202F, etc.)."""
    if not path.exists():
        return {}
    text   = path.read_text()
    counts: dict[str, int] = {}

    for line in text.splitlines():
        if 'cpu_atom/' not in line and 'cpu_core/' not in line:
            continue
        if '<not supported>' in line:
            continue
        m_event = re.search(r'cpu_(?:atom|core)/([\w-]+)/\w', line)
        if not m_event:
            continue
        m_num = re.search(r'\d[\d\s,  ]*\d|\d', line)
        if not m_num:
            continue
        val   = int(re.sub(r'[^\d]', '', m_num.group(0)))
        event = m_event.group(1)
        counts[event] = counts.get(event, 0) + val

    if not counts:  # classic format fallback
        skip = {'seconds', 'user', 'sys'}
        for m in re.finditer(r'^\s+(\d[\d,]*)\s+([\w-]+)', text, re.MULTILINE):
            event = m.group(2)
            if event not in skip:
                counts[event] = counts.get(event, 0) + int(m.group(1).replace(',', ''))

    return counts


def hw_metrics(counts: dict[str, int]) -> dict[str, float]:
    cycles  = counts.get('cycles', 0)
    instrs  = counts.get('instructions', 0)
    l1_ld   = counts.get('L1-dcache-loads', 0)
    l1_miss = counts.get('L1-dcache-load-misses', 0)
    llc_ld  = counts.get('LLC-loads', 0)
    llc_mis = counts.get('LLC-load-misses', 0)
    br      = counts.get('branches', 0)
    br_miss = counts.get('branch-misses', 0)
    fe_stl  = counts.get('stalled-cycles-frontend', 0)
    be_stl  = counts.get('stalled-cycles-backend', 0)
    return {
        'IPC':           instrs  / cycles  if cycles  else 0.0,
        'L1_miss_%':     100 * l1_miss / l1_ld   if l1_ld   else 0.0,
        'LLC_miss_%':    100 * llc_mis / llc_ld  if llc_ld  else 0.0,
        'branch_miss_%': 100 * br_miss / br      if br      else 0.0,
        'stall_fe_%':    100 * fe_stl  / cycles  if cycles  else 0.0,
        'stall_be_%':    100 * be_stl  / cycles  if cycles  else 0.0,
    }


# hw[(profile, solver)] = {metric: value}
hw: dict[tuple[str, str], dict[str, float]] = {}
for path in sorted(RESULTS.glob('perf_stat_*.txt')) + sorted(RESULTS_S.glob('perf_stat_*.txt')):
    parts = path.stem.split('_')   # perf_stat_<profile>_<solver>
    if len(parts) < 4:
        continue
    profile = parts[2]
    solver  = parts[3]
    hw[(profile, solver)] = hw_metrics(parse_perf_stat(path))

hw_keys    = [k for k in hw if hw[k]['IPC'] > 0]
HAVE_HW    = len(hw_keys) > 0
hw_solvers = sorted({s for _, s in hw_keys}, key=lambda s: ['Euler', 'Verlet', 'RK4'].index(s) if s in ['Euler', 'Verlet', 'RK4'] else 99)

if HAVE_HW:
    print(f'perf stat: {len(hw_keys)} file(s) — profiles: {sorted({p for p,_ in hw_keys})}, solvers: {hw_solvers}')
else:
    print('[skip] No perf stat data found.')


# ─────────────────────────────────────────────────────────────────────────────
# Shared legend helpers
# ─────────────────────────────────────────────────────────────────────────────
def solver_legend_handles(present_solvers=None):
    sols = present_solvers or list(SOLVER_STYLE)
    return [Line2D([0], [0], color=SOLVER_STYLE[s]['color'],
                   marker=SOLVER_STYLE[s]['marker'], ls='-', label=s, markersize=6)
            for s in sols if s in SOLVER_STYLE]


def profile_legend_handles():
    return [Line2D([0], [0], color='#555', ls=PROFILE_LS[p],
                   alpha=PROFILE_ALPHA[p], label=p, lw=1.8)
            for p in profiles]


# ─────────────────────────────────────────────────────────────────────────────
# Plot 1 — Wall time vs dt
# ─────────────────────────────────────────────────────────────────────────────
def plot_wall_time() -> Path:
    _, ax = plt.subplots(figsize=(11, 6))
    for profile in profiles:
        for solver in solvers:
            grp = df[(df['profile'] == profile) & (df['solver'] == solver)].sort_values('dt')
            if grp.empty:
                continue
            s = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
            ax.loglog(grp['dt'], grp['wall_time_s'],
                      color=s['color'], marker=s['marker'],
                      ls=PROFILE_LS.get(profile, '-'),
                      alpha=PROFILE_ALPHA.get(profile, 1.0),
                      markersize=4, lw=1.6)

    ax.set_xlabel('Time step  dt  (s)')
    ax.set_ylabel('Wall time  (s)')
    ax.set_title('Wall time vs dt  —  O(1/dt) scaling expected')
    handles = solver_legend_handles(solvers) + profile_legend_handles()
    ax.legend(handles=handles, fontsize=9, framealpha=0.5, ncol=2)

    plt.tight_layout()
    out = RESULTS / 'plot_wall_time.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# ─────────────────────────────────────────────────────────────────────────────
# Plot 2 — Time per step vs dt
# ─────────────────────────────────────────────────────────────────────────────
def plot_time_per_step() -> Path:
    _, ax = plt.subplots(figsize=(11, 6))
    for profile in profiles:
        for solver in solvers:
            grp = df[(df['profile'] == profile) & (df['solver'] == solver)].sort_values('dt')
            if grp.empty:
                continue
            s = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
            ax.semilogx(grp['dt'], grp['time_per_step_us'],
                        color=s['color'], marker=s['marker'],
                        ls=PROFILE_LS.get(profile, '-'),
                        alpha=PROFILE_ALPHA.get(profile, 1.0),
                        markersize=4, lw=1.6)

    ax.set_xlabel('Time step  dt  (s)')
    ax.set_ylabel('Time per step  (µs / step)')
    ax.set_title('Time per step vs dt\n(plateau at fine dt = true per-step cost; rising at coarse dt = startup overhead)')
    handles = solver_legend_handles(solvers) + profile_legend_handles()
    ax.legend(handles=handles, fontsize=9, framealpha=0.5, ncol=2)

    plt.tight_layout()
    out = RESULTS / 'plot_time_per_step.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# ─────────────────────────────────────────────────────────────────────────────
# Plot 3 — Scalar → Optimised speedup vs dt
# ─────────────────────────────────────────────────────────────────────────────
def plot_speedup() -> Path | None:
    if not have_both:
        print('[skip] Speedup plot requires both profiles.')
        return None

    _, ax = plt.subplots(figsize=(11, 6))
    scl_df = df[df['profile'] == 'scalar']
    opt_df = df[df['profile'] == 'optimised']
    merged = scl_df.merge(opt_df, on=['solver', 'dt'], suffixes=('_scl', '_opt'))

    for solver in solvers:
        grp = merged[merged['solver'] == solver].sort_values('dt')
        if grp.empty:
            continue
        speedup = grp['wall_time_s_scl'] / grp['wall_time_s_opt']
        s = SOLVER_STYLE.get(solver, {'color': '#888', 'marker': 'o'})
        ax.semilogx(grp['dt'], speedup,
                    color=s['color'], marker=s['marker'], ls='-',
                    markersize=4, lw=1.6, label=solver)

    ax.axhline(1.0, color='#555', ls='--', lw=1.2)
    ax.set_xlabel('Time step  dt  (s)')
    ax.set_ylabel('Speedup  (scalar wall time / optimised wall time)')
    ax.set_title('Scalar → Optimised speedup vs dt\n(> 1 means optimised wins)')
    ax.legend(handles=solver_legend_handles(solvers), fontsize=9, framealpha=0.5)

    plt.tight_layout()
    out = RESULTS / 'plot_speedup.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# ─────────────────────────────────────────────────────────────────────────────
# Plot 4 — Hardware counters (grouped bar: solver × profile)
# ─────────────────────────────────────────────────────────────────────────────
def _hw_grouped_bars(ax, metric: str, color_base: str, note: str) -> None:
    if not hw_solvers:
        return
    n_solvers  = len(hw_solvers)
    n_profiles = len(profiles)
    width      = 0.8 / n_profiles
    x          = np.arange(n_solvers)

    for pi, profile in enumerate(profiles):
        vals = [hw.get((profile, s), {}).get(metric, 0.0) for s in hw_solvers]
        # lighten colour for scalar
        alpha = PROFILE_ALPHA.get(profile, 1.0)
        bars = ax.bar(x + (pi - n_profiles / 2 + 0.5) * width, vals,
                      width * 0.9, label=profile,
                      color=color_base, alpha=alpha, zorder=3)
        for b in bars:
            h = b.get_height()
            if h > 0:
                ax.text(b.get_x() + b.get_width() / 2, h * 1.03,
                        f'{h:.2f}', ha='center', va='bottom', fontsize=7)

    ax.set_xticks(x)
    ax.set_xticklabels(hw_solvers)
    ax.set_title(f'{metric.replace("_", " ")}\n({note})', fontsize=9)


def plot_hw_overview() -> Path | None:
    if not HAVE_HW:
        return None

    hw_cfg = [
        ('IPC',           'higher is better', '#4C72B0'),
        ('L1_miss_%',     'lower is better',  '#DD8452'),
        ('LLC_miss_%',    'lower is better',  '#C44E52'),
        ('branch_miss_%', 'lower is better',  '#8172B2'),
        ('stall_fe_%',    'lower is better',  '#937860'),
        ('stall_be_%',    'lower is better',  '#64B5CD'),
    ]
    fig, axes = plt.subplots(2, 3, figsize=(16, 9))
    for ax, (metric, note, color) in zip(axes.flatten(), hw_cfg):
        _hw_grouped_bars(ax, metric, color, note)

    # Shared legend for profiles
    handles = profile_legend_handles()
    fig.legend(handles=handles, loc='lower center', ncol=len(profiles),
               fontsize=9, framealpha=0.5, bbox_to_anchor=(0.5, -0.02))
    fig.suptitle('Hardware Counters — perf stat  (grouped by solver, shading = profile)', fontsize=13, y=1.01)
    plt.tight_layout()
    out = RESULTS / 'plot_hw_overview.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# ─────────────────────────────────────────────────────────────────────────────
# Plot 5 — IPC + stall breakdown
# ─────────────────────────────────────────────────────────────────────────────
def plot_ipc_stalls() -> Path | None:
    if not HAVE_HW:
        return None

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    _hw_grouped_bars(ax1, 'IPC', '#4C72B0', 'higher = better')
    ax1.axhline(4.0, color='#555', ls=':', lw=1.2, alpha=0.7, label='OoO peak (~4)')
    ax1.set_ylabel('Instructions per cycle')
    ax1.set_title('IPC  (higher = better CPU utilisation)')
    ax1.legend(fontsize=9)

    n = len(hw_solvers)
    x = np.arange(n)
    w = 0.35
    for pi, profile in enumerate(profiles):
        fe = [hw.get((profile, s), {}).get('stall_fe_%', 0.0) for s in hw_solvers]
        be = [hw.get((profile, s), {}).get('stall_be_%', 0.0) for s in hw_solvers]
        offset = (pi - len(profiles) / 2 + 0.5) * w
        alpha  = PROFILE_ALPHA.get(profile, 1.0)
        ax2.bar(x + offset - w / 4, fe, w / 2, label=f'FE {profile}', color='#937860', alpha=alpha, zorder=3)
        ax2.bar(x + offset + w / 4, be, w / 2, label=f'BE {profile}', color='#64B5CD', alpha=alpha, zorder=3)

    ax2.set_xticks(x)
    ax2.set_xticklabels(hw_solvers)
    ax2.set_ylabel('% of cycles stalled')
    ax2.set_title('Stall breakdown  (lower = better)')
    ax2.legend(fontsize=8, ncol=2)

    plt.tight_layout()
    out = RESULTS / 'plot_ipc_stalls.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# ─────────────────────────────────────────────────────────────────────────────
# Plot 6 — Cache & branch miss rates
# ─────────────────────────────────────────────────────────────────────────────
def plot_cache_branch() -> Path | None:
    if not HAVE_HW:
        return None

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    for ax, (metric, title, color) in zip(axes, [
        ('L1_miss_%',     'L1 D-cache miss rate (%)', '#DD8452'),
        ('LLC_miss_%',    'LLC miss rate (%)',         '#C44E52'),
        ('branch_miss_%', 'Branch miss rate (%)',      '#8172B2'),
    ]):
        _hw_grouped_bars(ax, metric, color, 'lower is better')
        ax.set_ylabel('%')
        ax.set_title(f'{title}\n(lower is better)')

    handles = profile_legend_handles()
    fig.legend(handles=handles, loc='lower center', ncol=len(profiles),
               fontsize=9, framealpha=0.5, bbox_to_anchor=(0.5, -0.04))
    fig.suptitle('Cache & Branch Performance — perf stat', fontsize=13)
    plt.tight_layout()
    out = RESULTS / 'plot_cache_branch.png'
    plt.savefig(out, dpi=150, bbox_inches='tight')
    plt.close()
    return out


# ─────────────────────────────────────────────────────────────────────────────
# Summary table — pivot: (solver, profile) × metric
# ─────────────────────────────────────────────────────────────────────────────
def print_summary() -> None:
    finest = df.loc[df.groupby(['profile', 'solver'])['dt'].idxmin()]
    rows = []
    for _, row in finest.iterrows():
        p, s = row['profile'], row['solver']
        r = {
            'Profile': p, 'Solver': s,
            'dt':             f"{row['dt']:.2e}",
            'Wall time (ms)': f"{row['wall_time_ms']:.2f}",
            'µs/step':        f"{row['time_per_step_us']:.4f}",
            'Steps/s':        f"{row['steps_per_second']:.0f}",
            'GFLOP/s':        f"{row['gflops_per_second']:.4f}",
            'CV%':            f"{row['cv_percent']:.1f}" + (' ⚠' if row['cv_noisy'] else ''),
        }
        m = hw.get((p, s), {})
        if m.get('IPC', 0) > 0:
            r['IPC']     = f"{m['IPC']:.3f}"
            r['L1 miss'] = f"{m['L1_miss_%']:.2f}%"
            r['LLC miss']= f"{m['LLC_miss_%']:.2f}%"
            r['Br miss'] = f"{m['branch_miss_%']:.2f}%"
        rows.append(r)

    summary = pd.DataFrame(rows).set_index(['Profile', 'Solver'])
    print('\n── Summary (finest dt per solver) ─────────────────────────────────')
    print(summary.to_string())

    if have_both:
        merged = finest[finest['profile'] == 'scalar'].merge(
            finest[finest['profile'] == 'optimised'], on='solver', suffixes=('_scl', '_opt'))
        print('\n── Speedup at finest dt ────────────────────────────────────────────')
        for _, row in merged.iterrows():
            sp = row['wall_time_s_scl'] / row['wall_time_s_opt']
            print(f"  {row['solver']:<8}  {sp:.2f}×")


# ─────────────────────────────────────────────────────────────────────────────
# PDF export
# ─────────────────────────────────────────────────────────────────────────────
def export_pdf(plot_paths: list[Path | None]) -> Path:
    pdf_path = RESULTS / 'benchmark_report.pdf'
    with PdfPages(pdf_path) as pdf:
        for path in plot_paths:
            if not path or not path.exists():
                continue
            img = plt.imread(str(path))
            fig, ax = plt.subplots(figsize=(14, 8))  # fig kept: passed to pdf.savefig
            ax.imshow(img)
            ax.axis('off')
            pdf.savefig(fig, bbox_inches='tight')
            plt.close(fig)
    return pdf_path


# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────
RESULTS.mkdir(parents=True, exist_ok=True)

plot_paths: list[Path | None] = [
    plot_wall_time(),
    plot_time_per_step(),
    plot_speedup(),
    plot_hw_overview(),
    plot_ipc_stalls(),
    plot_cache_branch(),
]

print_summary()

pdf = export_pdf(plot_paths)
print(f'\nPDF saved → {pdf}')
