# MAQAO — tool-specific reference

Quick reference for `benchmarks/performance/maqao/profile_maqao.sh`. Read
[`BENCHMARK_TOOLS_GUIDE.md`](../BENCHMARK_TOOLS_GUIDE.md) Part 2 first if you haven't — it
explains *why* CQA/Lprof exist and how to interpret each; this page is the command/output
reference once you already know that.

**Status:** verified — run end to end on this machine (MAQAO 2025.1.0), both build profiles,
all three passes (CQA, Lprof, ONE-View). Real output is in the worked example below. Two
things the verification run corrected from an earlier guess:

- **There is no standalone "Roofline" module.** `maqao --list-modules` lists only
  `analyze`/`cqa`/`lprof`/`madras`/`oneview`. The roofline-relevant numbers — peak FLOP/cycle
  and achieved FLOP/cycle per loop — come from CQA's own output (see below); ONE-View
  (`oneview -R1`) is the closest thing to a combined dashboard, not a dedicated roofline
  chart. `profile_maqao.sh`'s third pass is `run_oneview`, not `run_roofline`.
- **Every invocation needs `cwd == repo root`.** `benchmark.cpp` hardcodes its CSV path as
  the relative string `"benchmarks/performance/results"` — MAQAO changing directory (which
  `oneview`/`lprof` do internally around the experiment path) crashes the binary with a
  filesystem error. `profile_maqao.sh` handles this with an explicit `cd "$ROOT"`.

## Install

Two equivalent options — same upstream binary either way, pick whichever fits:

```bash
# Arch (this machine) — AUR package, same binary release, just distro-packaged:
yay -S maqao-bin
maqao --version
```

```bash
# Any distro — generic binary release, no package needed (confirmed working; see
# maqao.org/download.html for the current version, MAQAO switched to year-based
# versioning in May 2025):
curl -LO http://www.maqao.org/maqao_archive/maqao.x86_64.2025.1.0.tar.xz
tar xf maqao.x86_64.2025.1.0.tar.xz
export PATH="$PWD/maqao.x86_64.2025.1.0:$PATH"   # add to your shell rc to persist
maqao --version
```

**macOS: not supported.** MAQAO is built for "64-bit Linux based operating systems" per its
own [download page](https://www.maqao.org/download.html) — no binary release or install path
exists for macOS.

## Quick reference

| Goal | Command |
|------|---------|
| Full pipeline, both profiles | `./benchmarks/performance/maqao/profile_maqao.sh` |
| One profile only | `./benchmarks/performance/maqao/profile_maqao.sh --profile optimised` |
| Just the static loop analysis | `./benchmarks/performance/maqao/profile_maqao.sh --skip-lprof --skip-oneview` |
| Reuse existing binaries | `./benchmarks/performance/maqao/profile_maqao.sh --skip-build` |
| Analyse every function, not just this project's | `./benchmarks/performance/maqao/profile_maqao.sh --cqa-functions ".*"` |

## What the script produces

```text
benchmarks/performance/maqao/results/
  cqa_<profile>/cqa.txt     CQA static loop analysis, text format
  lprof_<profile>/
    functions.txt             Lprof cluster-wide function hotspots
    loops.txt                 Lprof cluster-wide loop hotspots
    run/                       raw collection data (Lprof's own experiment dir)
  oneview_<profile>/
    RESULTS/Performance_one_html/index.html   combined HTML dashboard
```

## Reading a CQA report

Real syntax: `maqao cqa <binary> fct-loops=<pattern>` — `<pattern>` is a comma-separated list
of Lua patterns (not regex; no `|` alternation — commas are how you express OR), matched
against demangled function names. `profile_maqao.sh` defaults to this project's own
namespaces (`PhysicsWorld,NarrowCollision,BroadCollision,Sphere,Plane,Vector3D,simulation`)
so the report isn't buried under libc/gcov noise; use `--cqa-functions ".*"` for an
unscoped first look at a binary you don't know yet.

Per the guide §2.3, the numbers to find for each loop:

| Field | What it means |
|---|---|
| `X% of peak computational performance` | Achieved vs theoretical FLOP/cycle for that loop — this is the roofline Y-axis value, straight from CQA. |
| "Your loop is not vectorized" / "probably not vectorized" | CQA's vectorisation verdict. |
| "Only N% of vector register length is used" | Vector Efficiency Ratio (VER) in prose form — of the vector instructions present, how much of the register width is actually used. |
| "...lower the cost of an iteration from X to Y cycles (Zx speedup)" | CQA's potential-speedup estimate if fully vectorised — use this to prioritise which loop is worth hand-optimising. |
| "Execution units bottlenecks" / "Code clean check" sections | Bottleneck classification — a different fix for a front-end, execution-port, or scalar-address-computation bottleneck. |

## Reading an Lprof report

Two-step tool: collection (`maqao lprof -xp=<path> -- <binary> <args>`, executes the binary)
then display (`maqao lprof -df xp=<path>` for functions, `-dl` for loops — note no leading
dash on `xp=` in *display* mode, unlike collection's `-xp=`, that's MAQAO's own
inconsistency, not a typo here). Confirms what CQA's static analysis only predicted: which
function/loop actually dominates wall time when the binary runs, with full call-chain
attribution back to source lines. Cross-reference against CQA — if the loop CQA said had the
best vectorisation potential isn't the one Lprof says costs the most time, prioritise by
Lprof.

## Reading a ONE-View report

`maqao oneview -R1 -xp=<path> --replace -- <binary> <args>` (binary goes directly after `--`,
*not* via `--executable=` — that flag does something else and silently breaks argument
parsing, see the script's header comment). Open
`oneview_<profile>/RESULTS/Performance_one_html/index.html` in a browser: it's CQA + Lprof +
topology data combined into one dashboard, most useful as a shareable summary once you
already know what CQA/Lprof told you separately.

## Same representative workload as perf

`profile_maqao.sh` uses the same `--solver Verlet --dt 5e-5 --dur 30 --n-objects 1` default
as `perf/profile_perf.sh`, so the two tools' reports describe the same run and are directly
comparable. Override consistently across both scripts if you change it (e.g. to stress
collision detection: `--n-objects 100` on both).

## Worked example (this machine, default workload, `--cqa-functions` default)

Real output from running `profile_maqao.sh` on both profiles:

| Loop (representative) | scalar | optimised |
|---|---|---|
| Vectorised? | No (0% of peak, "not vectorized") | "probably not vectorized" — 23–27% of vector register length used |
| Peak FLOP/cycle used | 0.00 / 48.00 (0%) | 0.27–0.30 / 24.00 (~1%) |
| CQA's speedup-if-fully-vectorised estimate | 4.00x (consistent across loops — expected, `-fno-tree-vectorize` guarantees this) | 3.12–4.00x |

**This is a real, useful finding, not a demonstration artefact**: even the *optimised*
build's own physics code (`PhysicsWorld`/`NarrowCollision`/`Sphere`/`Vector3D`) is barely
vectorised — `-O3 -march=native` alone isn't getting the compiler to auto-vectorise these
loops, likely because of the AoS (array-of-structures) `Vector3D` layout CQA's own
"Code clean check" section flags directly: *"Detected a slowdown caused by scalar integer
instructions... Try to reorganize arrays of structures to structures of arrays."* That's
exactly the Phase 6 roadmap's data-layout item (`docs/ROADMAP.md`) — MAQAO independently
confirms it's the right next step, with a number attached (up to 4x per-loop speedup
estimated) rather than a guess.

Lprof's function-level breakdown for the optimised profile puts `Vector3D::operator[]`,
`std::array<double,3>::operator[]`, and the `applyVector`/`operator+` template chain used by
`PhysicsWorld::integrateVerlet` at the top of self-time — small, heavily-templated accessor
functions, consistent with an AoS `Vector3D` never getting hoisted into wide registers.
