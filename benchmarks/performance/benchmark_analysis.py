# Script converted from notebook
# %% [markdown]
# # Performance Benchmark Analysis — Euler / Verlet / RK4
# 
# Analyses two CSV files produced by the C++ benchmark:
# - **`benchmark.csv`** — per (solver, dt): physics quality metrics + timing statistics + estimated FLOP counts
# - **`energy_drift.csv`** — E(t) time series for 5 representative dt values
# 
# **Physics setup:** 1 sphere (r = 2 m, m = 1 kg) dropped from z₀ = 20 m, restitution e = 1.0 (elastic).  
# **Timing methodology:** 1 warm-up run + 5 timed runs per (solver, dt) combination.  
# CPU statistics (mean / min / max / σ) are computed across the 3 timed runs.
# 
# Plots produced:
# 1. Peak height error vs dt — convergence order
# 2. Flight energy drift vs dt — integrator quality in free flight
# 3. Max energy drift vs dt
# 4. **CPU time vs dt — with min/max timing band**
# 5. **Time per step vs dt**
# 6. **Estimated MFLOP/s vs dt**
# 7. Cost vs precision (FLOP budget vs height error)
# 8. Bounce count vs dt — solver stability
# 9. E(t) time series
# 10. Summary statistics table

# %%
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np
import pandas as pd
from matplotlib.lines import Line2D
from matplotlib.backends.backend_pdf import PdfPages
from scipy import stats

plt.rcParams.update({
    'figure.dpi': 130,
    'axes.spines.top': False,
    'axes.spines.right': False,
    'axes.grid': True,
    'grid.alpha': 0.25,
    'font.size': 11,
})

SOLVER_STYLE = {
    'Euler':  {'color': '#E24B4A', 'marker': 'o', 'ls': '-'},
    'Verlet': {'color': '#378ADD', 'marker': 's', 'ls': '--'},
    'RK4':    {'color': '#1D9E75', 'marker': '^', 'ls': '-.'},
}
ABOX = dict(boxstyle='round,pad=0.4', fc='white', ec='#cccccc', alpha=0.92, lw=0.8)
NBOX = dict(boxstyle='round,pad=0.6', fc='#f8f8f8', ec='#cccccc', lw=0.8)
SEP  = '─' * 29


def solver_legend(ax, loc='lower right'):
    handles = [
        Line2D([0], [0], color=s['color'], marker=s['marker'],
               linestyle=s['ls'], label=name, markersize=6)
        for name, s in SOLVER_STYLE.items()
    ]
    ax.legend(handles=handles, framealpha=0.5, loc=loc)


def note(ax, lines):
    """Display a monospace notes box on ax with axis('off')."""
    ax.axis('off')
    ax.text(0.04, 0.97, '\n'.join(lines), transform=ax.transAxes,
            fontsize=9, va='top', family='monospace', bbox=NBOX)


def log_slope(x, y, mask=None):
    """Return (slope, R²) from a log-log linear regression."""
    if mask is None:
        mask = (x > 0) & (y > 0)
    if mask.sum() < 4:
        return None, None
    slope, _, r, *_ = stats.linregress(np.log10(x[mask]), np.log10(y[mask]))
    return slope, r ** 2


# Analytical parameters — must match C++ benchmark
Z0          = 20.0
RADIUS      =  2.0
RESTITUTION =  1.0
G           =  9.81
TOTAL_TIME  = 22.0


def analytical_peak(n):
    return RADIUS + (Z0 - RADIUS) * RESTITUTION ** (2 * n)

# %%
BENCHMARK_CSV    = Path('results/benchmark.csv')
ENERGY_DRIFT_CSV = Path('results/energy_drift.csv')

df  = pd.read_csv(BENCHMARK_CSV)
ede = pd.read_csv(ENERGY_DRIFT_CSV)

df['cpu_ms_mean'] = df['cpu_us_mean'] / 1000.0
df['cpu_ms_min']  = df['cpu_us_min']  / 1000.0
df['cpu_ms_max']  = df['cpu_us_max']  / 1000.0

n_solvers = df['solver'].nunique()
n_dts     = df['dt'].nunique()
print(f'benchmark.csv    : {len(df)} rows  ({n_solvers} solvers, {n_dts} dt values)')
print(f'energy_drift.csv : {len(ede)} rows')
df.head()

# %% [markdown]
# ## Plot 1 — Peak height error vs dt (convergence order)

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

slopes = {}
for solver, grp in df.groupby('solver'):
    s    = SOLVER_STYLE[solver]
    grp  = grp.sort_values('dt')
    mask = grp['max_height_error'] > 1e-14
    ax.loglog(grp.loc[mask, 'dt'], grp.loc[mask, 'max_height_error'],
              color=s['color'], marker=s['marker'], ls=s['ls'], markersize=4, lw=1.6)
    sl, r2 = log_slope(grp['dt'], grp['max_height_error'], mask)
    slopes[solver] = (sl, r2)

dt_ref = np.array([1e-5, 5e-1])
ax.loglog(dt_ref, 6    * dt_ref ** 1, 'k:',  lw=1.2)
ax.loglog(dt_ref, 2e-1 * dt_ref ** 2, 'k--', lw=1.2)
ax.loglog(dt_ref, 2e-1 * dt_ref ** 4, 'k-.', lw=1.2)
ax.text(3e-1, 6 * 2.5,  'O(dt¹)', fontsize=8, color='#888', ha='center')
ax.text(2e-1, 1.5e-1,   'O(dt²)', fontsize=8, color='#888', ha='center')
ax.text(2e-1, 7e-4,     'O(dt⁴)', fontsize=8, color='#888', ha='center')

handles_s = [Line2D([0],[0], color=s['color'], marker=s['marker'],
                    ls=s['ls'], label=k, markersize=6) for k, s in SOLVER_STYLE.items()]
handles_r = [Line2D([0],[0], color='k', ls=':', label='O(dt¹) ref.'),
             Line2D([0],[0], color='k', ls='--', label='O(dt²) ref.'),
             Line2D([0],[0], color='k', ls='-.', label='O(dt⁴) ref.')]
ax.legend(handles=handles_s + handles_r, fontsize=9, framealpha=0.5, ncol=2, loc='upper left')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('|height_error|  (relative)')
ax.set_title('Peak height error vs dt — convergence order  (e = 1.0)')

e_sl,  e_r2  = slopes.get('Euler',  (None, None))
vl_sl, vl_r2 = slopes.get('Verlet', (None, None))
r4_sl, r4_r2 = slopes.get('RK4',    (None, None))

note(ax_n, [
    'CONVERGENCE ORDER  (e = 1.0)',
    SEP,
    '',
    'Measured log-log slopes:',
    f'  Euler  → O(dt^{e_sl:.2f})  R²={e_r2:.3f}' if e_sl else '  Euler  → n/a',
    f'  Verlet → O(dt^{vl_sl:.2f})  R²={vl_r2:.3f}' if vl_sl else '  Verlet → n/a',
    f'  RK4    → O(dt^{r4_sl:.2f})  R²={r4_r2:.3f}' if r4_sl else '  RK4    → n/a',
    '',
    'With e = 1.0, the O(dt) contact-',
    'timing error cancels for exact-',
    'position integrators (Verlet/RK4)',
    '→ residual is O(dt²).',
    '',
    'Euler: O(dt) position error',
    'breaks the cancellation',
    '→ O(dt¹) peak-height error.',
    '',
    'RK4 does NOT show O(dt⁴):',
    'both Verlet and RK4 are exact',
    'for quadratic trajectories;',
    'the O(dt²) residual comes from',
    'the contact geometry, not the',
    'ODE integrator order.',
])

plt.tight_layout()
plt.savefig('results/plot_convergence.png', dpi=150, bbox_inches='tight')
# plt.show()

print('{:<10} {:>8} {:>8}'.format('Solver', 'slope', 'R2'))
print('-' * 30)
for solver, (sl, r2) in slopes.items():
    if sl is not None:
        print(f'{solver:<10} {sl:>8.3f} {r2:>8.4f}')
    else:
        print('{:<10} {:>8}'.format(solver, 'n/a'))

# %% [markdown]
# ## Plot 2 — Flight energy drift vs dt (integrator quality in free flight)
# 
# Measures energy conservation error *between* bounces — during free-flight under constant gravity only.  
# Physical energy is conserved with e = 1.0, so any drift is purely numerical.  
# The reference energy is reset after each bounce, isolating integrator error from contact-phase effects.

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

flight_slopes = {}
for solver, grp in df.groupby('solver'):
    s    = SOLVER_STYLE[solver]
    grp  = grp.sort_values('dt')
    mask = (grp['max_flight_energy_drift'] > 1e-15) & (grp['bounce_count'] > 0)
    ax.loglog(grp.loc[mask, 'dt'], grp.loc[mask, 'max_flight_energy_drift'],
              color=s['color'], marker=s['marker'], ls=s['ls'], markersize=4, lw=1.6)
    sl, r2 = log_slope(grp.loc[mask, 'dt'], grp.loc[mask, 'max_flight_energy_drift'])
    flight_slopes[solver] = (sl, r2)

# Fit reference slope to Euler
e_grp = df[(df['solver'] == 'Euler')].sort_values('dt')
e_mask = (e_grp['max_flight_energy_drift'] > 1e-15) & (e_grp['bounce_count'] > 0)
if e_mask.sum() >= 4:
    sl_e, ic_e, *_ = stats.linregress(
        np.log10(e_grp.loc[e_mask, 'dt']),
        np.log10(e_grp.loc[e_mask, 'max_flight_energy_drift'])
    )
    dt_fit = np.array([e_grp.loc[e_mask, 'dt'].min(), e_grp.loc[e_mask, 'dt'].max()])
    ax.loglog(dt_fit, 10 ** ic_e * dt_fit ** sl_e, 'k:', lw=1.4,
              label=f'O(dt^{sl_e:.2f}) fit')

# Verlet/RK4 floor
vlt_floor = df[(df['solver'] == 'Verlet') & (df['bounce_count'] > 0)]['max_flight_energy_drift'].median()
ax.axhline(vlt_floor, color='#378ADD', ls=':', lw=1.2, alpha=0.6)
ax.text(1e-4, vlt_floor * 3,
        f'Verlet / RK4 floor ≈ {vlt_floor:.1e}  (machine precision)',
        color='#378ADD', fontsize=8, bbox=ABOX)

e_fs, e_fr2 = flight_slopes.get('Euler', (None, None))
note(ax_n, [
    'FLIGHT ENERGY DRIFT',
    SEP,
    '',
    'Integrator error in free flight.',
    'Physical bounce losses excluded',
    '(reference reset after each bounce).',
    '',
    'For constant-gravity free fall',
    '(quadratic trajectory):',
    '',
    '  Euler  → O(dt)  drift',
    f'    slope = {e_fs:.2f}  R² = {e_fr2:.3f}' if e_fs else '',
    '    −½g²dt² dissipation per step.',
    '',
    '  Verlet → exact  (machine ε)',
    '  RK4    → exact  (machine ε)',
    '',
    'Conclusion: Verlet is optimal —',
    'same flight accuracy as RK4',
    'at ¼ the per-step FLOP cost.',
])

solver_legend(ax, loc='upper left')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('Max |ΔE / E_ref|  between bounces')
ax.set_title('Flight energy drift vs dt — integrator quality in free flight')
plt.tight_layout()
plt.savefig('results/plot_flight_energy_drift.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 3 — Max energy drift vs dt

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

for solver, grp in df.groupby('solver'):
    s    = SOLVER_STYLE[solver]
    grp  = grp.sort_values('dt')
    mask = grp['max_energy_drift'] > 0
    ax.loglog(grp.loc[mask, 'dt'], grp.loc[mask, 'max_energy_drift'],
              color=s['color'], marker=s['marker'], ls=s['ls'], markersize=4, lw=1.6)

# Annotate Euler tunneling region
e_tun = df[(df['solver'] == 'Euler') & (df['bounce_count'] == 0)].sort_values('dt')
if len(e_tun):
    ann = e_tun.iloc[len(e_tun) // 2]
    ax.annotate('Euler (tunneling):\npure dissipation — up to 270%',
                xy=(ann['dt'], ann['max_energy_drift']),
                xytext=(3e-3, 1.5),
                arrowprops=dict(arrowstyle='->', color='#E24B4A', lw=1.2),
                color='#E24B4A', fontsize=9, bbox=ABOX)

# Verlet/RK4 machine precision region
vlt_fine = df[(df['solver'] == 'Verlet') & (df['bounce_count'] > 0) &
              (df['max_energy_drift'] > 0)].sort_values('dt')
if len(vlt_fine):
    ann_v = vlt_fine.iloc[0]
    ax.annotate('Verlet & RK4: machine precision\n(~10⁻¹²–10⁻¹¹ all dt)\ne=1 → no physical bounce losses',
                xy=(ann_v['dt'], ann_v['max_energy_drift']),
                xytext=(3e-3, 1e-9),
                arrowprops=dict(arrowstyle='->', color='#378ADD', lw=1.2),
                color='#555', fontsize=9, bbox=ABOX)

note(ax_n, [
    'ENERGY CONSERVATION  (e = 1.0)',
    SEP,
    '',
    'With e = 1, there is NO physical',
    'energy loss at bounces.',
    'All drift is NUMERICAL.',
    '',
    'Verlet & RK4 (bouncing regime):',
    '  Machine precision ~10⁻¹².',
    '  Exact free-flight + elastic',
    '  bounce → near-zero error.',
    '',
    'Euler (bouncing regime):',
    '  O(dt) drift from −½g²dt²/step.',
    '  No plateau: e=1 removes the',
    '  masking from physical loss.',
    '',
    'Large dt — tunneling:',
    '  Euler: drift up to 270%.',
    '  Verlet/RK4: near zero.',
    '',
    '→ Energy gap: ~10⁸× at fine dt.',
])

solver_legend(ax, loc='upper left')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('|E(t) − E₀| / E₀  (max over simulation)')
ax.set_title('Max energy drift vs dt  (e = 1.0)')
plt.tight_layout()
plt.savefig('results/plot_energy_drift_vs_dt.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 4 — CPU time vs dt with min/max timing band
# 
# The shaded band spans [min, max] across the 5 timed runs.  
# The solid line is the mean. Band width reflects OS scheduling noise and cache variability.

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

ratios = {}
mid_dt = df['dt'].median()

for solver, grp in df.groupby('solver'):
    s   = SOLVER_STYLE[solver]
    grp = grp.sort_values('dt')

    ax.fill_between(grp['dt'], grp['cpu_ms_min'], grp['cpu_ms_max'],
                    color=s['color'], alpha=0.15)
    ax.plot(grp['dt'], grp['cpu_ms_mean'],
            color=s['color'], marker=s['marker'], ls=s['ls'],
            markersize=4, lw=1.6, label=solver)

    row = grp.iloc[(grp['dt'] - mid_dt).abs().argsort().iloc[:1]]
    ratios[solver] = float(row['cpu_ms_mean'].iloc[0])

ax.set_xscale('log')
ax.set_yscale('log')

e_cpu  = ratios.get('Euler', 1.0)
vl_cpu = ratios.get('Verlet', 1.0)
r4_cpu = ratios.get('RK4', 1.0)
vl_ratio = vl_cpu / e_cpu if e_cpu > 0 else 0.0
r4_ratio = r4_cpu / e_cpu if e_cpu > 0 else 0.0

note(ax_n, [
    'CPU TIME  (mean ± min/max band)',
    SEP,
    '',
    'All solvers scale as O(1/dt):',
    'halving dt doubles CPU time.',
    '',
    'Cost ratios vs Euler at median dt:',
    f'  Verlet : ~{vl_ratio:.2f}×',
    f'  RK4    : ~{r4_ratio:.2f}×',
    '',
    'Band interpretation:',
    '  Narrow band (small dt) →',
    '    many iterations amortise',
    '    OS scheduling noise.',
    '  Wide band (large dt) →',
    '    few iterations → each run',
    '    more sensitive to jitter.',
    '',
    'For reliable timing, always',
    'use small dt (many iterations)',
    'or average more runs.',
])

ax.legend(framealpha=0.5, loc='upper right')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('CPU time  (ms, mean)')
ax.set_title('CPU time vs dt — with min/max timing band  (1 warm-up + 3 timed runs)')
plt.tight_layout()
plt.savefig('results/plot_cpu_time.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 5 — Time per step vs dt
# 
# `time_per_step_us = cpu_us_mean / num_steps`.  
# At fine dt (many iterations), startup overhead is negligible and the plateau reveals the true per-step integration cost.  
# At coarse dt (few iterations), apparent cost rises due to poor amortisation of fixed overhead.

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

per_step_plateau = {}
for solver, grp in df.groupby('solver'):
    s   = SOLVER_STYLE[solver]
    grp = grp.sort_values('dt')
    ax.plot(grp['dt'], grp['time_per_step_us'],
            color=s['color'], marker=s['marker'], ls=s['ls'],
            markersize=4, lw=1.6, label=solver)
    # plateau = median of the 10 finest-dt rows
    per_step_plateau[solver] = grp.nsmallest(10, 'dt')['time_per_step_us'].median()

ax.set_xscale('log')
ax.set_yscale('log')

e_ps  = per_step_plateau.get('Euler',  float('nan'))
vl_ps = per_step_plateau.get('Verlet', float('nan'))
r4_ps = per_step_plateau.get('RK4',   float('nan'))

note(ax_n, [
    'TIME PER STEP',
    SEP,
    '',
    'time_per_step = cpu_mean / N_steps',
    '',
    'Plateau at fine dt = pure',
    'integration cost (overhead',
    'fully amortised over many steps).',
    '',
    'Sustained per-step cost (median',
    'of 10 finest-dt runs):',
    f'  Euler  : {e_ps:.3f} µs/step',
    f'  Verlet : {vl_ps:.3f} µs/step',
    f'  RK4    : {r4_ps:.3f} µs/step',
    '',
    'Rising curve at coarse dt:',
    '  startup + teardown overhead',
    '  dominates when only O(10)',
    '  steps are executed.',
    '',
    'Use fine-dt values for real-time',
    'per-step budget planning.',
])

ax.legend(framealpha=0.5, loc='upper right')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('Time per step  (µs / step)')
ax.set_title('Time per step vs dt  (startup overhead visible at coarse dt)')
plt.tight_layout()
plt.savefig('results/plot_time_per_step.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 6 — Estimated MFLOP/s vs dt
# 
# `mflops_per_second = estimated_flops_total / cpu_us_mean` (computed in C++).  
# The plateau at fine dt is the **sustained estimated throughput** for each solver.  
# Use `perf` / `Maqao` for exact FLOP counts and hardware counter validation.

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

sustained = {}
for solver, grp in df.groupby('solver'):
    s   = SOLVER_STYLE[solver]
    grp = grp.sort_values('dt')
    ax.plot(grp['dt'], grp['mflops_per_second'],
            color=s['color'], marker=s['marker'], ls=s['ls'],
            markersize=4, lw=1.6, label=solver)
    sustained[solver] = grp.nsmallest(10, 'dt')['mflops_per_second'].median()

ax.set_xscale('log')

e_mf  = sustained.get('Euler',  float('nan'))
vl_mf = sustained.get('Verlet', float('nan'))
r4_mf = sustained.get('RK4',   float('nan'))

note(ax_n, [
    'ESTIMATED MFLOP/s',
    SEP,
    '',
    'Sustained MFLOP/s (median of',
    '10 finest-dt runs):',
    f'  Euler  : {e_mf:.1f} MFLOP/s',
    f'  Verlet : {vl_mf:.1f} MFLOP/s',
    f'  RK4    : {r4_mf:.1f} MFLOP/s',
    '',
    'Drop at coarse dt = startup',
    'overhead dominates (few iters).',
    '',
    'For exact FLOPs: perf / Maqao.',
    'For memory bandwidth: Malt.',
])

ax.legend(framealpha=0.5, loc='lower left')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('Estimated MFLOP/s')
ax.set_title('Estimated MFLOP/s vs dt  (approximate — see notes for caveats)')
plt.tight_layout()
plt.savefig('results/plot_mflops.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 7 — Cost vs precision
# 
# X-axis: total estimated FLOPs (proxy for computation budget, larger = more expensive).  
# Y-axis: peak height error (accuracy).  
# X-axis is inverted so left = costlier.  
# An ideal solver sits top-left (cheap) and bottom-left simultaneously → we want the lowest curve.

# %%
fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

for solver, grp in df.groupby('solver'):
    s    = SOLVER_STYLE[solver]
    grp  = grp.sort_values('estimated_flops_total')
    mask = grp['max_height_error'] > 0
    ax.loglog(grp.loc[mask, 'estimated_flops_total'],
              grp.loc[mask, 'max_height_error'],
              color=s['color'], marker=s['marker'], ls=s['ls'],
              markersize=4, lw=1.6, label=solver)

ax.invert_xaxis()

note(ax_n, [
    'COST vs PRECISION',
    SEP,
    '',
    'X = estimated FLOP budget.',
    'Left = more expensive.',
    '',
    'Euler:',
    '  O(dt¹) height error.',
    '',
    'Verlet:',
    '  O(dt²) height error.',
    '  Same FLOP budget as Euler',
    '  → Strictly dominates Euler.',
    '',
    'RK4:',
    '  O(dt²) height error.',
    '  Slightly lower prefactor',
    '  than Verlet at same budget.',
    '',
    'Verlet is uniquely optimal:',
    '  O(dt²) accuracy at O(dt¹)',
    '  cost — best achievable for',
    '  this contact model.',
])

solver_legend(ax, loc='upper right')
ax.set_xlabel('Estimated total FLOPs  (← more expensive)')
ax.set_ylabel('|height_error|  (relative)')
ax.set_title('Cost vs precision — iso-FLOP comparison')
plt.tight_layout()
plt.savefig('results/plot_cost_vs_precision.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 8 — Bounce count vs dt (solver stability)

# %%
def expected_bounces(total_time=TOTAL_TIME, z0=Z0, r=RADIUS, e=RESTITUTION, g=G):
    h0 = z0 - r
    t  = (2 * h0 / g) ** 0.5
    n  = 1
    while True:
        h_post = h0 * e ** (2 * n)
        if h_post < 1e-10:
            break
        flight = 2 * (2 * h_post / g) ** 0.5
        if t + flight > total_time:
            break
        t += flight
        n += 1
    return n

expected_n = expected_bounces()
print(f'Analytical expected bounces in {TOTAL_TIME:.0f} s (e={RESTITUTION}): {expected_n}')

fig, (ax, ax_n) = plt.subplots(1, 2, figsize=(15, 6), gridspec_kw={'width_ratios': [2, 1]})

for solver, grp in df.groupby('solver'):
    s   = SOLVER_STYLE[solver]
    grp = grp.sort_values('dt')
    ax.semilogx(grp['dt'], grp['bounce_count'],
                color=s['color'], marker=s['marker'], ls=s['ls'],
                markersize=4, lw=1.6)

ax.axhline(expected_n, color='black', ls=':', lw=1.5, alpha=0.7)
ax.text(6e-1, expected_n + 0.3,
        f'Analytical: {expected_n} bounces ({TOTAL_TIME:.0f} s)',
        fontsize=9, ha='right')

e_tun_dt  = df[(df['solver'] == 'Euler')  & (df['bounce_count'] == 0)]['dt'].min()
vl_tun_dt = df[(df['solver'] == 'Verlet') & (df['bounce_count'] == 0)]['dt'].min()

note(ax_n, [
    'BOUNCE COUNT STABILITY',
    SEP,
    '',
    f'Analytical: {expected_n} bounces in {TOTAL_TIME:.0f} s.',
    'With e=1, every bounce returns',
    'to z₀ → constant flight time.',
    '',
    'Tunneling thresholds:',
    f'  Euler      : dt > {e_tun_dt:.3f} s',
    f'  Verlet/RK4 : dt > {vl_tun_dt:.3f} s',
    '',
    'Below threshold: all solvers',
    f'converge to {expected_n} bounces.',
    '',
    'Euler transition zone (near',
    'tunneling threshold) is wider',
    'and less stable than Verlet/RK4.',
])

solver_legend(ax, loc='lower left')
ax.set_xlabel('Time step  dt  (s)')
ax.set_ylabel('Bounce count  (detected)')
ax.set_title(f'Bounce count vs dt — solver stability  ({TOTAL_TIME:.0f} s, e = {RESTITUTION})')
plt.tight_layout()
plt.savefig('results/plot_bounce_count.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Plot 9 — E(t) time series

# %%
selected_dts = sorted(ede['dt'].unique())
n_panels     = len(selected_dts)

fig = plt.figure(figsize=(15, 3.5 * n_panels))
gs  = gridspec.GridSpec(n_panels, 2, width_ratios=[2.5, 1], hspace=0.5)

for i, dt_val in enumerate(selected_dts):
    ax = fig.add_subplot(gs[i, 0])
    for solver in ['Euler', 'Verlet', 'RK4']:
        sub = ede[(ede['solver'] == solver) & np.isclose(ede['dt'], dt_val)].sort_values('time')
        s   = SOLVER_STYLE[solver]
        ax.semilogy(sub['time'], sub['energy_drift'].clip(lower=1e-17),
                    color=s['color'], ls=s['ls'], lw=1.4, label=solver, alpha=0.85)
    ax.set_title(f'E(t) — dt = {dt_val:.2e} s  (e = 1.0)', fontsize=10)
    ax.set_ylabel('|E(t) − E₀| / E₀', fontsize=9)
    ax.set_xlabel('Time (s)', fontsize=9)
    solver_legend(ax, loc='upper left')

ax_n = fig.add_subplot(gs[:, 1])
note(ax_n, [
    'ENERGY DRIFT  E(t)  (e = 1.0)',
    SEP,
    '',
    'Each panel: |E(t)−E₀|/E₀ over',
    'the full simulation.',
    '',
    'NO staircase! With e=1.0:',
    '  Perfectly elastic bounce →',
    '  no physical energy loss.',
    '  All drift is NUMERICAL.',
    '',
    'Verlet / RK4:',
    '  Flat at machine precision',
    '  (~10⁻¹²) for all dt.',
    '',
    'Euler:',
    '  Steady upward slope from',
    '  −½g²dt² dissipation/step.',
    '  Steeper slope at larger dt.',
    '',
    'Coarsest dt: ball tunnels.',
    '  Euler: linear drift.',
    '  Verlet/RK4: machine ε.',
])

fig.suptitle('E(t) time series — energy drift over simulation  (e = 1.0)', fontsize=12, y=1.005)
plt.savefig('results/plot_energy_time_series.png', dpi=150, bbox_inches='tight')
# plt.show()

# %% [markdown]
# ## Summary statistics table

# %%
expected_n = expected_bounces()
rows = []

for solver, grp in df.groupby('solver'):
    best = grp.loc[grp['dt'].idxmin()]

    stable_pct   = (abs(grp['bounce_count'] - expected_n) <= 1).mean() * 100
    sustained_us = grp.nsmallest(10, 'dt')['time_per_step_us'].median()
    sustained_mf = grp.nsmallest(10, 'dt')['mflops_per_second'].median()

    rows.append({
        'Solver':                      solver,
        'Best height error':           f"{best['max_height_error']:.2e}",
        'Flight drift (min dt)':       f"{best['max_flight_energy_drift']:.2e}",
        'Max energy drift':            f"{best['max_energy_drift']:.2e}",
        'Bounce count ±1 (%)':         f'{stable_pct:.0f}%',
        'CPU mean @ min dt (ms)':      f"{best['cpu_ms_mean']:.1f}",
        'Timing stddev @ min dt (ms)': f"{best['cpu_us_stddev'] / 1000:.2f}",
        'Sustained µs/step':           f'{sustained_us:.3f}',
        'Sustained MFLOP/s (est.)':    f'{sustained_mf:.1f}',
    })

summary = pd.DataFrame(rows).set_index('Solver')
print('Notes:')
print('  • Flight drift = integrator error only (between bounces).')
print('  • Verlet / RK4 are at machine ε for flight drift.')
print(f'  • Bounce count ±1 uses |count − {expected_n}| ≤ 1.')
print('  • MFLOP/s is estimated from static FLOP counts per step.')
print()
summary

# %% [markdown]
# ## Export all plots to PDF

# %%
plot_files = [
    'results/plot_convergence.png',
    'results/plot_flight_energy_drift.png',
    'results/plot_energy_drift_vs_dt.png',
    'results/plot_cpu_time.png',
    'results/plot_time_per_step.png',
    'results/plot_mflops.png',
    'results/plot_cost_vs_precision.png',
    'results/plot_bounce_count.png',
    'results/plot_energy_time_series.png',
]

with PdfPages('results/benchmark_report.pdf') as pdf:
    for path in plot_files:
        img = plt.imread(path)
        fig, ax = plt.subplots(figsize=(14, 7))
        ax.imshow(img)
        ax.axis('off')
        pdf.savefig(fig, bbox_inches='tight')
        plt.close(fig)

print('PDF saved to results/benchmark_report.pdf')


