# HPC optimisation preparation plan

I will first implement a reproducible benchmarking pipeline to evaluate performance across different optimisation stages.

## Metric to collect

- Wall-clock simulation time
- FLOP/s
- Time per step
- Scaling behaviour (vs dt, vs N ?)
- CPU metrics :
  - Instructions per cycle (IPC)
  - Cache miss rates (L1, LLC)
- Memory :
  - Bandwidth usage
  - Allocation overhead
- Numerical
  - Energy drift
  - Stability vs dt

## Benchmarking methodology

- Fixed input configuration
- Multiple runs for statistical averaging
- Warm-up phase
- Consistent compiler flags : ```-O3 -march=native```
- Release build
- Store results in CSV

## Reference baseline

Before optimisation, I will establish two performance baselines: a scalar reference case and a compiler-optimised single-core case.

The first one will use low compiler optimisation with auto-vectorisation disabled and no multithreading:

```bash
-O2 -fno-tree-vectorize -fno-inline
```

The second one will use full compiler optimisations enabled with auto-vectorisation and single-thread execution:

```bash
-O3 -march=native
```

These baselines will serve as reference points for all future performance comparisons.

## Compiler analysis

I will analyse auto-vectorisation using:

- ```-fopt-info-vec```
- ```perf```
- profiling tools : Malt (memory allocation) & Maqao (Vectorisation)

The goal is to identify:

- Vectorised loops
- Non-vectorised bottlenecks
- Memory-bound

## Optimisation roadmap

0. Tools implementation
    - Reproducible benchmark simulation script : recompilation, fixed parameters, multiple realisations
    - Analysis tools wrappers
    - Plotting tools to save results

1. Data layout optimisation
    - Evaluate SoA vs AoS based on memory access patterns and SIMD efficiency

2. SIMD optimisation
    - Auto-vectorisation first
    - Intrinsics if necessary (Eve ?)

3. Multithreading
    - OpenMP / TBB
    - Evaluate scaling efficiency across cores

4. GPU
    - CUDA/SYCL (low-level)
    - Kokkos (abstraction)
    - nvc++ (compiler)

## External

I may use MKL as a performance reference for linear algebra operations, but not in the core simulation logic.
