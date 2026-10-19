# Performance Benchmark Binary

`benchmark.cpp` builds `benchmarks/Performance`, the binary every benchmark and profiling
script measures. This page is its reference (scene, options, CSV columns) and lists how to
install the profiling tools. For how to run the benchmarks and read the results, see
[`BENCHMARK_TOOLS_GUIDE.md`](BENCHMARK_TOOLS_GUIDE.md).

## Scene

| Parameter | Value |
|---|---|
| Dynamic objects | N spheres (`--n_objects`, radius 2, mass 1, restitution 1), on a grid 3 radii apart, starting at z = 20 |
| Static objects | 1 ground plane at z = 0, sized to the grid |
| Collisions | Impulse model by default; spring-damper contact forces with `--contact-forces` |
| Gravity | Default engine gravity |

The scene is rebuilt from scratch before every run so no state leaks between repetitions.

## Run structure

```
[ warm-up runs (discarded) ] [ timed runs (recorded) ] [ one step-timing run ]
```

- **Warm-up runs** (`--n_warmup`): the CPU reaches a steady clock and the caches warm up.
- **Timed runs** (`--n_runs`): each full run of `dur / dt` steps is timed; the statistics below
  come from these.
- **Step-timing run**: one more run, timed in ~500 batches of steps (nanosecond clock), to
  measure how regular the cost of a step is.

## Command-line options

| Flag | Default | Description |
|---|---|---|
| `--solver` | `Verlet` | Integrator: `Euler`, `Verlet` or `RK4` |
| `--dt` | `1e-3` | Timestep (s) |
| `--dur` | `22.0` | Simulated duration (s); the run executes `dur / dt` steps |
| `--n_objects` | `1` | Number of spheres |
| `--n_warmup` | `1` | Discarded warm-up runs |
| `--n_runs` | `5` | Timed runs |
| `--contact-forces` | off | Use the spring-damper contact model instead of impulses |
| `--out` | `benchmarks/performance/results` | Directory for `benchmark.csv` (created if needed) |
| `--append` | off | Append a row to `benchmark.csv` instead of overwriting it (no header) |

The scripts always pin the binary to one P-core (`taskset`, see the guide §3.2). When running
it by hand, do the same:

```bash
taskset -c 2 ./build/benchmark_optimised/benchmarks/Performance --solver RK4 --n_objects 50
```

## Output: `benchmark.csv`

One row per invocation.

| Column | Description |
|---|---|
| `solver`, `dt`, `n_objects` | Configuration |
| `cpu_us_mean`, `cpu_us_min`, `cpu_us_max` | Duration of a timed run (µs): mean, best, worst |
| `cpu_us_stddev`, `cv_percent` | Standard deviation (Bessel-corrected) and coefficient of variation = stddev / mean × 100. CV > 10% means a noisy measurement |
| `wall_time_s` | Mean run duration (s) |
| `time_per_step_us`, `steps_per_second` | Mean cost of one step and the corresponding throughput |
| `flops_per_step`, `gflops_per_second` | Analytical FLOP estimate (below) and the resulting rate |
| `step_us_mean`, `step_us_min`, `step_us_max`, `step_us_stddev`, `step_us_cv` | Distribution of the per-step cost over the step-timing run's batches |
| `peak_rss_kb` | Peak resident memory of the process (`getrusage`) |

### FLOP estimate

Counted analytically from the algorithm, not from hardware counters:

```
force     = 6·N (gravity) + 45·N(N−1)/2 (pairwise collision detection)
integrate = 18·N for Verlet, 12·N otherwise
per step  = force + integrate          (Euler, Verlet)
          = 4·force + integrate        (RK4: forces evaluated at 4 stages)
GFLOP/s   = flops_per_step × steps / (wall_time_s × 10⁹)
```

Use it to compare configurations, not as a fraction of the CPU's peak.

## Installation (profiling tools)

`benchmark.cpp` itself needs nothing beyond the project's normal build (see the root
`README.md`/`scripts/build.sh`). The three tools below are only needed for the deeper
analysis scripts under `perf/`, `maqao/`, and `malt/` — hardware counters, vectorisation
analysis, and memory allocation profiling respectively.

### perf

```bash
# Arch:
sudo pacman -S perf

# Ubuntu/Debian:
sudo apt install linux-tools-common linux-tools-generic
```

**macOS: not supported, no equivalent.** `perf` wraps the Linux kernel's `perf_events`
subsystem directly — it doesn't exist there. Closest built-in tools are Instruments (GUI,
ships with Xcode) or `dtrace` (blocked by System Integrity Protection for much of what this
pipeline would need); neither is a drop-in replacement for these scripts.

Optional, referenced by `perf/perf.md`: `flamegraph` (provides `stackcollapse-perf.pl` /
`flamegraph.pl` — AUR/chaotic-aur on Arch, cloned from
[Brendan Gregg's FlameGraph repo](https://github.com/brendangregg/FlameGraph) on Ubuntu/Debian)
and `hotspot` (GUI viewer — `chaotic-aur/hotspot` on Arch, `hotspot` via apt on recent
Ubuntu). Neither is required; `profile_perf.sh` skips the flamegraph step gracefully if
they're missing.

### MAQAO

```bash
# Arch — AUR package:
yay -S maqao-bin

# Any distro (incl. Ubuntu/Debian) — generic binary release, no package needed
# (check maqao.org/download.html for the current version):
curl -LO http://www.maqao.org/maqao_archive/maqao.x86_64.2025.1.0.tar.xz
tar xf maqao.x86_64.2025.1.0.tar.xz
export PATH="$PWD/maqao.x86_64.2025.1.0:$PATH"   # add to your shell rc to persist
```

**macOS: not supported.** MAQAO ships binaries for "64-bit Linux based operating systems"
only, per its [download page](https://www.maqao.org/download.html) — no macOS build exists.

### MALT

No package on any of Arch/AUR or Ubuntu/Debian — build from source:

```bash
# Arch:
sudo pacman -S --needed base-devel cmake gcc make openssl libunwind elfutils \
                         nodejs npm nlohmann-json graphviz python curl

# Ubuntu/Debian:
sudo apt install cmake g++ make libssl-dev libunwind-dev libelf-dev nodejs npm \
                  nlohmann-json3-dev graphviz python3-dev curl

git clone https://github.com/memtt/malt.git
cd malt && mkdir build && cd build
../configure --prefix=/usr/local
make
sudo make install
```

This installs a `malt` command (and `malt-webview`) onto `PATH`. If `malt-webview` then fails
with `cannot open shared object file: libmalt-reader.so.1`, refresh the linker cache:
`sudo ldconfig` (or run once with `LD_LIBRARY_PATH=/usr/local/lib` prefixed).

**macOS: not supported.** MALT's own dependencies page only documents Debian/Ubuntu,
Fedora/Rocky, Arch, and Gentoo. It relies on `libelf`/`libunwind` for symbol resolution, both
built around the ELF binary format macOS doesn't use (Mach-O).
