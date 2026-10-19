# MAQAO - reference

Reference for [`profile_maqao.sh`](profile_maqao.sh) (verified with MAQAO 2025.1.0). What MAQAO
measures and why the pipeline uses it:
[guide §5.2](../BENCHMARK_TOOLS_GUIDE.md#52-maqao---is-the-code-vectorised).
Install: [`README.md`](../README.md#maqao).

## Commands

| Goal | Command |
|------|---------|
| Both profiles, all modules | `./benchmarks/performance/maqao/profile_maqao.sh` |
| One profile | `./benchmarks/performance/maqao/profile_maqao.sh --profile optimised` |
| Static analysis only (fast, no execution) | `./benchmarks/performance/maqao/profile_maqao.sh --skip-lprof --skip-oneview` |
| Analyse every function, not only the engine's | `./benchmarks/performance/maqao/profile_maqao.sh --cqa-functions ".*"` |
| Another workload | `./benchmarks/performance/maqao/profile_maqao.sh --n-objects 100` |

## What the script does

For each profile (`scalar`, `optimised`):

1. Builds the profile incrementally and checks its configuration.
2. **CQA** (static): `maqao cqa <binary> fct-loops=<filter>`. The filter (MAQAO's own pattern
   syntax, comma = OR) keeps the engine's classes (`PhysicsWorld`, `NarrowCollision`,
   `BroadCollision`, `Sphere`, `Plane`, `Vector3D`, `simulation`) so libc/STL loops do not
   bury the report.
3. **Lprof** (dynamic): `maqao lprof` runs the workload pinned to a P-core, then writes the
   per-function and per-loop tables.
4. **ONE-View**: `maqao oneview -R1` runs CQA + Lprof together and builds an HTML report.

MAQAO has no separate roofline module in this version (`maqao --list-modules`: `analyze`,
`cqa`, `lprof`, `madras`, `oneview`); CQA prints "X% of peak computational performance (A out
of B FLOP per cycle)" for each loop instead.

## Outputs (`maqao/results/`)

| Path | Content |
|---|---|
| `cqa_<profile>/cqa.txt` | Per-loop static analysis |
| `lprof_<profile>/functions.txt`, `loops.txt` | Time share per function and per loop |
| `oneview_<profile>/RESULTS/Performance_one_html/index.html` | Combined HTML report |
| `build_<profile>.log`, `csv/` | Build log; the benchmark binary's CSV for these runs |

`profiling_analysis.py` joins `loops.txt` and `cqa.txt` (same loop IDs) into
`fig_maqao_loops.png`.

## Reading the outputs

**`cqa.txt`**, one section per binary loop (`Section x.y.z: Binary loop #ID`):

| Line | Meaning |
|---|---|
| `The loop is defined in:` | Source files and lines of the loop |
| `Your loop is not vectorized` / `probably not vectorized` / `vectorized` | Whether vector instructions process several values at once |
| `Only N% of vector register length is used` | Share of the 256-bit AVX2 registers actually used |
| `lower the cost of an iteration from A to B cycles (Sx speedup)` | Estimated gain if fully vectorised |
| `X% of peak computational performance is used` | Achieved vs peak FLOP per cycle |
| `Workaround(s)` | Generic suggestions (unit stride, AoS → SoA, FMA ordering...) |

A loop can have several "paths" (branches inside the loop); each gets its own block.

**`loops.txt` / `functions.txt`**: `Coverage (%)` is the share of samples in that loop or
function, `Time w.r.t Walltime (s)` the corresponding time. Indented lines give the call
chains.

## Current numbers

Default workload, 2026-10-19:

| | scalar | optimised |
|---|---:|---:|
| Loops analysed / vectorised (CQA) | 50 / 0 | 81 / 12 |
| Hottest loops (Lprof) | Verlet integration loops #161, #159 (not vectorised) | Verlet loops #285 (6.9%), #286 (5.6%), `solveCollisions` #284 (3.8%), all not vectorised |
| CQA speedup estimate on the Verlet loops | 4.0× | 4.0× |

The scalar build showing 0 vectorised loops confirms its flags are applied. Interpretation:
[guide §7](../BENCHMARK_TOOLS_GUIDE.md#7-current-baseline).

## Pitfalls

- Lprof and ONE-View execute the binary; CQA does not. Only the first two need a meaningful
  workload duration.
- Loop coverage is spread thin (no loop above 7%) because most of the time is in small
  function calls, not loops; the perf hotspots give the function-level view.
