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
| 2026-11-11 | `cf6f0a0` | Grid and SweepPrune broad phases, random 3D cloud scene | 0.58 s | 12.4 G | 4.55 | 0.26 µs (SweepPrune) | 11 µs (SweepPrune) | 0.46 ms (SweepPrune) | 7.8 ms (Grid) |

## Measurement checks

### 2026-11-18 - Google Benchmark vs in-house benchmark

Google Benchmark (`./scripts/benchmark.sh --gbench`, code `398972c` + `GBench`) timed the
n_objects sweep (Verlet, dt = 1e-3 s, 1000 steps, 5 repetitions). It calls the same timed
function as the in-house benchmark (`runSteps`, `benchmarks/performance/scenario.hpp`). The
in-house results it was compared with date from 2026-11-11 (code `cf6f0a0`): the two sides
differ by one engine commit (`398972c`, `computeAccelerations`), not only by the tool.

- **Run conditions:** `performance` governor, CPU time = 99.7% of wall time (no preemption),
  load average 2.1-2.6.
- **Grid and SweepPrune, N ≥ 20:** both tools agree within 4% (ratio 0.97-1.04): they measure
  the same thing.
- **Run-to-run variation, N ≥ 50:** median CV 1.1% (Google Benchmark) vs 1.0% (in-house).
- **Missing warm-up in Google Benchmark:** no measurable effect with the `performance`
  governor. The first repetition is 2% slower than the others on average (0.93-1.15).
- **Not explained by this run:**
  - Brut is 8-22% faster with Google Benchmark at N ≥ 50 (ratio 0.78-0.92).
  - Below ~0.2 µs per step (N ≤ 5), Google Benchmark is 1.2-1.9× slower.

  Both tools gave the same Brut time at N = 1000 when run together on the current code (2579 µs
  vs 2576 µs, `powersave`), so these gaps probably come from the code or machine state of
  2026-11-11, not from the tool. At N ≤ 5 a run lasts only 15-30 µs, and the in-house timer
  counts whole microseconds: 1000 steps are too few to measure there.

**Next:** run `./scripts/benchmark.sh` without `--gbench`, so both tools measure the same code in
the same session, before concluding on Brut and small N.

### 2026-11-18 - Same comparison, both tools in the same session

`./scripts/benchmark.sh` (without `--gbench`) ran both tools on the same code (`398972c`), point
after point, 16:36-16:45. Optimised profile, Verlet, dt = 1e-3 s, 1000 steps, 5 runs;
`performance` governor, load average 2.3-3.0.

- **N ≥ 20:** both tools agree within 5% (ratio of mean step times 0.91-1.04, median 1.00),
  for all three broad phases. The Brut gap of the previous check is gone: it came from the
  2026-11-11 data, not from the tool.
- **Two outliers, one per tool, both at N = 100:** one slow run raised the CV to 25% (in-house,
  Brut) and 32% (Google Benchmark, Grid). The fastest runs still agree within 7%.
- **N ≤ 10 (under 0.25 µs per step):** Google Benchmark's mean is 1.05-1.28× higher, but the
  fastest runs agree (ratio 0.93-1.02). The difference is the first repetition, 1.2-1.9× slower
  than the others: a 30-100 µs run starts on cold caches, and Google Benchmark has no warm-up run
  here, unlike the in-house benchmark. Above N = 20 the first repetition is within 10% of the
  others.

**Conclusion:** the two tools measure the same thing. For N ≤ 10, read Google Benchmark's median
or minimum, not its mean, or add a warm-up.
