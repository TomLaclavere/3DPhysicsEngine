# Performance Benchmark

## Overview

`benchmark.cpp` measures the raw simulation throughput of a single bouncing sphere against a fixed ground plane. The same scene is run repeatedly under controlled conditions to produce stable, comparable timing statistics across solvers and timesteps.

## Scene

| Parameter | Value |
|---|---|
| Dynamic objects | 1 sphere (z₀ = 20, radius = 2, mass = 1) |
| Static objects | 1 ground plane at z = 0 |
| Collision mode | Impulse (restitution = 1) |
| Gravity | Default engine gravity |

The scene is reconstructed from scratch before every run to avoid state leaking between repetitions.

## Run structure

```
[ warmup runs (discarded) ] [ timed runs (recorded) ]
```

- **Warmup runs** (`--n_warmup`, default 1): let the CPU reach steady-state frequency and warm instruction/data caches. Their timings are discarded.
- **Timed runs** (`--n_runs`, default 5): the wall-clock duration of each full simulation loop is recorded.

Each run executes exactly `maxiter = duration / dt` integration steps.

## Metrics collected

### Timing

| Field | Description |
|---|---|
| `cpu_us_mean` | Arithmetic mean of timed-run durations (µs) |
| `cpu_us_min` / `cpu_us_max` | Best and worst observed run (µs) |
| `cpu_us_stddev` | Bessel-corrected sample standard deviation (µs) |
| `cv_percent` | Coefficient of variation = stddev / mean × 100. CV > 10 % indicates thermal throttling or OS noise |
| `wall_time_s` | Mean run duration in seconds |
| `time_per_step_us` | `wall_time_s / maxiter` — per-step cost (µs) |
| `steps_per_second` | Throughput: `1 / time_per_step_us` scaled to Hz |

### FLOP/s

FLOP/s is estimated analytically from the algorithm structure — no hardware counters are used.

For this scene (1 dynamic object, impulse collision):

| Phase | Euler | Verlet | RK4 |
|---|---|---|---|
| Gravity (vec3 scale-add) | 6 | 6 | 6 × 4 |
| Sphere-plane contact test | ~10 | ~10 | ~10 × 4 |
| Impulse response | ~20 | ~20 | ~20 × 4 |
| Integration | ~12 | ~18 | ~12 × 4 |
| **Total / step** | **~48** | **~54** | **~192** |

RK4 evaluates forces at 4 stages per step, hence its 4× factor on force-related phases.

```
GFLOP/s = (flops_per_step × maxiter) / (wall_time_s × 1e9)
```

### Memory

`peak_rss_kb` is the peak resident set size of the process at benchmark end, read via `getrusage(RUSAGE_SELF)`. Useful for catching unexpected allocations when scaling object count or timestep.

## CLI arguments

| Flag | Default | Description |
|---|---|---|
| `--solver` | `Verlet` | Integration scheme: `Euler`, `Verlet`, or `RK4` |
| `--dt` | `1e-3` | Timestep (seconds) |
| `--dur` | `22.0` | Simulated duration (seconds) |
| `--n_warmup` | `1` | Number of discarded warmup runs |
| `--n_runs` | `5` | Number of timed runs |

## Output

Results are written to `benchmarks/performance/results/`.

**`benchmark.csv`** — one row per invocation:

```
solver, dt,
cpu_us_mean, cpu_us_min, cpu_us_max, cpu_us_stddev, cv_percent,
wall_time_s, time_per_step_us, steps_per_second,
flops_per_step, gflops_per_second, peak_rss_kb
```

**`energy_drift.csv`** — placeholder for per-step energy tracking (not yet populated).

## Interpreting results

- **Compare solvers at the same `dt`** to see the raw cost multiplier: RK4 ≈ 4× Euler in FLOPs, but actual wall time may differ due to memory access patterns.
- **CV > 10 %** means the machine is noisy — increase `--n_warmup` or pin the process to a core (`taskset -c 0`).
- **GFLOP/s** is scene-specific; it is meaningful for relative comparisons across solvers and `dt` values, not as an absolute hardware utilisation figure.
