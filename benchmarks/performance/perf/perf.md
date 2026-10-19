# perf - reference

Reference for [`profile_perf.sh`](profile_perf.sh). What perf measures and why the pipeline
uses it: [guide §5.1](../BENCHMARK_TOOLS_GUIDE.md#51-perf---what-the-cpu-is-doing).
Install: [`README.md`](../README.md#perf).

## Commands

| Goal | Command |
|------|---------|
| Both profiles, all passes | `./benchmarks/performance/perf/profile_perf.sh` |
| One profile | `./benchmarks/performance/perf/profile_perf.sh --profile optimised` |
| Counters only (no sampling) | `./benchmarks/performance/perf/profile_perf.sh --skip-record` |
| Another workload | `./benchmarks/performance/perf/profile_perf.sh --n-objects 100 --solver RK4` |
| Frame-pointer unwinding (lighter than DWARF) | `./benchmarks/performance/perf/profile_perf.sh --fp` |
| Browse the samples interactively | `perf report -i benchmarks/performance/perf/results/perf_optimised.data` |
| GUI viewer (optional `hotspot` package) | `hotspot benchmarks/performance/perf/results/perf_optimised.data` |

## What the script does

For each profile (`scalar`, `optimised`):

1. Builds the profile incrementally (`profiles.sh`) and checks its configuration.
2. **Counters**: `perf stat -r 5` with four exact groups: `{cycles,instructions}`,
   `{branches,branch-misses}`, `{L1-dcache-loads,L1-dcache-load-misses}`,
   `{LLC-loads,LLC-load-misses}` (plus front-end/back-end stall cycles on CPUs that have them).
3. **Top-Down**: `perf stat -M TopdownL1`, a separate pass.
4. **Sampling**: `perf record -F 999 --call-graph dwarf`, then `perf script` (text call stacks),
   `perf report` (top functions) and, if `stackcollapse-perf.pl`/`flamegraph.pl` are on
   `PATH`, a flamegraph.

Every pass is pinned to one P-core. On a hybrid CPU each event is written `cpu_core/<event>/`
so only the P-core counters are reported.

## Outputs (`perf/results/`)

| File | Content |
|---|---|
| `perf_stat_<profile>.txt` | Counters, mean ± variance over 5 runs |
| `perf_topdown_<profile>.txt` | Top-Down level-1 percentages |
| `perf_report_<profile>.txt` | Functions by share of samples (self time) |
| `perf_<profile>.data`, `.script` | Raw samples, resolved call stacks |
| `flame_<profile>.svg` | Flamegraph (optional tools) |
| `build_<profile>.log`, `csv/` | Build log; the benchmark binary's CSV for these runs |

`profiling_analysis.py` turns these into `fig_perf_*.png`.

## Reading the outputs

**`perf_stat_<profile>.txt`.** `( +- X% )` after a count is the variation across the 5 runs
(not multiplexing: grouped events are always counted together). Useful ratios:

| Ratio | Meaning |
|---|---|
| IPC = instructions / cycles | How busy the pipeline is. Up to ~4-5 on this CPU. Does not show vectorisation. |
| misses × 1000 / instructions (MPKI) | Cost of branch, L1 or LLC misses relative to the work. LLC MPKI > 1 = significant RAM traffic. |

**`perf_topdown_<profile>.txt`.** Four `tma_*` percentages summing to ~100% of pipeline slots:
`retiring` (useful), `frontend_bound`, `backend_bound`, `bad_speculation`. `nan` /
`<not counted>` lines for `cpu_atom` (E-cores) are expected and ignored.

**`perf_report_<profile>.txt` / flamegraph.** Share of samples per function. In a flamegraph,
width = share of samples (not time order), height = call depth; wide plateaus at the top are
the hotspots. In `perf report`, pressing `a` on a function shows annotated source and assembly.

## Current numbers

Default workload (`--solver Verlet --dt 5e-5 --dur 30 --n-objects 10`), 2026-10-19,
i7-13850HX:

| | scalar | optimised |
|---|---:|---:|
| Time per execution | 9.69 s | 1.97 s |
| Instructions | 80.0 G | 26.3 G |
| IPC | 2.19 | 3.56 |
| Top-Down retiring / front-end / back-end / bad spec. | 42 / 55 / 1 / 2 % | 62 / 20 / 11 / 7 % |
| Branch MPKI | 0.39 | 0.56 |
| Top function (self) | `Vector3D::operator[]` 9.6% | `BroadCollision::isColliding(Sphere, Sphere)` 15.6% |

Interpretation: [guide §7](../BENCHMARK_TOOLS_GUIDE.md#7-current-baseline).

## Pitfalls

- **Permission errors**: `sudo sysctl kernel.perf_event_paranoid=1` (resets at reboot). The
  script only warns, since counters work at the default value of 2 on this machine.
- **`stalled-cycles-frontend/backend` unsupported** on recent Intel CPUs: the script detects it
  and relies on Top-Down instead.
- **Few samples** (`# Samples:` in `perf_report_*.txt` below ~1 000): make the workload longer
  (`--dur`) or larger (`--n-objects`) before trusting the ranking.
