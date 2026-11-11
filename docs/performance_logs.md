# Performance logs

One row per optimisation, to see what each change gained. All numbers come from the optimised
build (`-O3 -march=native`) on an Intel Core i7-13850HX, pinned to a P-core, with the
`performance` CPU governor, set by the scripts (the `baseline` row used `powersave`).

| Column | Source | Meaning |
|---|---|---|
| Time / exec | `./scripts/profile.sh` (`perf stat`) | Wall time of one execution of the profiling workload: Verlet, brute force, N = 10, dt = 5e-5 s, 30 s simulated, 3 passes of 600 000 steps (warm-up, timed run, step-timing run) |
| Instructions | same | Instructions executed by that execution: fewer = less work |
| IPC | same | Instructions per cycle: how busy the CPU pipeline is |
| Step N = … | `./scripts/benchmark.sh` (`step_us_mean` in `results/benchmark.csv`) | Cost of one simulation step at N objects, Verlet. The broad phase used is given in brackets |

Rows are only comparable when they use the same benchmark scene: the `baseline` row used
spheres on a 2D grid above a ground plane; later rows use the random 3D cloud of colliding
spheres (n_objects sweep, dt = 1e-3 s).

| Date | Commit | Change | Time / exec | Instructions | IPC | Step N = 10 | Step N = 100 | Step N = 1000 | Step N = 10000 |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 2026-10-19 | `3492eb0` (tag `baseline`) | Reference, unoptimised engine | 1.97 s | 26.3 G | 3.56 | 1.1 µs (Brut) | 59 µs (Brut) | - | - |
