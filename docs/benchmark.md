# HPC optimisation plan

Strategy behind Phase 6 of the [roadmap](ROADMAP.md) (which holds the detailed checklist).
How to run and read the measurements: [`BENCHMARK_TOOLS_GUIDE.md`](../benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md).

## Measurement

Every optimisation is measured with the same reproducible pipeline: fixed scene and seed,
Release builds, runs pinned to one P-core, warm-up then repeated runs, results stored as CSV.

| Metric | Status |
|---|---|
| Wall time, time per step, scaling vs N and vs dt | `scripts/benchmark.sh` |
| Estimated FLOP/s | `scripts/benchmark.sh` (brute force only) |
| IPC, cache and branch misses, Top-Down | `perf` |
| Vectorisation per loop | MAQAO, `-fopt-info-vec` |
| Allocations | MALT |
| Memory bandwidth | Not measured yet (roofline, Phase 6 documentation) |
| Energy drift, stability vs dt | Physics benchmarks ([`benchmarks/physics/`](../benchmarks/physics/)) |

Two reference builds bound every result: `scalar` (`-O2 -fno-tree-vectorize -fno-inline`) and
`optimised` (`-O3 -march=native`). The unoptimised engine is kept as tag `baseline`.

## Optimisation order

| Step | Status |
|---|---|
| Benchmark and profiling tools, reference builds | Done |
| Quick wins from the baseline profile (allocations, inlining) | Done |
| Broad phase (grid, sweep & prune) | Done |
| Data layout: AoS vs SoA | Next |
| SIMD: auto-vectorisation, then intrinsics if needed | Planned |
| Multithreading: OpenMP / TBB, scaling across cores | Planned |
| GPU: CUDA/SYCL, Kokkos, nvc++ (Phase 7) | Planned |

MKL may serve as a performance reference for linear algebra, not in the simulation core.
