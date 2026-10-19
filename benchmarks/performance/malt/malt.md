# MALT - reference

Reference for [`profile_malt.sh`](profile_malt.sh) (verified with MALT 1.6.3). What MALT measures
and why the pipeline uses it:
[guide §5.3](../BENCHMARK_TOOLS_GUIDE.md#53-malt---does-the-code-allocate).
Install: [`README.md`](../README.md#malt).

## Commands

| Goal | Command |
|------|---------|
| Default run (optimised profile) | `./benchmarks/performance/malt/profile_malt.sh` |
| Another workload | `./benchmarks/performance/malt/profile_malt.sh --n-objects 100` |
| Interactive viewer | `malt-webview benchmarks/performance/malt/results/malt_optimised/malt-Performance-*.json` |

## What the script does

1. Builds the optimised profile incrementally and checks its configuration. One profile is
   enough: compiler flags do not change which allocations the source code makes
   (`--profile scalar` remains available).
2. Runs `malt <binary> <workload>`. The `malt` wrapper preloads MALT's library, which
   intercepts every `malloc`/`free`/`new`/`delete`, records size, call stack and time, then
   forwards the call to the real allocator.
3. Moves the report (`malt-Performance-<pid>.json`, written in the working directory) into
   `malt/results/malt_<profile>/`, replacing the previous one.

## Outputs (`malt/results/`)

| Path | Content |
|---|---|
| `malt_<profile>/malt-Performance-<pid>.json` | Full report |
| `build_<profile>.log`, `csv/` | Build log; the benchmark binary's CSV for this run |

`profiling_analysis.py` reads the JSON (`stacks.stats`, `memStats.sizeMap`,
`timeline.memoryTimeline`, `sites` for symbol names) into `fig_malt_sites.png` and
`fig_malt_timeline.png`.

## Reading the report

| Look at | Healthy | Suspicious |
|---|---|---|
| Allocations per step (total allocations ÷ integration steps) | ~0 in a steady-state time loop | Any steady per-step count: temporaries rebuilt every step |
| Size histogram | Spread, few counts | One size repeated millions of times: a buffer to reuse, or a pool |
| Heap over time | Flat | Rising: a container that keeps growing |
| Reallocations | Few | Many on one site: a `std::vector` growing without `reserve` |
| Blocks left at exit | A handful (static objects) | Growing with the run length: a leak |

Symbol names come from MALT's own resolution, which names a frame after the innermost
*inlined* function; an inlined `std::vector` allocation can therefore appear under the wrong
caller. Cross-check with Lprof's `functions.txt` (call chain of `operator new`) or the perf
hotspots, as the report does.

## Current numbers

Default workload (`--dt 5e-5 --dur 30 --n-objects 10`: 1.8 million integration steps over the
binary's 3 passes), 2026-10-19:

| Metric | Value |
|---|---|
| Heap allocations | 3 600 068, i.e. **2 per step** |
| Dominant size | 264 B (3 600 000 allocations) = 11 × `Vector3D` (24 B), one per object incl. the plane |
| Origin | `std::vector<Vector3D>` buffers `prevAcc` / `nextAcc` in `PhysicsWorld::integrateVerlet` (`src/world/physicsWorld.cpp:283`, `:302`) |
| Peak heap in use | 84 KiB, flat over the run |
| Blocks left at exit | 8 |

Interpretation: [guide §7](../BENCHMARK_TOOLS_GUIDE.md#7-current-baseline).

## Pitfalls

- MALT slows allocation-heavy programs down; keep the workload modest (counts scale linearly
  with the number of steps, the per-step pattern is what matters).
- `malt-webview` failing with `libmalt-reader.so.1`: run `sudo ldconfig` once after install.
