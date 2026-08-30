# Benchmark Tools Guide - perf + MAQAO + MALT, from zero

A from-scratch tutorial for building the Phase 6 (CPU Performance Engineering,
`docs/ROADMAP.md`) benchmarking pipeline yourself, and actually understanding what each
number means while you do it.

**How to use this document:** read Part 0 once - it's the shared vocabulary the rest of the
guide leans on. Then work through Part 1 (perf), Part 2 (MAQAO), Part 3 (MALT) in order -
each is self-contained, each ends with a working script and a "checkpoint" question to test
whether you actually understood the output or just copied a command. Part 4 wires the three
scripts together. This document contains no finished scripts - skeletons and checklists only.
Writing the code is the point.

---

## Part 0 - Foundations (read this once, before anything else)

### 0.1 Why wall-clock time isn't enough

`time ./my_binary` tells you *that* something is slow. It tells you nothing about *why*. The
three tools in this guide each attack "why" from a different angle:

| Approach | Tool | How it works |
|---|---|---|
| Watch hardware counters while the program runs | **perf stat** | The CPU has a small set of physical registers (the PMU - Performance Monitoring Unit) that count events like "cycles elapsed" or "cache miss occurred" in hardware, for free, as the program executes. `perf stat` reads them before/after. |
| Sample *where* the program is executing, many times a second | **perf record** | The kernel interrupts the program ~1000×/second, records the call stack at that instant, and stops. After thousands of samples, "which function appears most often" ≈ "which function costs the most time." This is *statistical*, not exact. |
| Analyse the compiled machine code without running it | **MAQAO CQA** | Reads the binary's assembly for a specific loop and reasons about it structurally - e.g. "these instructions are 256-bit AVX2 vector instructions, therefore this loop is vectorised." No execution needed, so no sampling noise, but it can't tell you how often that loop actually runs. |
| Intercept specific function calls precisely | **MALT** | Replaces `malloc`/`free` with wrapper versions (via `LD_PRELOAD`, see §0.3) that log every call before forwarding it to the real allocator. Exact counts, not samples - but only for the calls it intercepts. |

None of these four is "the profiler." You need at least a sampling tool (perf) and a
static/structural tool (MAQAO or a memory interceptor) because they answer different
questions. A function can look fine to `perf stat` (good IPC) while still running fully
scalar - IPC doesn't measure vectorisation, it measures pipeline utilisation.

### 0.2 CPU vocabulary you need before any of this makes sense

You don't need a computer architecture course, but these terms show up constantly in every
tool's output and you should be able to define each in one sentence:

- **Cycle**: one tick of the CPU clock. Everything is measured relative to cycles because
  clock speed varies (frequency scaling, turbo boost).
- **Instruction**: one machine-code operation (add, load, compare, ...).
- **IPC (Instructions Per Cycle)**: `instructions ÷ cycles`. The single most-used summary
  number. Modern superscalar CPUs can retire 3-5+ instructions per cycle in theory. IPC well
  below that (say < 1.0-1.5 depending on the CPU) usually means the pipeline is stalling -
  waiting on something, not computing.
- **Pipeline stall**: a cycle where the CPU *could* execute an instruction but can't, because
  the instruction it needs isn't ready yet (data dependency, cache miss, branch
  misprediction...).
- **Cache hierarchy**: L1 (fastest, smallest, per-core, ~4 cycles latency) → L2 (per-core or
  shared, ~12 cycles) → L3/LLC ("Last Level Cache", shared across cores, ~40 cycles) → main
  memory / RAM (~200+ cycles). A "cache miss" means the requested data wasn't in that cache
  level and the CPU had to go further out - the deeper it goes, the more cycles are wasted
  waiting.
- **Branch prediction / misprediction**: the CPU guesses which way an `if` will go *before*
  evaluating the condition, so it can keep executing speculatively. A wrong guess
  (misprediction) means it has to throw away the speculative work and restart - expensive
  (~15-20 cycles typically). Unpredictable branches (e.g. data-dependent collision checks)
  cause this.
- **SIMD / vectorisation**: instead of one instruction operating on one number, a vector
  instruction operates on several numbers packed into one wide register at once. E.g. AVX2
  gives 256-bit registers = 4 `double`s (or 8 `float`s) processed by a single instruction. A
  loop the compiler successfully vectorises can, in the best case, run ~4x faster than the
  scalar version for the same operation - this is the entire premise of the Phase 6 SIMD
  roadmap item.
- **FLOP/s and peak FLOP/s**: floating-point operations per second, and the theoretical
  maximum your specific CPU can sustain (a function of clock speed × cores × SIMD width ×
  instructions-per-cycle for FP ops). "Achieving 5% of peak" is a meaningful, comparable
  number; "1.2 GFLOP/s" alone is not, until you know what peak is.
- **Arithmetic intensity**: FLOPs performed ÷ bytes moved from memory. Low intensity (e.g. a
  simple vector add: 1 FLOP per 2 loads + 1 store) means performance is capped by memory
  bandwidth, not compute speed, no matter how fast the CPU's ALUs are. This is the X-axis of
  a **roofline plot** (§3.6 in Part 2) - the single most useful mental model for "should I
  even bother optimising the arithmetic here."

### 0.3 What `LD_PRELOAD` is (needed to understand MALT)

On Linux, `LD_PRELOAD=/path/to/lib.so ./program` tells the dynamic linker to load that
library *before* any other, so if it defines a function with the same name as one the program
calls (e.g. `malloc`), the preloaded version wins. MALT ships a shared library that defines
`malloc`/`free`/`new`/`delete` etc., logs the call (size, call stack, timestamp), then calls
the real allocator. No recompilation needed - that's why it's "drop-in." In practice you won't
type `LD_PRELOAD=` yourself: MALT installs a `malt` wrapper command that sets this up
internally (§3.2) - worth knowing the mechanism anyway, since it's the same trick a lot of
allocator-swapping/interposition tooling uses.

### 0.4 The project's benchmark setup you'll be driving

You already have (don't rebuild these - they're the given foundation for everything below):

- `scripts/build.sh` - builds the project with arbitrary CMake flags into a chosen build dir.
- `benchmarks/performance/benchmark.cpp` → compiled binary `benchmarks/Performance` - runs a
  fixed scene (one bouncing sphere) for a chosen solver/dt/duration, with warmup + repeated
  timed runs. See [`benchmarks/performance/README.md`](README.md) for its CLI flags and what
  it already measures (timing, analytic FLOP/s, peak RSS).
- Two build **profiles**, both `Release`:
  - `scalar`: `-O2 -fno-tree-vectorize -fno-inline` - deliberately crippled, your control
    group. If any tool reports this build as vectorised, something's wrong with your
    measurement, not the compiler.
  - `optimised`: `-O3 -march=native` - what you're actually trying to understand.

Every tool in this guide gets pointed at the *same* binary+arguments so results are
comparable across tools and across profiles. Pick one fixed, cheap-to-run invocation now and
reuse it everywhere below, e.g.:

```text
--solver Verlet --dt 1e-3 --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

(A single object keeps early runs fast while you're learning the tools; you can scale
`--n_objects` up later once the pipeline works, to see how conclusions change with problem
size.)

### 0.5 Reproducibility ground rules (apply to all three tools)

- **Warm up first.** The first run after a cold start pays for cache/TLB warming and CPU
  frequency ramp-up. The benchmark binary already discards warmup runs internally
  (`--n_warmup`) - keep at least 1.
- **Run more than once, look at variance, not just the mean.** A single measurement of
  anything on a general-purpose OS (not a dedicated benchmarking node) can be noise. If a
  tool supports repeated measurement (perf's `-r`), use it.
- **Know what "noisy" looks like** before you start: background processes, thermal
  throttling, and CPU frequency scaling all inflate variance. You don't need to eliminate
  this yet (that's an optional hardening step in Part 1), just recognise it when e.g. two
  supposedly-identical runs differ by 20%.

---

## Part 1 - perf

### 1.1 Concept: what perf actually is

`perf` is the standard Linux profiler - a userspace CLI on top of the kernel's `perf_events`
subsystem. It has two modes you'll use:

- **`perf stat`**: read PMU hardware counters across the whole run of a program, print
  totals/derived ratios at the end. Zero sampling - exact hardware-counted numbers (subject
  to the multiplexing caveat in §1.4).
- **`perf record`**: periodically interrupt the program (default ~1000 Hz, i.e. every ~1ms)
  and record the call stack at that instant into a file. After the run, `perf report` /
  `perf script` turn thousands of these samples into "which function was on-CPU most often."

### 1.2 Hands-on: install & permission check

```bash
perf --version
```

If missing, install your distro's `perf`/`linux-tools` package. Then check you're allowed to
read hardware counters:

```bash
perf stat ls
```

If this fails with a permission error, the kernel's paranoia setting is too strict for a
regular user:

```bash
sudo sysctl kernel.perf_event_paranoid=1
```

(This resets on reboot; make it permanent via `/etc/sysctl.d/` if you'll be doing this
regularly - your call, not scripted here since it needs root.)

**Checkpoint:** run `perf stat ls` again. You should see a summary block at the bottom with
`task-clock`, `context-switches`, `cycles`, `instructions`, and an `insn per cycle` line.
Can you say in one sentence why `ls` (a trivial program) still shows a non-zero cycle count?
*(It has to be loaded, linked, and its process torn down - that's not free even for a no-op
program.)*

### 1.3 Hands-on: your first real measurement

Build the optimised profile once if you haven't:

```bash
./scripts/build.sh --build-dir build/benchmark_optimised --build-type Release \
    -D3DPE_BUILD_TESTS=OFF -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native -DNDEBUG"
```

Then:

```bash
perf stat ./build/benchmark_optimised/benchmarks/Performance \
    --solver Verlet --dt 1e-3 --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

Read the default output top to bottom. `perf stat` with no `-e` flags gives you a curated
default set (task-clock, context-switches, cycles, instructions, branches, branch-misses).
Note the `insn per cycle` line - that's IPC, from §0.2. Write down the number; you'll compare
it against the scalar build in a moment.

Now build the scalar profile and repeat:

```bash
./scripts/build.sh --build-dir build/benchmark_scalar --build-type Release \
    -D3DPE_BUILD_TESTS=OFF \
    -DCMAKE_CXX_FLAGS_RELEASE="-O2 -fno-tree-vectorize -fno-inline -DNDEBUG"

perf stat ./build/benchmark_scalar/benchmarks/Performance \
    --solver Verlet --dt 1e-3 --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

**Checkpoint:** compare cycles, instructions, and IPC between the two runs. The scalar build
should take noticeably more cycles for the same work. Is IPC higher or lower on the scalar
build, and can you explain why using only §0.2's definitions? *(Often scalar IPC looks
similar or even higher - IPC measures pipeline efficiency, not work done per instruction; the
optimised build does more work per instruction via SIMD, which IPC alone won't show you. This
is exactly the blind spot MAQAO fixes in Part 2.)*

### 1.4 Concept: choosing your own counters, and the multiplexing trap

Beyond the default set, request specific hardware events with `-e`:

```bash
perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses \
    ./build/benchmark_optimised/benchmarks/Performance --solver Verlet --dt 1e-3 \
    --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

List all available events on your machine with `perf list`.

**The trap:** most CPUs expose only 4-6 *general-purpose* PMU counter registers
simultaneously (a few more are fixed-purpose, e.g. always-on cycles/instructions counters).
If you request more distinct events than there are physical registers, `perf` **time-slices**
between groups of them and *statistically scales up* the results to estimate what the full
count would have been - you get an estimate, not an exact count, and `perf stat`'s text
output marks this in the right-hand column when it happens. Two ways to avoid it:

- **Group events that need to be exact together** with braces: `-e '{cycles,instructions}'`
  forces that pair to be counted in the same time slice, so their *ratio* (IPC) stays exact
  even if the absolute counts of other ungrouped events around them get scaled.
- **Ask for fewer total events per run.** Run twice with 4 events each rather than once with
  8, if you need all 8 to be exact.

**Checkpoint:** run the same `perf stat -e ...` command twice - once with 8+ ungrouped
events, once with the same events split into groups of 2-3 with braces. Do you see a
percentage annotation (e.g. `(66.67%)`) next to counter values in the ungrouped run that
disappears in the grouped one? That annotation is `perf` telling you "this event was only
actually measured 66.67% of the time and I scaled it up" - grouping is how you make it go
away for the events you actually care about.

### 1.5 Concept: building up a useful metric set, one at a time

Don't reach for 10 events immediately - add them one at a time and understand what each buys
you over what you already had:

1. **`cycles`, `instructions`** → IPC. Your first-order "is the pipeline busy" signal.
2. **`branches`, `branch-misses`** → branch miss rate = `branch-misses ÷ branches × 100`. A
   rate above roughly 1-3% on a numeric loop is worth investigating - for this project,
   collision-detection conditionals are the natural suspect, not the integrator math.
3. **`L1-dcache-loads`, `L1-dcache-load-misses`** and **`LLC-loads`, `LLC-load-misses`** →
   cache miss rates at two levels. L1 miss rate above a few percent, or a high fraction of L1
   misses that *also* miss LLC (i.e. going all the way to RAM), points at poor data layout -
   directly relevant to the SoA-vs-AoS roadmap item.
4. **`stalled-cycles-frontend`, `stalled-cycles-backend`** → `stalled ÷ cycles × 100` tells
   you what fraction of cycles the CPU had nothing useful to issue. Frontend stalls = fetch/
   decode couldn't keep up (code size, instruction cache); backend stalls = waiting on
   execution units or memory (this is where cache misses and long dependency chains show up).
5. **Top-Down Microarchitecture Analysis**: `perf stat --topdown` (or `-M TopdownL1` on
   older `perf`/kernel combinations - check `perf stat --topdown --help` on your machine)
   classifies every pipeline slot into one of four buckets automatically: *Retiring* (useful
   work), *Front-End Bound*, *Back-End Bound*, *Bad Speculation* (mispredicted branches).
   This is the fastest way to get a first-order "what category is the bottleneck" answer
   without manually cross-referencing 8 separate counters yourself - run it as a *separate*
   invocation (it uses its own internal counter grouping, don't mix it into a custom `-e`
   list).

**Checkpoint:** run the Top-Down pass on the optimised build. Which of the four buckets
dominates? Does that match your guess from the individual counters in steps 1-4? If Top-Down
says Back-End Bound and your L1/LLC miss rates from step 3 are low, what does that suggest
instead? *(Likely a long dependency chain or execution-port pressure rather than memory -
exactly the kind of thing MAQAO's CQA bottleneck classification in Part 2 will confirm at the
loop level.)*

Add `-r 5` to any `perf stat` command to run it 5 times and get mean ± stddev per counter
instead of trusting one sample - cheap insurance against the noise described in §0.5.

### 1.6 Concept: sampling with `perf record`

`perf stat` gives you *aggregate* numbers for the whole run. It can't tell you *which line of
code* is responsible. For that, sample:

```bash
perf record -F 999 -g --call-graph dwarf \
    -o results/perf.data \
    ./build/benchmark_optimised/benchmarks/Performance \
    --solver Verlet --dt 1e-3 --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

- `-F 999` - sample at 999 Hz rather than exactly 1000 Hz, deliberately avoiding aliasing
  with the kernel's own 1000 Hz timers (sampling at exactly the same frequency as a periodic
  system event can bias which instant you happen to always catch).
- `-g --call-graph dwarf` - capture the full call stack per sample using DWARF debug info to
  unwind it. Needed here because heavily inlined C++ templates and STL internals confuse the
  cheaper frame-pointer unwinder. `--call-graph fp` is faster/lower-overhead but shallower -
  an option once `dwarf` overhead becomes annoying on longer runs.

**Checkpoint, conceptual:** why is `perf record`'s output *statistical* rather than exact,
while `perf stat`'s counters are exact (mod multiplexing)? *(Because `perf record` only knows
what the program was doing at the ~1000 sampled instants, not continuously - a function that
runs for a very short but frequent burst could be under- or over-represented by chance,
especially with few total samples. `perf stat` counters increment continuously in hardware
regardless of whether anyone's looking.)*

### 1.7 Hands-on: reading the samples

Text/interactive view:

```bash
perf report -i results/perf.data
```

Navigate with arrow keys, `Enter` to expand a call tree, `a` on a selected function to see
**annotated source+assembly** - literally which line accumulated the most samples. `q` to
quit. Look at `% Self` (time in that function alone) vs `% Children` (including everything it
calls) - `% Self` is where you actually optimise; `% Children` just tells you where in the
call tree that cost is nested.

Flamegraph (visual, easier to scan for "what's wide"):

```bash
perf script -i results/perf.data > results/perf.script
stackcollapse-perf.pl results/perf.script | flamegraph.pl > results/flame.svg
```

(Install `FlameGraph` from Brendan Gregg's repo if `stackcollapse-perf.pl`/`flamegraph.pl`
aren't found - check with `command -v flamegraph.pl` first.) Open the SVG in a browser.
X-axis = fraction of samples (width, **not** chronological time), Y-axis = call stack depth,
`main` at the bottom. Wide flat plateaus at the top are your hotspots. Click any frame to
zoom into that sub-tree.

**Checkpoint:** find the widest plateau in the flamegraph. Open `perf report`, find the same
function, press `a`, and identify the single source line with the highest sample count inside
it. Does it match what you'd guess just from reading the source (e.g. the position-update
line inside the integrator), or does it surprise you (e.g. time going into a
constructor/destructor, or STL container overhead)?

### 1.8 Build: your `perf` script

You now understand every piece. Build a script (e.g. `benchmarks/performance/perf/profile_perf.sh`
if starting fresh, or revise what's there) that, for a given build profile:

1. Runs a grouped, repeated `perf stat` pass (§1.4, §1.5 steps 1-4) → save to a text file.
2. Runs a separate Top-Down pass (§1.5 step 5) → append or save separately.
3. Runs `perf record` (§1.6) → `perf script` → flamegraph (§1.7), guarding the flamegraph
   step with a check for `stackcollapse-perf.pl`/`flamegraph.pl` on `PATH` so the script
   doesn't hard-fail on a machine without them.
4. Loops over both build profiles (scalar/optimised) so you always get a side-by-side.

**Verify:** run your script against both profiles. You should end up with two `perf stat`
reports, two Top-Down reports, and two flamegraphs, and be able to point at one number in
each that differs meaningfully between scalar and optimised (IPC, a stall percentage, or
which function dominates the flamegraph).

---

## Part 2 - MAQAO

### 2.1 Concept: static analysis, and why perf alone doesn't answer "is this vectorised"

Recall §0.2: SIMD packs several numbers into one wide register so one instruction processes
several at once. `perf` counters don't distinguish a scalar `add` from a 256-bit vector
`add` - both just count as "one instruction." IPC can look identical whether a loop is
vectorised or not. To know whether the compiler actually vectorised a loop, and how
*efficiently* (using the full register width vs only part of it), you need to look at the
actual assembly instructions the compiler generated - that's what MAQAO's CQA module does.

MAQAO has three modules you'll use, each answering a different sub-question:

- **CQA (Code Quality Analyzer)** - static: reads the compiled binary's assembly for a loop,
  no execution needed. "Is this loop vectorised, how efficiently, what's the theoretical
  bottleneck."
- **Lprof** - dynamic: actually runs the binary, samples like `perf record` but aggregates by
  *source loop* instead of by function, and estimates achieved FLOP/s. "Which loop actually
  dominates runtime, and how close to peak is it."
- **ONE-View** - combines CQA and Lprof (plus topology data) into one HTML dashboard. There's
  no separate "Roofline" module in current MAQAO (confirmed: `maqao --list-modules` only
  lists `analyze`/`cqa`/`lprof`/`madras`/`oneview`) - the roofline-relevant numbers (peak
  FLOP/cycle, achieved FLOP/cycle) are printed directly in CQA's own output per loop; §2.5
  shows how to read them as a roofline placement yourself.

### 2.2 Hands-on: install

Download the prebuilt release tarball for your OS/architecture from the MAQAO project site
(no root needed - self-contained), extract it (e.g. to `~/opt/maqao/`), add its `bin/` to
`PATH`.

```bash
maqao --version
```

Check the printed microarchitecture info recognises your CPU - CQA's "X% of peak" numbers
(§2.5) depend on knowing your exact CPU model correctly.

**Checkpoint:** `maqao cqa --help` and `maqao lprof --help` should both print usage without
error. If either subcommand is missing, your download may be an incomplete/partial release -
re-check what you downloaded before going further.

### 2.3 Hands-on: your first CQA report

CQA needs a binary built **with debug info** (already always-on in this project's
`CMakeLists.txt`, so nothing to change) so it can map assembly back to source loops.

```bash
maqao cqa ./build/benchmark_optimised/benchmarks/Performance
```

(Check `maqao cqa --help` for how your installed version selects a specific loop/function -
options differ slightly by MAQAO version; you likely want to point it at the function
containing your hot loop, e.g. the integrator step, once you know which one that is from
Part 1's flamegraph.)

Read the report for that loop. You're looking for:

- **Vectorisation ratio** - what fraction of instructions in the loop are vector (SIMD) forms
  rather than scalar. 0% means the compiler didn't vectorise this loop at all.
- **Vector Efficiency Ratio (VER)** - of the vector instructions present, what fraction of
  the register width is actually used. A loop can report a decent vectorisation ratio while
  VER is low (e.g. using 128-bit SSE-style width inside a CPU capable of 256-bit AVX2) - VER
  is what catches that, a plain vectorisation ratio would not.
- **Potential speedup** - CQA's estimate of the cycles-per-iteration improvement if VER were
  100%. This is your prioritisation signal for *which* loop is worth hand-optimising first.
- **Bottleneck classification** - CQA's static guess at what limits the loop: front-end
  (instruction fetch/decode), a specific execution port, or a data dependency chain.

Now run the exact same command against the **scalar** binary:

```bash
maqao cqa ./build/benchmark_scalar/benchmarks/Performance
```

**Checkpoint:** the scalar build's vectorisation ratio for the same loop should be ~0%
(that's why it's the control group - `-fno-tree-vectorize` explicitly disabled this). If it
isn't ~0%, something about the scalar build config is wrong and you should fix that before
trusting any optimised-build numbers, since you'd have no valid baseline to compare against.

### 2.4 Hands-on: Lprof - does the vectorised loop actually run the most?

CQA told you what's *possible* for a given loop. Lprof tells you what actually *happens* when
you run the program - which loop dominates wall time, and what fraction of peak FLOP/s it
achieves:

```bash
maqao lprof ./build/benchmark_optimised/benchmarks/Performance \
    --solver Verlet --dt 1e-3 --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

Check the generated report for per-loop time share and measured FLOP/s. Cross-reference: is
the loop CQA said had the best vectorisation potential the *same* loop Lprof says dominates
runtime? If not, your optimisation priority is whichever loop actually costs time in
practice, not whichever looks most vectorisable in isolation.

**Checkpoint:** compare Lprof's "% of peak FLOP/s achieved" for your hottest loop against the
IPC you measured with `perf stat` in Part 1 for the same run. They're measuring related but
different things - can you articulate the difference in one sentence? *(IPC counts
instructions regardless of whether each one processes 1 or 4+ numbers; % of peak FLOP/s
accounts for SIMD width, so it's a truer measure of "how much useful arithmetic work per
cycle," which is exactly why IPC alone missed the scalar-vs-vectorised distinction back in
§1.3's checkpoint.)*

### 2.5 Concept: the Roofline model

Two numbers place any loop on a 2D plot:

- **X-axis - arithmetic intensity**: FLOPs performed ÷ bytes moved from memory, for that
  loop. Low intensity = the loop is "thin" arithmetically relative to how much data it
  touches (e.g. a vector add - 2 loads + 1 store per 1 FLOP).
- **Y-axis - achieved GFLOP/s**: from Lprof's dynamic measurement.

Two ceilings bound every possible point on that plot for your specific machine:

- A flat **compute roof** at your CPU's peak GFLOP/s (independent of intensity).
- A rising **memory-bandwidth roof**, `intensity × peak_bandwidth`, which is low at low
  intensity and eventually crosses the compute roof.

A loop plotted **under the memory roof, to its left** is **memory-bound** - no amount of
arithmetic optimisation (intrinsics, unrolling) will help; you need fewer bytes moved
(better data layout, e.g. the SoA-vs-AoS roadmap item) or better cache reuse. A loop plotted
**near the compute roof, to the right** is **compute-bound** - arithmetic optimisation
(vectorisation, reducing instruction count) is where the payoff is.

There's no dedicated "maqao roofline" command (confirmed against a real install - see
maqao/maqao.md) - build the Y-axis yourself from CQA's own per-loop output, which already
prints exactly this: `"X% of peak computational performance is used (A out of B FLOP per
cycle)"` is your achieved-vs-peak pair directly. Place your hottest loop on the plot for both
profiles using that number.

**Checkpoint:** is your hot loop compute-bound or memory-bound? Does that match your guess
from Part 1's cache-miss rates and Top-Down classification (§1.5)? If the roofline placement
suggests memory-bound but Part 1's L1/LLC miss rates looked low, what would you check next? *(Whether
the loop is bandwidth-limited even with a high cache-hit rate - e.g. streaming through a
large array once, where every access hits *some* cache level but the sheer volume of bytes
moved still saturates bandwidth. This is a case low miss *rates* can hide but arithmetic
intensity exposes directly.)*

### 2.6 Build: your MAQAO script

Build `benchmarks/performance/maqao/profile_maqao.sh`, mirroring the shape of the perf script
from Part 1 (§1.8) - same representative binary+args so results are directly comparable
tool-to-tool:

1. Run CQA (static, no execution) against the built binary for the hot loop(s).
2. Run Lprof (dynamic - actually executes) for the same invocation.
3. Run ONE-View for a combined report (optional - CQA + Lprof already give you everything;
   ONE-View is mainly useful as a single shareable HTML summary).
4. Run the whole thing once per build profile (scalar/optimised) - the scalar-vs-optimised
   *comparison* is the point, not either number alone.

Save output under `benchmarks/performance/maqao/results/<profile>/`.

**Verify:** for both profiles, you can state the vectorisation ratio and VER of the hottest
loop, and place it on a roofline plot using CQA's own peak-FLOP/cycle numbers. The optimised
profile's ratio/VER should be visibly higher than scalar's; if it isn't, that itself is a
finding - the compiler isn't vectorising what you expected, worth investigating with
`-fopt-info-vec` before writing any intrinsics.

---

## Part 3 - MALT

### 3.1 Concept: why perf's flamegraph isn't enough for memory

A `perf record` flamegraph can show `malloc`/`operator new` as a wide-ish frame, telling you
"allocation happens somewhere in this call path." It can't tell you: how many allocations,
what sizes, whether they're growing/reallocating containers, or whether any are leaked. MALT
(§0.3) intercepts every allocator call directly, so it reports exact counts and sizes, not
samples.

### 3.2 Hands-on: install

No package for most distros — build from source (confirmed against MALT's own docs;
Ubuntu/Debian package names shown, substitute your distro's equivalents):

```bash
sudo apt install cmake g++ make libssl-dev libunwind-dev libelf-dev nodejs npm \
                  nlohmann-json3-dev graphviz python3-dev curl
git clone https://github.com/memtt/malt.git
cd malt && mkdir build && cd build
../configure --prefix=/usr/local
make
sudo make install
```

This installs a `malt` command (and `malt-webview`) onto `PATH`. Unlike the raw `LD_PRELOAD`
mechanic explained in §0.3, you don't set that up by hand — `malt` wraps a program and does it
internally: `malt {YOUR_PROGRAM} [ARGS]`. No project-side build flag changes needed either -
debug info and frame pointers are already always on (§1.6 already established this for
`perf`; MALT benefits from the same settings for the same reason - accurate call-stack
attribution).

**Checkpoint:** `malt /bin/ls` should still just list files (proof the wrapper itself works)
and drop a report file named something like `malt-ls-<pid>.json` in the working directory -
find it before moving to the real binary.

### 3.3 Hands-on: your first memory report

```bash
malt ./build/benchmark_optimised/benchmarks/Performance \
    --solver Verlet --dt 1e-3 --dur 2.0 --n_warmup 1 --n_runs 1 --n_objects 1
```

Locate the report MALT drops (`malt-Performance-<pid>.json` by default, in the working
directory - check `malt --help` for output-path/naming options on your installed version). If
`malt-webview` is on `PATH` (it should be, installed alongside `malt`), open the report there
for the visual timeline; otherwise the raw JSON is still readable.

### 3.4 Interpreting the report

- **Peak / live allocated memory over time** - a timeline, not just one endpoint number. Does
  the peak happen during scene construction (expected, one-time cost) or does memory usage
  keep climbing during the simulation loop itself (that would be a real problem - a
  steady-state loop shouldn't grow memory over time)?
- **Allocation count + call-stack attribution** - which function calls `new`/`malloc`, and
  how many times across the run. For a fixed-size scene, an allocation count much larger than
  the object count is suspicious - something inside the per-step loop is allocating.
- **Allocation size histogram** - repeated allocations of the same size are a pool/arena
  allocator candidate; this becomes directly relevant once you touch the SoA/data-layout
  roadmap item, since that's exactly when object storage patterns change.
- **Realloc count** - a growing `std::vector` (e.g. a contact list) that reallocates and
  copies its contents every time it outgrows its capacity shows up here; a large realloc
  count on something that should be roughly fixed-size per step is a red flag independent of
  raw allocation count.
- **Leak count** - allocations never freed by process exit. Not a performance metric, but
  free with the same run - worth a glance.

**Checkpoint:** cross-check MALT's peak memory number against the `peak_rss_kb` field the
benchmark binary already writes to `benchmark.csv` (see
[`benchmarks/performance/README.md`](README.md#memory)) for the same configuration. They
measure slightly different things (RSS includes everything the OS mapped for the process;
MALT's peak is heap allocations it specifically tracked) - are they in the same ballpark? A
huge mismatch would suggest large non-heap memory use (e.g. stack, mapped files) dominating,
which MALT wouldn't be the right tool to explain.

### 3.5 Concept: why one run is enough here

Unlike perf and MAQAO, you don't need a scalar-vs-optimised MALT comparison. Compiler flags
like `-march=native` change *how* arithmetic is compiled, not *which* allocator calls the
source code makes - the same `new`/`push_back` calls happen in both binaries. One run against
the optimised profile is sufficient; running MALT against scalar too would just cost time for
no new information.

### 3.6 Build: your MALT script

Build `benchmarks/performance/malt/profile_malt.sh`:

1. Run the representative binary once (optimised profile only, per §3.5) via the `malt`
   wrapper.
2. Save the report to `benchmarks/performance/malt/results/`.
3. If a viewer/render step is available, document the manual command to launch it - don't
   script a GUI launch inside an automated pipeline.

**Verify:** the report's top allocation call-stack resolves to a named function in this
codebase, not `[unknown]` - that confirms symbols are being picked up correctly and the
numbers are trustworthy.

---

## Part 4 - Bringing it together

### 4.1 What you should have on disk by this point

```text
benchmarks/performance/
  perf/
    profile_perf.sh      # Part 1: perf stat (grouped+repeated), Top-Down, perf record+flamegraph
    results/
  maqao/
    profile_maqao.sh     # Part 2: CQA, Lprof, ONE-View
    results/
  malt/
    profile_malt.sh       # Part 3: malt wrapper run + report
    results/
```

Each script independently runnable, independently useful, each documented well enough (start
each with a short header comment, like `perf/profile_perf.sh` likely already has) that you can
come back in three months and remember what it does without re-reading this guide.

### 4.2 Reading all three together on one hot loop

Once all three scripts run against the same representative workload, you can build one
sentence per loop that no single tool could give you alone, e.g.:

> "The Verlet integration loop (perf flamegraph: 62% self-time) has IPC 2.1 and low cache-miss
> rates (perf stat), is fully vectorised at VER 0.95, using 95% of peak FLOP/cycle - near the
> compute roof (MAQAO CQA) - achieving 40% of peak FLOP/s (MAQAO Lprof), so it's compute-bound
> and already well-optimised. Meanwhile MALT shows 200 allocations/step from the contact list
> `std::vector` reallocating - that's the next thing worth fixing, and none of the other two
> tools would have surfaced it."

If you can't write a sentence like that - if two tools are telling you overlapping things and
a third is telling you nothing new - that's a sign you haven't picked a workload where all
three are actually exercised (e.g. too few objects means the contact/collision path barely
runs), not that a tool is redundant.

### 4.3 Optional: one command to run everything

Only worth building once the three scripts individually work and you're tired of running them
one at a time - a thin orchestrator that calls all three in sequence and reports which ones
actually completed (a missing MAQAO/MALT install shouldn't stop perf from running). Not a
prerequisite for the pipeline being "done" and useful; each script standing alone is already
a complete, valuable tool. Already built: `scripts/profile.sh`.

### 4.4 Closing the loop

Once you've run all three tools for real and read their output (not just "the script exits 0"
- you should be able to answer the checkpoint questions above using *your* project's actual
numbers), check off the `malt`/`maqao` boxes in `docs/ROADMAP.md` Phase 6.

---

## Definition of done

- [ ] You can explain, from memory, what each of perf/MAQAO/MALT measures and *why the other
      two can't measure the same thing* (§0.1/§2.6).
- [ ] `perf stat` output on your machine shows no multiplexing-scaled counters for the events
      you actually care about (§1.4).
- [ ] You've run a Top-Down pass and can name which of the four buckets dominates your hot
      loop, and why (§1.5).
- [ ] MAQAO CQA reports ~0% vectorisation on the scalar build and a materially higher
      vectorisation ratio + VER on the optimised build, for the same loop (§2.3).
- [ ] You can place your hottest loop on a roofline plot and state whether it's compute- or
      memory-bound (§2.5).
- [ ] MALT attributes the majority of allocations to named call-stacks, and you know whether
      any of them are inside the per-step simulation loop (§3.4).
- [ ] All three scripts exist, run independently, and write to their own `results/`
      subdirectory (§4.1).
