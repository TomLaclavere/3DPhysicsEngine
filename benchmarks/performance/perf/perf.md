# perf - tool-specific reference

Quick reference for `benchmarks/performance/perf/profile_perf.sh`. Read
[`BENCHMARK_TOOLS_GUIDE.md`](../BENCHMARK_TOOLS_GUIDE.md) Part 1 first if you haven't - it
explains *why* each of the pieces below exists; this page is just the command/output
reference once you already know that.

## Install

```bash
# Arch (this machine, already installed):
sudo pacman -S perf

# Ubuntu/Debian:
sudo apt install linux-tools-common linux-tools-generic
```

**macOS: not supported, no equivalent.** `perf` wraps the Linux kernel's `perf_events`
subsystem directly — there's nothing to install on macOS because the subsystem itself doesn't
exist there. The closest built-in tools are Instruments (GUI, ships with Xcode) or `dtrace`
(command-line, but macOS's System Integrity Protection blocks much of what it can collect);
neither is a drop-in for this pipeline's scripts.

`perf --version` to confirm. Optional but referenced below: `flamegraph` (provides
`stackcollapse-perf.pl`/`flamegraph.pl`, on Arch it's an AUR/chaotic-aur package, on
Ubuntu/Debian it's cloned from Brendan Gregg's [FlameGraph repo](https://github.com/brendangregg/FlameGraph)
rather than packaged) and `hotspot` (a GUI viewer — `chaotic-aur/hotspot` on Arch, `hotspot`
via apt on recent Ubuntu). Neither is required — `profile_perf.sh` skips the flamegraph step
gracefully if the tools aren't on `PATH`, and `hotspot` is only ever suggested as a manual
follow-up, never invoked by the script.

## Quick reference

| Goal | Command |
|------|---------|
| Full pipeline, both profiles | `./benchmarks/performance/perf/profile_perf.sh` |
| One profile only | `./benchmarks/performance/perf/profile_perf.sh --profile optimised` |
| Stat only, skip record/flamegraph | `./benchmarks/performance/perf/profile_perf.sh --skip-record` |
| Reuse existing binaries | `./benchmarks/performance/perf/profile_perf.sh --skip-build` |
| Different representative workload | `./benchmarks/performance/perf/profile_perf.sh --n-objects 100` |
| Interactive TUI on the recorded data | `perf report -i benchmarks/performance/perf/results/perf_optimised.data` |
| GUI viewer | `hotspot benchmarks/performance/perf/results/perf_optimised.data` |

## What the script produces

```text
benchmarks/performance/perf/results/
  perf_stat_<profile>.txt       grouped hardware counters, repeated 5x (mean +- stddev)
  perf_topdown_<profile>.txt    Top-Down microarchitecture breakdown
  perf_<profile>.data           raw perf record data
  perf_<profile>.script         perf script -i ... output (resolved call stacks)
  flame_<profile>.svg           flamegraph (needs stackcollapse-perf.pl + flamegraph.pl on PATH)
```

`<profile>` is `scalar` or `optimised` - see `benchmarks/performance/profiles.sh` for what
those compiler flags are.

## This machine's specifics (check yours before trusting numbers blindly)

- **Hybrid P-core/E-core CPU**: this machine exposes two PMUs, `cpu_core` (P-cores) and
  `cpu_atom` (E-cores). A bare event name like `cycles` silently expands to both; whichever
  PMU the process didn't run on reports `<not counted>` for every line - not a bug, just
  noise. `profile_perf.sh` detects `/sys/devices/cpu_core` and, if present, pins the run to a
  P-core with `taskset` and qualifies every event as `cpu_core/.../` so the report only
  contains real numbers. If your machine isn't hybrid, the script skips this and uses plain
  event names.
- **`stalled-cycles-frontend`/`stalled-cycles-backend` may not exist on your CPU.** On this
  machine they're unsupported outright (superseded by Top-Down on recent Intel parts) - the
  script probes for them once at startup and drops them from the event groups if absent,
  falling back entirely on the Top-Down pass for that question.
- **`kernel.perf_event_paranoid`**: if `perf stat` fails with a permission error, run
  `sudo sysctl kernel.perf_event_paranoid=1`. On this machine it's `2` by default but hardware
  counters still work for a normal user - the script only warns, doesn't block.

## Reading `perf_stat_<profile>.txt`

Events are grouped with `{...}` so each group is counted simultaneously rather than
multiplexed (see the guide §1.4) - the `( +- X% )` after each value is run-to-run variance
from `-r 5`, not multiplexing.

| Metric | How to read it |
|---|---|
| `cycles`, `instructions` | IPC = instructions ÷ cycles. Compare scalar vs optimised - see the worked example below, the direction can surprise you. |
| `branches`, `branch-misses` | miss rate = branch-misses ÷ branches × 100. > 1-3% on a numeric loop is worth investigating. |
| `L1-dcache-loads`, `L1-dcache-load-misses` | L1 miss rate. Needs a workload that actually touches enough memory to be interesting - see the caveat below. |
| `LLC-loads`, `LLC-load-misses` | Same idea, one cache level further out - a miss here goes all the way to RAM. |

## Reading `perf_topdown_<profile>.txt`

Four percentages (`tma_retiring`, `tma_frontend_bound`, `tma_backend_bound`,
`tma_bad_speculation`) summing to ~100% of pipeline slots. `<not counted>`/`nan` lines for
`cpu_atom/...` are expected on a hybrid machine pinned to a P-core (see above) - ignore them,
the `cpu_core/...` lines are the real numbers.

## Reading the flamegraph / `perf report`

Covered in depth in the guide (§1.6-1.7). Short version: X-axis = fraction of samples (not
time), Y-axis = call stack depth, wide flat plateaus at the top are your hotspots, click any
frame to zoom. `perf report -i <data>` then `a` on a selected function shows annotated
source+assembly with per-line hit counts.

## Worked example (this machine, `--solver Verlet --dt 5e-5 --dur 30 --n-objects 1`)

Real output from running `profile_perf.sh` on both profiles:

| Metric | scalar | optimised |
|---|---:|---:|
| cycles | 2 611 357 084 | 445 340 440 |
| instructions | 5 849 260 664 | 1 858 501 257 |
| **IPC** | **2.24** | **4.17** |
| Top-Down: Frontend-Bound | **57.2%** | 27.4% |
| Top-Down: Retiring | 42.0% | **69.0%** |
| Top-Down: Bad Speculation | 0.4% | 1.2% |
| Top-Down: Backend-Bound | 0.4% | 2.4% |
| wall time (5 timed runs, warmup discarded) | 180.6 ms | 30.4 ms |

Two things worth noticing:

1. **IPC went *up* on the optimised build, not down.** The guide's Part 1.3 checkpoint
   flagged this as a real possibility, not a certainty - here it happened: fewer, wider
   (SIMD) instructions retiring more efficiently per cycle, on top of just needing 3.15x
   fewer instructions in the first place. IPC alone would have told a correct but incomplete
   story; MAQAO's vectorisation ratio (once installed) is what actually confirms *why* the
   instruction count dropped, not just that IPC rose.
2. **Scalar is Frontend-Bound at 57%** - plausibly `-fno-inline` doing exactly what it says:
   many more, smaller function calls instead of inlined code, meaning far more instruction
   fetch/decode work relative to actual arithmetic. This is a concrete, checkable hypothesis
   the Top-Down number gives you for free, not a guess.

**Caveat**: cache-miss and branch-miss rates were negligible in both profiles here
(well under 0.01%) - expected and not informative at `--n-objects 1`: one sphere against one
plane touches almost no memory and has one predictable branch path. Re-run with
`--n-objects 100` (or higher) before drawing conclusions about cache behaviour or
collision-detection branch prediction - this default workload is sized for a fast first look,
not for stressing memory/branches.

## Note on the old pipeline

The old pipeline profiled a separate minimal binary, `perf_target.cpp` (removed, along with
its CMake target, once nothing referenced it) - everything here targets
`benchmarks/Performance` instead, for consistency with the MAQAO/MALT scripts (same binary,
same representative workload, comparable reports).
