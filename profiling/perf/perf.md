# Profiling with `perf` — 3D Physics Engine

## Quick reference

| Goal | Command |
|------|---------|
| Record samples | `./profiling/perf/profile.sh` |
| Custom target | `./profiling/perf/profile.sh --target benchmarks/Contact_Forces` |
| Interactive TUI | `perf report -i profiling/perf/results/perf.data` |
| Automated report | `python3 profiling/perf/analyse_perf.py` |
| GUI viewer | `hotspot profiling/perf/results/perf.data` |
| Hardware counters | `perf stat -e cycles,instructions,cache-misses ./build/benchmarks/Bouncing` |
| Annotate source | Inside `perf report`: navigate to symbol → press `a` |

---

## Prerequisites

**Check `perf` is available:**
```bash
perf stat ls
```
If it fails with "permission denied", lower the paranoia level (requires root once per session):
```bash
sudo sysctl kernel.perf_event_paranoid=1
```

**Optional tools (both already installed on this system):**
- `/usr/bin/stackcollapse-perf.pl` and `/usr/bin/flamegraph.pl` — flamegraph generation
- `hotspot` — GUI viewer (Qt-based, most ergonomic for iterative analysis)

**Build note:** The project always compiles with `-g -fno-omit-frame-pointer` (CMakeLists.txt line 168). No special build flags are needed — frame pointers and debug symbols are present in every build type.

---

## Choosing a binary to profile

| Binary | When to use |
|--------|-------------|
| `./build/benchmarks/Bouncing` | **Default choice.** Fixed workload, deterministic, exits cleanly. |
| `./build/benchmarks/Contact_Forces` | Profile the spring-damper contact model and stiff integration. |
| `./build/benchmarks/Bouncing_Conservative` | Profile with perfectly elastic collisions (e = 1.0). |
| `./build/examples/Bouncing` | Profile with YAML config loading, closer to real usage. |
| `./build/PhysicsEngine` | Interactive engine — profile a specific user session. |

Benchmark binaries are preferred because they run a fixed workload and exit without user input.

---

## Recording with `perf record`

### Full pipeline (recommended)
```bash
./profiling/perf/profile.sh
```
This records, generates the flamegraph, and prints the automated hotspot report in one command. See [profile.sh](#profilesh) below.

### Manual recording
```bash
perf record -F 999 -g --call-graph dwarf \
    -o profiling/perf/results/perf.data \
    ./build/benchmarks/Bouncing
```

**Flag explanation:**

| Flag | Meaning |
|------|---------|
| `-F 999` | Sample at 999 Hz — avoids aliasing with 1000 Hz kernel timers |
| `-g --call-graph dwarf` | Capture full call stacks via DWARF unwind info. Required because the engine uses heavily inlined templates and `std` internals |
| `--call-graph fp` | Faster alternative (uses frame pointers instead of DWARF). Slightly shallower stacks but lower overhead |
| `-o path` | Output file for raw binary data |

---

## Analysis methods

### 1. Automated report (`analyse_perf.py`)

```bash
python3 profiling/perf/analyse_perf.py
# or with options:
python3 profiling/perf/analyse_perf.py --input profiling/perf/results/perf.script --top 30
```

Parses `perf.script`, groups symbols by physics-engine subsystem, detects red flags, and writes `profiling/perf/results/hotspots.png`. See [analyse_perf.py](#analyse_perfpy) for full details.

---

### 2. `perf report` — interactive TUI

```bash
perf report -i profiling/perf/results/perf.data
```

**Navigation:**

| Key | Action |
|-----|--------|
| `↑` / `↓` | Move between functions |
| `Enter` | Expand call tree for the selected function |
| `a` | Annotate: show interleaved source + assembly with per-line hit counts |
| `+` | Zoom in on a sub-tree |
| `q` | Quit |

**Key columns:**
- `% Self` — time spent inside the function itself (the hottest indicator)
- `% Children` — time including all callees
- `Shared Object` — which binary or library the symbol comes from

**Useful flags:**
```bash
# Show only self-time (no children aggregation) — cleaner view
perf report --no-children -i profiling/perf/results/perf.data

# Filter to a specific binary
perf report -i profiling/perf/results/perf.data --dso=Bouncing
```

**What to look for in the physics engine:**

| Symbol pattern | Interpretation |
|---------------|----------------|
| `PhysicsWorld::integrate` or `integrateVerlet` high self% | Integration loop is the bottleneck — expected for fine dt |
| `computeAcceleration` or `computeContactForce` wide | Force evaluation expensive — consider caching or SIMD |
| `solveCollisions` / `reboundCollision` dominant | Collision resolution overhead — check O(n²) broad phase |
| `operator new` / `malloc` in top-20 | Dynamic allocation in the hot path — use pre-allocated containers |
| `[unknown]` frames > 10% | Missing symbols — verify the binary was built with `-g` |
| `Vector3D::` math primitives > 5% | Consider SIMD (`__m256d`) for batched vector ops |

---

### 3. Flamegraph

```bash
# Generate from existing perf.data
perf script -i profiling/perf/results/perf.data > profiling/perf/results/perf.script
stackcollapse-perf.pl profiling/perf/results/perf.script \
    | flamegraph.pl > profiling/perf/results/flame.svg
```
Then open `profiling/perf/results/flame.svg` in a browser.

**Reading the flamegraph:**
- **X-axis** — fraction of total samples (wider = more CPU time). Not chronological.
- **Y-axis** — call stack depth: `main` at the bottom, leaf functions at the top.
- **Wide, flat plateaus at the top** — functions with high self-time (the hotspots to optimize).
- **Narrow, tall towers** — deep call chains with little branching (typically the integrator loop).
- **Click any frame** — zooms in to show only that sub-tree, rescaled to 100%.

**Physics-engine patterns to spot:**
- A wide `integrate` plateau with `applyGravityForces` underneath → force evaluation dominates
- A wide `solveCollisions` block → collision response is unexpectedly expensive
- `std::vector` / `_M_realloc_insert` spikes → dynamic memory in the simulation loop
- Mangled template names (`applyVector<std::minus...>`) → heavily inlined math; usually fine unless very wide

---

### 4. `hotspot` GUI

```bash
hotspot profiling/perf/results/perf.data
```

Most ergonomic for iterative analysis. Key tabs:
- **Summary** — quick overview with top functions
- **FlameGraph** — interactive flamegraph (click to zoom, hover for %)
- **Top-Down** — call tree from `main` down; expand to find where time goes
- **Bottom-Up** — grouped by leaf function; best for finding hotspots
- **Caller/Callee** — click any symbol to see exactly what called it and what it calls

---

### 5. `perf stat` — hardware counters

```bash
perf stat -e cycles,instructions,cache-misses,cache-references,branch-misses \
    ./build/benchmarks/Bouncing
```

**Interpreting the output:**

| Metric | Concern threshold | Diagnosis |
|--------|-----------------|-----------|
| IPC (instructions / cycles) | < 1.0 | Memory-bound; consider data layout (AoS → SoA for Vector3D arrays) |
| Cache miss rate | > 5% | Poor spatial/temporal locality; profile object storage order |
| Branch miss rate | > 3% | Unpredictable conditionals in collision detection |
| Very high cycle count with low IPC | — | Likely waiting on memory, not compute |

```bash
# Count-only run (no sampling overhead)
perf stat ./build/benchmarks/Bouncing

# Per-function cache miss breakdown (requires sampling)
perf record -e cache-misses:u -g --call-graph dwarf \
    -o profiling/perf/results/perf_cache.data \
    ./build/benchmarks/Bouncing
perf report -i profiling/perf/results/perf_cache.data
```

---

## `profile.sh`

Automates the full record → flamegraph → analysis pipeline.

```bash
# Profile Bouncing benchmark (default)
./profiling/perf/profile.sh

# Profile a different binary
./profiling/perf/profile.sh --target benchmarks/Contact_Forces
./profiling/perf/profile.sh --target examples/Bouncing

# Profile and open hotspot GUI afterwards
./profiling/perf/profile.sh --target benchmarks/Bouncing --hotspot
```

Output files written to `profiling/perf/results/`:
- `perf.data` — raw binary data
- `perf.script` — text call stacks
- `flame.svg` — flamegraph
- `hotspots.png` — bar chart from `analyse_perf.py`

---

## `analyse_perf.py`

Python script that parses `perf.script` and prints a structured report.

```bash
python3 profiling/perf/analyse_perf.py
python3 profiling/perf/analyse_perf.py --input profiling/perf/results/perf.script --top 30 --no-plot
```

**Report sections:**
1. **Top-N self-time functions** — which functions appear most often as the leaf frame
2. **Subsystem breakdown** — time grouped into Integration, Force computation, Collision, Math, Memory/STL, Other
3. **Red flags** — automatic warnings for memory allocation in hot path, high `[unknown]` rate, dominant single bottleneck
4. **`hotspots.png`** — horizontal bar chart of top-N by self-%, color-coded by subsystem

---

## Worked example: interpreting a typical run

After running `./profiling/perf/profile.sh` against the `Bouncing` benchmark:

1. **Check `analyse_perf.py` output first** — if Integration > 80% and Math primitives < 2%, the bottleneck is the integration loop itself, not the math operations within it. Optimize `physicsWorld.cpp:integrate`.

2. **Open the flamegraph** — look for anything unexpectedly wide outside `integrate`. A wide `loadFromFile` or YAML parsing block means the config-loading overhead is being profiled; use a benchmark binary instead.

3. **Use `perf report -a`** on the hottest symbol — press `a` to see which line of `physicsWorld.cpp` accumulates the most hits. Often a single inner-loop line (e.g. the position update) accounts for > 30% of all samples.

4. **Run `perf stat`** — if IPC < 1.5 on a modern CPU while integration dominates, the math is compute-bound. If IPC < 0.8, it's memory-bound and data layout should be reviewed.
