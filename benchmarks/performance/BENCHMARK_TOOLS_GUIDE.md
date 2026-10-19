# Benchmarking & Profiling Guide

Everything needed to measure the engine's performance, understand where the time goes, and
check that an optimisation actually helped. Two commands do all the work; this guide explains
what they measure, how, and how to read the results.

- [1. Quick start](#1-quick-start)
- [2. Two questions, two tools](#2-two-questions-two-tools)
- [3. How measurements are kept reliable](#3-how-measurements-are-kept-reliable)
- [4. The benchmark sweep (`scripts/benchmark.sh`)](#4-the-benchmark-sweep-scriptsbenchmarksh)
- [5. The profiling pipeline (`scripts/profile.sh`)](#5-the-profiling-pipeline-scriptsprofilesh)
- [6. The profiling report, figure by figure](#6-the-profiling-report-figure-by-figure)
- [7. Current baseline](#7-current-baseline)
- [8. Workflow for an optimisation](#8-workflow-for-an-optimisation)
- [9. Troubleshooting](#9-troubleshooting)
- [10. File map](#10-file-map)

---

## 1. Quick start

```bash
# Optional, for the most stable timings (restore afterwards with: -g powersave)
sudo cpupower frequency-set -g performance

./scripts/benchmark.sh    # how fast, and how it scales                  (~40 min)
./scripts/profile.sh      # why: counters, vectorisation, allocations   (~3 min)
./scripts/full_report.sh  # both, one after the other
```

Then open:

| Report | Produced by |
|---|---|
| `benchmarks/performance/results/benchmark_report.pdf` | `benchmark.sh` |
| `benchmarks/performance/profiling_report/profiling_report.pdf` | `profile.sh` |

Both scripts rebuild what they need (incrementally), so they always measure the current code.
Requirements: the project's normal build dependencies, Python 3 with `numpy`, `pandas` and
`matplotlib`, and for profiling `perf`, MAQAO and MALT (install commands in
[`README.md`](README.md#installation-profiling-tools)). A missing profiler only skips its own
step.

---

## 2. Two questions, two tools

| | `scripts/benchmark.sh` | `scripts/profile.sh` |
|---|---|---|
| **Question** | How fast is the engine, and how does the cost scale with the solver, the number of objects N and the timestep dt? | *Why* does it take that long: is the CPU busy or stalled, where is the time spent, is the code vectorised, does it allocate? |
| **Runs** | A sweep: 3 solvers × 7 values of N × 5 values of dt, for each build profile | One fixed, representative workload, observed in depth |
| **Measures** | Wall time, time per step, run-to-run variance, estimated GFLOP/s, peak memory | Hardware counters, Top-Down bottleneck classification, hottest functions (perf); per-loop vectorisation and hotspots (MAQAO); every heap allocation (MALT) |
| **Output** | `results/benchmark_report.pdf` + PNGs + CSV | `profiling_report/profiling_report.pdf` + PNGs + raw tool reports |

Use the sweep to *see* a problem (or a gain), and profiling to *explain* it.

They are separate scripts on purpose. Profilers observe the program from the inside, and that
slows it down (MALT makes this workload ~7× slower), so timings must be taken with no profiler
attached. They also run at different rhythms: profiling (~3 min) after every change, the
sweep (~40 min) to confirm a gain holds at every size. `scripts/full_report.sh` runs both
sequentially, never in parallel, when both are wanted.

Both work on the same binary, `benchmarks/Performance` (source:
[`benchmark.cpp`](benchmark.cpp), CLI and CSV columns in [`README.md`](README.md)). It builds
a scene of N spheres falling on a plane, runs it for `dur / dt` steps, and reports timings.

---

## 3. How measurements are kept reliable

A benchmark is only useful if running it twice gives the same answer. Each point below removes
a specific source of error; all of them are applied automatically.

### 3.1 Two build profiles

Every measurement is made on two Release builds of the same code (defined in
[`profiles.sh`](profiles.sh)):

| Profile | Compiler flags | Role |
|---|---|---|
| `scalar` | `-O2 -fno-tree-vectorize -fno-inline` | Reference: no vectorisation, no inlining |
| `optimised` | `-O3 -march=native` | What the compiler achieves alone, using every instruction of this CPU |

The gap between them is what the compiler gives for free. Every manual optimisation is then
measured against `optimised`. The scalar profile also serves as a sanity check: MAQAO must
find **zero** vectorised loops in it.

Each profile has its own build directory (`build/benchmark_<profile>`). The build is
incremental, so it always reflects the current sources and flags. After building, the scripts
check in the CMake cache that the directory really is configured as Release with that
profile's flags, and stop otherwise (a silent CMake cache reset once made both profiles
identical, unoptimised builds).

Debug information (`-g`) and frame pointers are always on: they do not change the generated
code, and the profilers need them to name functions and source lines.

### 3.2 One core, always a P-core

This machine (i7-13850HX) has fast P-cores and slower E-cores. Measured on this engine, an
E-core runs the same code **~40% slower**, and an unpinned process can migrate between core
types in the middle of a run (observed: run-to-run variation jumping from <1% to 22%). Every
measured run is therefore pinned with `taskset` to one P-core (the second one, CPU 2, since
CPU 0 handles more interrupts). Override with `BENCH_CPU=<id>`.

### 3.3 CPU frequency

With the `powersave` governor the clock speed varies with load. The scripts print a warning
with the command to switch to `performance` (it needs `sudo`, so they never do it themselves).
Pinning already removes most of the noise; the governor removes the rest.

### 3.4 Warm-up, repetitions, variance

Each benchmark point runs 1 discarded warm-up run (caches and clock ramp up) then 5 timed runs,
and reports mean, min, max, standard deviation and the coefficient of variation
**CV = stddev / mean**. Rule of thumb: CV < 2% is clean; CV > 10% means the measurement is
noisy and should not be trusted (it is flagged with ⚠ in the summary).

### 3.5 Time per step

The total run time divided by the number of steps gives the average cost of a step
(`time_per_step_us`). A separate run measures the *distribution* of the step cost: steps are
timed in batches (~500 batches per run, so the clock overhead is negligible) in nanoseconds,
giving `step_us_mean/min/max/cv`. A high step CV means irregular steps, typically from
collision handling.

### 3.6 GFLOP/s is an estimate

`gflops_per_second` uses an analytical operation count (gravity O(N), ~45 FLOP per pair for
collision detection O(N²), integration O(N), ×4 force evaluations for RK4), not hardware
counters. It is good for comparing configurations, not as an absolute fraction of the CPU's
peak.

### 3.7 The representative profiling workload

Profiling uses one fixed configuration, shared by perf, MAQAO and MALT so their reports
describe the same run:

```text
--solver Verlet --dt 5e-5 --dur 30 --n_objects 10      (600 000 steps)
```

- **10 objects** exercise object-object collisions (45 pairs), which a single sphere never
  reaches.
- **600 000 steps** last long enough (~0.65 s per pass optimised, ~3 s scalar) for the
  sampling tools to collect over a thousand samples.

Change it with `--n-objects`, `--dt`, `--dur`, `--solver` on each `profile_*.sh` script (keep
the three in sync if you change the defaults).

---

## 4. The benchmark sweep (`scripts/benchmark.sh`)

### What it does

1. Checks the CPU setup (§3.2-3.3).
2. For each profile (scalar, then optimised): rebuilds, then runs every combination of
   solver ∈ {Euler, Verlet, RK4} × N ∈ {1, 2, 5, 10, 20, 50, 100} × 5 dt values log-spaced
   between 1e-5 and 1e-2 s, each simulating 2 s.
3. Runs [`benchmark_analysis.py`](benchmark_analysis.py), which plots both profiles and prints a
   summary table.

The simulated duration does not change the cost of a step, so 2 s is enough; the sweep takes
about 40 minutes (38 min measured), almost all of it in the scalar profile at N = 100 and
dt = 1e-5. `--dt-min 1e-4` drops the finest timestep and roughly divides that by 3.

```bash
./scripts/benchmark.sh --solvers "Verlet" --n-objects "1 10 100"   # narrower sweep
./scripts/benchmark.sh --skip-build --skip-analysis                 # reuse binaries, no plots
./scripts/benchmark.sh --help
```

### Outputs

```text
benchmarks/performance/
  results_scalar/benchmark.csv     one row per (solver, dt, N) - scalar profile
  results/benchmark.csv            same, optimised profile
  results/plot_*.png               figures
  results/benchmark_report.pdf     all figures in one file
```

### Reading the figures

| Figure | What it shows | What to look for |
|---|---|---|
| `plot_wall_time_vs_dt` | Total time per run, one panel per N | A straight line of slope −1 in log-log: time ∝ number of steps = dur/dt. Anything else means the per-step cost changes with dt. |
| `plot_wall_time_vs_nobjs` | Total time vs N (finest dt) | Growth close to N² once collisions dominate (every pair is tested). |
| `plot_step_time_vs_dt` / `_vs_nobjs` | Cost of one step | The plateau at fine dt is the true per-step cost; compare solvers (RK4 evaluates forces 4×). |
| `plot_step_cv_vs_dt` / `_vs_nobjs` | Regularity of the step cost | Below the 5% line = regular. Spikes = irregular steps (collisions) or noise. |
| `plot_speedup_vs_dt` / `_vs_nobjs` | scalar time ÷ optimised time | Above 1 = optimised wins. This is the compiler's gain; manual optimisations should push it further. |

The console summary repeats the key numbers at the finest dt and lists every noisy point
(step CV > 10%).

---

## 5. The profiling pipeline (`scripts/profile.sh`)

```bash
./scripts/profile.sh                      # perf + MAQAO + MALT + figures
./scripts/profile.sh --profile optimised  # only one profile for perf/MAQAO
./scripts/profile.sh --skip-maqao         # also --skip-perf, --skip-malt, --skip-plots
```

It runs the three tool scripts in order, then
[`profiling_analysis.py`](profiling_analysis.py). A failing or missing tool fails only its own
step; a summary lists what ran. Each tool script can also be run alone (`--help` for options).

### 5.1 perf - what the CPU is doing

Script: [`perf/profile_perf.sh`](perf/profile_perf.sh) · reference: [`perf/perf.md`](perf/perf.md)

`perf` reads the CPU's hardware counters and samples the program. For each profile it runs
three passes:

| Pass | Command | Gives |
|---|---|---|
| Counters | `perf stat -r 5` with grouped events | Cycles, instructions (→ IPC), branch misses, L1 and last-level cache misses. Mean ± variance over 5 runs. |
| Top-Down | `perf stat -M TopdownL1` | The share of pipeline slots spent on useful work vs each kind of bottleneck. |
| Sampling | `perf record -F 999 --call-graph dwarf` | Which functions the time is spent in (`perf_report_<profile>.txt`), and a flamegraph if the FlameGraph tools are installed. |

Details that keep the numbers exact:

- **Grouped events.** A CPU has only a handful of counter registers. Asking for more events
  than registers makes perf time-share them and *extrapolate* the counts. Events that form a
  ratio (cycles + instructions, loads + misses) are grouped `{...}` so they are always counted
  together and the ratio stays exact.
- **Top-Down in its own pass**, because it uses its own counter grouping.
- **Hybrid CPU.** Events are qualified as `cpu_core/<event>/` so only the P-core's counters are
  reported (the E-core lines would read `<not counted>`).
- **999 Hz** rather than 1000 Hz, so sampling never locks in phase with the kernel's 1 kHz timer.

### 5.2 MAQAO - is the code vectorised?

Script: [`maqao/profile_maqao.sh`](maqao/profile_maqao.sh) · reference: [`maqao/maqao.md`](maqao/maqao.md)

perf counts a scalar `add` and a 4-wide vector `add` the same way, so it cannot say whether
the code uses SIMD. MAQAO reads the generated assembly loop by loop.

| Module | How | Gives |
|---|---|---|
| **CQA** | Static: reads the binary, does not run it. Restricted to the engine's own functions. | Per loop: vectorised or not, share of the vector register width used, estimated speedup if fully vectorised, bottleneck hints. |
| **Lprof** | Runs and samples the program, aggregated per source loop. | Which loops actually take the time. |
| **ONE-View** | Runs CQA + Lprof together. | One HTML dashboard per profile (`oneview_<profile>/RESULTS/Performance_one_html/index.html`). |

Loop IDs are shared between Lprof and CQA, which is how the report joins "this loop takes X%
of the time" with "this loop could be Y× faster".

### 5.3 MALT - does the code allocate?

Script: [`malt/profile_malt.sh`](malt/profile_malt.sh) · reference: [`malt/malt.md`](malt/malt.md)

MALT intercepts every `malloc`/`new`/`free`/`delete` (it preloads its own allocator wrapper)
and records the size, the call stack and the time of each call. Counts are exact, not sampled.
It runs on the optimised profile only: compiler flags change how arithmetic is compiled, not
which allocations the source code makes.

Interactive exploration (call trees, source annotations):
`malt-webview benchmarks/performance/malt/results/malt_optimised/malt-Performance-*.json`.

### Outputs

```text
benchmarks/performance/
  perf/results/      perf_stat_*.txt, perf_topdown_*.txt, perf_report_*.txt, perf_*.data
  maqao/results/     cqa_*/cqa.txt, lprof_*/{functions,loops}.txt, oneview_*/
  malt/results/      malt_optimised/malt-Performance-<pid>.json
  */results/csv/     the benchmark binary's own CSV for these runs (kept apart from the sweep)
  profiling_report/  fig_*.png, profiling_report.pdf
```

---

## 6. The profiling report, figure by figure

`profiling_report.pdf` opens with a page of key findings, computed from the numbers (they
update with every run). Each following page shows one figure with a "How to read" and a "What
it shows" paragraph.

| Figure | Question | How to read it |
|---|---|---|
| `fig_perf_overview` | How much faster is optimised, and why? | Time per execution, instruction count, IPC (instructions per cycle). Fewer instructions = less work; higher IPC = the pipeline is better fed. IPC alone says nothing about vectorisation. |
| `fig_perf_topdown` | What limits the CPU? | 100% of pipeline slots split into **Retiring** (useful work), **Front-End Bound** (instruction fetch/decode can't keep up: many small calls, code size), **Back-End Bound** (waiting for data or execution units: memory, dependencies), **Bad Speculation** (mispredicted branches). The largest non-Retiring share is where to look. |
| `fig_perf_rates` | Do caches or branches cost anything? | Misses per 1000 instructions (MPKI). Preferred over miss *rates*, which can look huge on a handful of accesses. LLC MPKI > 1 (RAM traffic) or branch MPKI > 5 deserve attention. |
| `fig_perf_hotspots` | Which functions take the time? | Share of samples per function, self time only. Trivial getters near the top mean calls that are not inlined. |
| `fig_maqao_loops` | Which hot loops could be vectorised, and for how much? | Left: share of run time per loop (Lprof). Right: CQA's estimated speedup if that loop were fully vectorised. Hot *and* high potential = first target. |
| `fig_malt_sites` | Who allocates, how often, what size? | Allocations per call site and per size. Any allocation repeated every time step is a target: reuse the buffer. |
| `fig_malt_timeline` | Does memory grow? | Heap in use over the run. Flat = no growth; a rising curve = something accumulates. |

---

## 7. Current baseline

Measured on 2026-10-19 on an Intel Core i7-13850HX (pinned to a P-core, `powersave` governor),
representative workload (§3.7). These are the numbers optimisations will be compared against;
re-run `./scripts/profile.sh` to refresh them.

| | scalar | optimised |
|---|---:|---:|
| Time per execution (3 passes of 600 000 steps) | 9.69 s | 1.97 s (**4.9×**) |
| Instructions | 80.0 G | 26.3 G (3.0× fewer) |
| IPC | 2.19 | 3.56 |
| Top-Down: Retiring / Front-End / Back-End / Bad Spec. | 42 / 55 / 1 / 2 % | 62 / 20 / 11 / 7 % |
| Branch / L1 / LLC misses per 1000 instructions | 0.39 / 0.002 / 0.00004 | 0.56 / 0.002 / 0.00009 |
| Hot loops vectorised (MAQAO, top 6) | 0 | 0 |
| Loops vectorised among all analysed (MAQAO CQA) | 0 of 50 | 12 of 81 |

What this says:

1. **The compiler's 4.9× comes from doing less work, not from SIMD.** Inlining and `-O3`
   remove two thirds of the instructions; none of the hot loops is vectorised.
2. **The CPU is not waiting for memory.** Cache and branch misses are negligible: at N = 10
   all the data fits in L1. The remaining loss is mostly Front-End Bound: many small
   function calls.
3. **Small accessors are not inlined.** `Sphere::getRadius()` (12%), `Sphere::getCenter()`
   (9%), `Object::getPosition()` (5%) appear as separate hot functions in the optimised build:
   they are defined in `.cpp` files, so other files cannot inline them without link-time
   optimisation (`-flto`) or moving them to the headers.
4. **Two heap allocations every step.** `PhysicsWorld::integrateVerlet` creates two temporary
   `std::vector<Vector3D>` (`prevAcc`, `nextAcc`, `src/world/physicsWorld.cpp:283` and `:302`)
   at each step: 3.6 million allocations of 264 bytes over the run (MALT), and
   `new`/`malloc`/`free` visible in the perf hotspots. Reusing the buffers across steps
   removes them.
5. **Vectorisation potential is real but gated by layout.** CQA estimates up to 4× on the
   Verlet integration loops if fully vectorised. The current layout (one object = one
   `Vector3D` of 3 doubles, array of structures) is what prevents it; a structure-of-arrays
   layout is the Phase 6 data-layout item.

Benchmark sweep (same date and machine, `results*/benchmark.csv`), at the finest dt:

| N objects | 1 | 10 | 50 | 100 |
|---|---:|---:|---:|---:|
| Step time, Verlet scalar | 0.39 µs | 5.3 µs | 70 µs | 252 µs |
| Step time, Verlet optimised | 0.06 µs | 1.1 µs | 16 µs | 59 µs |
| Compiler speedup (Euler / Verlet / RK4) | 6.5 / 5.9 / 7.9× | 4.6 / 4.9 / 7.8× | 4.2 / 4.3 / 5.5× | 4.2 / 4.2 / 4.8× |

6. **The step cost grows as N².** From N = 50 to 100 the step time is multiplied by ~3.6
   (ideal N² scaling: 4): the pairwise collision tests dominate from a few tens of objects.
   This is the broad-phase item of the roadmap, and the largest lever for large N.
7. **The compiler's gain shrinks with N** (from 6-10× at small N to ~4.2× at N = 100): the
   collision loop that dominates at large N benefits less from `-O3` than the per-object code.

Run-to-run noise over the 210 points: median CV 0.7-0.9%, 4 points above 10% (very short
runs or one-off disturbances).

---

## 8. Workflow for an optimisation

1. **Baseline.** Commit or note the current numbers (`profile.sh`, and `benchmark.sh` if the
   change affects scaling).
2. **Pick the target from the report**: the largest Top-Down loss, the hottest function, the
   hottest loop with a high CQA speedup, or a per-step allocation.
3. **Change one thing.**
4. **Check physics is unchanged**: `./scripts/tests.sh`.
5. **Re-measure** with the same scripts, and compare: time and instructions
   (`fig_perf_overview`), the targeted metric, and CV (a gain smaller than the noise is not a
   gain).
6. **Scale it**: `benchmark.sh --n-objects "10 100 1000"` (or larger) to see whether the gain
   holds where it matters.

---

## 9. Troubleshooting

| Symptom | Cause and fix |
|---|---|
| `build_profile: ... is not configured as Release` | The build directory's CMake cache does not match the profile. Delete `build/benchmark_<profile>` and run again. |
| `[cpu] Warning: CPU governor is 'powersave'` | Informational. For the most stable numbers: `sudo cpupower frequency-set -g performance`, and `-g powersave` afterwards. |
| Large CV (> 10%) on some points | Background activity, thermal throttling, or very short runs (few steps at coarse dt). Re-run on a quiet machine; short configurations are naturally noisier. |
| `perf stat` permission error | `sudo sysctl kernel.perf_event_paranoid=1` (resets at reboot). |
| No flamegraph produced | Optional: install the FlameGraph tools (`stackcollapse-perf.pl`, `flamegraph.pl`). `perf_report_<profile>.txt` and the hotspot figure work without them. |
| A profiler step fails with "not found on PATH" | Install it ([`README.md`](README.md#installation-profiling-tools)); the other steps still run. |
| `malt-webview` fails with `libmalt-reader.so.1` | `sudo ldconfig` once after installing MALT. |
| A figure is missing from the report | Its input files are missing; the console says which script to run. |

---

## 10. File map

```text
scripts/
  benchmark.sh                     the sweep (§4)
  profile.sh                       the profiling pipeline (§5)
  full_report.sh                   benchmark.sh then profile.sh
benchmarks/performance/
  BENCHMARK_TOOLS_GUIDE.md         this guide
  README.md                        the benchmark binary: CLI, CSV columns, tool installation
  benchmark.cpp                    the benchmark binary (benchmarks/Performance)
  profiles.sh                      build profiles, CPU pinning, setup checks (shared)
  benchmark_analysis.py            figures + summary for the sweep
  profiling_analysis.py            figures + report for the profilers
  perf/   profile_perf.sh,  perf.md
  maqao/  profile_maqao.sh, maqao.md
  malt/   profile_malt.sh,  malt.md
docs/benchmark.md                  the Phase 6 optimisation plan
```
