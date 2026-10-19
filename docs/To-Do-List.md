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