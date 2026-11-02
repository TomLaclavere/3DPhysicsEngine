# Performance logs

One line per change, measured with `./scripts/profile.sh` (time, instructions, IPC: optimised build,
representative workload) and `./scripts/benchmark.sh` (Verlet optimised step time, finest dt).
Machine: Intel Core i7-13850HX, pinned to a P-core, `powersave` governor.

| Date | Commit | Change | Time / exec | Instructions | IPC | Step N=10 | Step N=100 |
|---|---|---|---:|---:|---:|---:|---:|
| 2026-10-19 | `3492eb0` (tag `baseline`) | Reference, unoptimised engine | 1.97 s | 26.3 G | 3.56 | 1.1 µs | 59 µs |
