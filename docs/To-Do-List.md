# To-Do List for 3D Physics Engine project

While the roadmap outlines the major phases and tasks, this To-Do list captures smaller, actionable items that need to be addressed throughout the project. These side tasks are essential for maintaining code quality, ensuring smooth development, and enhancing the overall project.

## General Tasks

- [x] Benchmark performance : 
    - [x] adjust script to add flag to change solver, timestep or maxiteration + Nmb of run
    - [x] add perf in script
    - [x] add loop over timestep in script
    - [x] add MFLOP/s with perf

- [x] Benchmark :
    - [x] Correct benchmark process
        - [x] Lot of misses (high LLC miss *rate* on very few accesses; misses per 1000 instructions are negligible)
        - [x] Inconsistent results / bad stability (runs pinned to a P-core; CMake cache reset that silently unoptimised both profiles fixed; step timing no longer rounded to 0)
        - [x] Analyse results (baseline in `benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md` §7)
    - [x] Profiling
        - [x] Maqao
        - [x] Malt



- [x] Code Quality
    - [x] Unit Testing
        - [x] Add tests for new files
        - [x] Add basic physical examples as unit tests, to ensure correctness after each changes
    - [x] Documentation for new files

- [ ] Broad phase (branch `broad-phase`)
    - [x] Benchmark scene: random 3D cloud, mixed sizes, collisions; separate N and dt sweeps
    - [x] Fill the `docs/performance_logs.md` row once the sweep and `profile.sh` have run
    - [ ] Open the pull request: CI only runs on `main` and pull requests to it
    - [ ] Grid cells sized on 2 × the largest diameter (`getMaxValue() * 2` on a size that is already a diameter): try 1 diameter
    - [ ] Sweep & prune intervals use the bounding sphere of the size (`getNorm() * 0.5`), √3 too wide for spheres

- [x] Documentation and examples review
    - [x] READMEs, benchmark guide, scripts usage: checked against the code
    - [x] Free_Fall / Projectiles: contact heights (half-sizes), bullet position tolerance, `computation.ipynb` import
    - [x] CMake run targets (`make Free_Fall`...) run from the repository root
    - [x] CI: CMake option names (`3DPE_BUILD_TESTS`, `3DPE_ENABLE_COVERAGE`)
