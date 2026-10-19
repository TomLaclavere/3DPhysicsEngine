# Performance baseline

Reference results of the unoptimised engine, kept so that every later optimisation can be
compared against them. Do not overwrite: produce new results in the normal `results/`
directories and compare.

| | |
|---|---|
| Code | tag `perf-baseline` (commit `088e992`) |
| Date | 2026-10-19 |
| Machine | Intel Core i7-13850HX, runs pinned to P-core CPU 2, `powersave` governor |
| Sweep | `./scripts/benchmark.sh` defaults: Euler/Verlet/RK4 × N ∈ {1, 2, 5, 10, 20, 50, 100} × 5 dt in [1e-5, 1e-2], 2 s simulated, 1 warm-up + 5 timed runs |
| Profiling | `./scripts/profile.sh` defaults: Verlet, dt = 5e-5, 30 s simulated, N = 10 |

The layout mirrors `benchmarks/performance/`:

```text
results_scalar/benchmark.csv, results/benchmark.csv   benchmark sweep (scalar / optimised)
results/benchmark_report.pdf                          sweep figures
perf/results/                                         perf stat, Top-Down, perf report
maqao/results/                                        CQA and Lprof text reports
malt/results/malt_optimised/                          MALT report (JSON)
profiling_report/profiling_report.pdf                 profiling figures
```

Not kept (large, regenerable): `perf_*.data` / `.script` and the MAQAO ONE-View directories.
To regenerate everything, build the tagged code in a separate worktree and run the scripts
there:

```bash
git worktree add ../3DPE-baseline perf-baseline
cd ../3DPE-baseline && ./scripts/full_report.sh
```

The key numbers and their interpretation are in
[`BENCHMARK_TOOLS_GUIDE.md` §7](../BENCHMARK_TOOLS_GUIDE.md#7-current-baseline).
