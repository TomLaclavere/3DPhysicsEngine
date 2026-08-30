# MALT - tool-specific reference

Quick reference for `benchmarks/performance/malt/profile_malt.sh`. Read
[`BENCHMARK_TOOLS_GUIDE.md`](../BENCHMARK_TOOLS_GUIDE.md) Part 3 first if you haven't - it
explains *why* MALT exists (exact allocation interception vs perf's sampling) and how to read
its metrics; this page is the command/output reference once you already know that.

**Status:** verified — run end to end on this machine (MALT 1.6.3, from-source install to
`/usr/local`), real output in the worked example below.

## Install (from source — no package on Ubuntu/Debian or AUR)

Confirmed: no AUR package exists (`yay -Ss malt`/`memtt` turn up nothing relevant — the only
AUR `malt` is an unrelated freelance-platform desktop app). Build dependencies for both
distros verified against MALT's own docs / this machine's package repos:

```bash
# Arch (this machine) — everything below is in core/extra, no AUR needed:
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

**macOS: not supported.** MALT's dependencies page only documents Debian/Ubuntu, Fedora/
Rocky, Arch, and Gentoo — no macOS/Homebrew instructions exist. It relies on `libelf` and
`libunwind` for symbol resolution, both built around the ELF binary format macOS doesn't use
(Mach-O), so a from-source build wouldn't be a small config tweak even if attempted.

This installs a `malt` command (and `malt-webview`) onto `PATH` — no manual `LD_PRELOAD`
needed, `malt` sets that up internally. Confirm the install worked:

```bash
malt --version
```

**Gotcha hit on this machine:** `malt` itself worked immediately, but `malt-webview` failed
with `error while loading shared libraries: libmalt-reader.so.1: cannot open shared object
file` — installing to `/usr/local` doesn't automatically get picked up by the dynamic linker
until its cache is refreshed:

```bash
sudo ldconfig
# or, without root, just for one invocation:
LD_LIBRARY_PATH=/usr/local/lib malt-webview ...
```

## Quick reference (once installed)

| Goal | Command |
|------|---------|
| Run | `./benchmarks/performance/malt/profile_malt.sh` |
| Reuse existing binary | `... --skip-build` |
| Different representative workload | `... --solver RK4 --n-objects 20` |

Only the **optimised** profile is run by default — see the guide §3.5 for why (allocation
behaviour doesn't change between build profiles, only the arithmetic around it does).

## What the script produces

```text
benchmarks/performance/malt/results/
  malt_optimised/
    malt-Performance-<pid>.json   MALT's report, default naming
```

```bash
malt-webview benchmarks/performance/malt/results/malt_optimised/malt-Performance-*.json
```

## Reading the report

Per the guide §3.4:

| Field | What to check |
|---|---|
| Peak / live memory over time | Does the peak happen during one-time scene construction, or does memory keep climbing during the simulation loop itself (the latter would be a real problem)? |
| Allocation count + call-stack | Which function allocates, how many times. Much more than the object count for a fixed-size scene is suspicious - something inside the per-step loop is allocating. |
| Allocation size histogram | Repeated same-size allocations are a pool/arena-allocator candidate - relevant once the SoA/data-layout roadmap item touches object storage. |
| Realloc count | A growing container (e.g. a contact list) reallocating every few steps shows up here independent of raw allocation count. |
| Leak count | Free sanity check, not a performance metric. |

Cross-check the peak-memory number against `peak_rss_kb` in
[`benchmarks/performance/README.md`](../README.md#memory) for the same configuration - they
measure related but different things (RSS vs tracked heap allocations); a large mismatch
would point at non-heap memory use MALT isn't the right tool to explain.

## Worked example (this machine, default workload, optimised profile)

Real numbers, computed from the report's `stacks.stats` array (`--solver Verlet --dt 5e-5
--dur 30 --n_objects 1`, 600 000 integration steps):

| Metric | Value |
|---|---|
| Total allocations | 3,600,076 |
| Total frees | 3,600,068 |
| Net (leaked) | 8 — matches the report's own `leaks` count exactly |
| Unique call-stacks that allocate | 62 |
| Top allocation sites | Two sites × 1,200,000 allocations of 48 bytes each; two more × 600,000 allocations of 48 bytes each |

**This is a real, actionable finding**: with `--n_objects 1` you'd hope for zero *per-step*
heap allocation in a steady-state simulation loop — instead there are sites firing exactly
2x and 1x per step (1,200,000 / 600,000 steps = 2 and 1) at a fixed 48-byte size, all through
600,000 steps. That fixed size and the exact per-step multiplier is the "candidate for a
pool/arena allocator" pattern the guide's §3.4 describes, not a guess — next step is
resolving which call site that is via `malt-webview` (this raw-JSON extraction doesn't
resolve symbols; the webview's JS-side symbolication does) and checking whether it's fixable
before or alongside the Phase 6 SoA work.

Cross-checked against `peak_rss_kb` from the same run in `benchmark.csv`: MALT's tracked
peak physical memory was 16,065 kB, `peak_rss_kb` (via `getrusage`) was 15,692 kB — 2.4%
apart, close enough to confirm both are measuring essentially the same thing at this scale.
Worth re-running at `--n-objects 100`+ to see whether that gap grows (it would, if
significant non-heap memory — stack, mapped files — started to dominate; MALT wouldn't be
the right tool to explain that).
