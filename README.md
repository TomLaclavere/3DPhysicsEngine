# 3D Physics Engine in C++ for HPC

[![C++](https://img.shields.io/badge/C++-23-blue?style=flat&logo=c%2B%2B)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.22.1-064f8d?style=flat&logo=cmake)](https://cmake.org/)
[![GitHub Actions](https://img.shields.io/github/actions/workflow/status/tomlaclavere/3DPhysicsEngine/ci.yml?branch=main&logo=githubactions&style=flat)](https://github.com/tomlaclavere/3DPhysicsEngine/actions)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue?logo=readthedocs&style=flat)](https://tomlaclavere.github.io/3DPhysicsEngine/)
[![License](https://img.shields.io/badge/license-AGPL%203.0-green?style=flat)](LICENSE)
[![GitHub Repo Size](https://img.shields.io/github/repo-size/tomlaclavere/3DPhysicsEngine?style=flat)](https://github.com/tomlaclavere/3DPhysicsEngine)
[![Last Commit](https://img.shields.io/github/last-commit/tomlaclavere/3DPhysicsEngine?style=flat)](https://github.com/tomlaclavere/3DPhysicsEngine/commits/main)

<p align="center">
  <img src="docs/logo/Logo_wo_Background_White.png" width="250"/>
</p>

<h1 align="center">3DPhysicsEngine</h1>
<p align="center">
  High-performance 3D physics simulation engine
</p>

A rigid-body physics engine written from scratch in C++23, focused on numerical accuracy and HPC
scalability: custom 3D math, three integrators, a two-phase collision pipeline, two collision
response models, and benchmarks of both accuracy and performance. Roadmap toward OpenMP,
CUDA/SYCL and MPI: [`docs/ROADMAP.md`](docs/ROADMAP.md).

Contributions are welcome: read [CONTRIBUTING.md](CONTRIBUTING.md) and [CLA.md](CLA.md) first.

## Features

- **Integrators:** semi-implicit Euler (order 1), Velocity Verlet (order 2), RK4 (order 4).
- **Collision pipeline:**
  - broad phase: brute force, uniform grid or sweep & prune;
  - narrow phase: analytical contacts between spheres, AABBs and planes.
- **Collision response,** selectable at runtime:
  - **impulses:** velocity change from the restitution coefficient, plus position correction;
  - **contact forces:** spring-damper (Hooke + viscous damping) with Coulomb friction.
- **Materials:** per-object stiffness, damping, friction, restitution; presets in
  [`data/materials/`](data/materials/) (steel, aluminium, rubber, wood).
- **Configuration:** YAML file, overridden by command-line flags or the interactive CLI.
- **Precision:** `double` by default, `float` with `-D3DPE_USE_DOUBLE_PRECISION=OFF`.
- **Output:** per-object CSV (position, velocity, acceleration over time), replayable in the 3D viewer.

## Layout

```text
lib/, src/       engine headers and sources: mathematics, objects, collision, world, utilities, gui, viewer
examples/        6 standalone simulations (free fall, projectiles, bouncing ×2, rolling, random balls)
benchmarks/      physics/ (solver accuracy) and performance/ (timing, profiling)
tests/           GoogleTest suites (5 targets)
data/materials/  YAML material presets
scripts/         build, test, coverage, benchmark and profiling scripts
```

## Quick start

Dependencies (Ubuntu / Arch):

```bash
sudo apt install -y build-essential cmake git libyaml-cpp-dev   # Ubuntu
sudo pacman -S --needed base-devel cmake git yaml-cpp            # Arch
```

Optional: `qt6-base-dev` / `qt6-base` for the GUI, `gcovr` for coverage, `doxygen graphviz`
for the documentation, `clang ninja` as an alternative toolchain. The 3D viewer downloads
raylib at configure time and needs the OpenGL/X11 development packages (on Ubuntu:
`libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`).

Build, test, run an example:

```bash
git clone git@github.com:TomLaclavere/3DPhysicsEngine.git
cd 3DPhysicsEngine
cmake -S . -B build
cmake --build build -j
ctest --test-dir build -j --output-on-failure

# Run from the repository root: examples read their config.yml from there
./build/examples/Bouncing
./build/examples/Bouncing --solver RK4 --timestep 0.001 --duration 5 --save true
```

Command-line flags: `--gravity`, `--timestep`, `--duration`, `--iters`, `--solver`
(`Euler`, `Verlet`, `RK4`), `--broad` (`Brut`, `Grid`, `SweepPrune`), `--verbose`, `--save`.

### CMake options

| Option | Description | Default |
|---|---|---|
| `3DPE_USE_DOUBLE_PRECISION` | `double` instead of `float` | ON |
| `3DPE_BUILD_EXAMPLES` | Example simulations (`build/examples/`) | ON |
| `3DPE_BUILD_BENCHMARKS` | Benchmark executables (`build/benchmarks/`) | ON |
| `3DPE_BUILD_TESTS` | GoogleTest suites, run with `ctest` | ON |
| `3DPE_BUILD_VIEWER` | raylib 3D viewer (downloads raylib) | ON |
| `3DPE_ENABLE_COVERAGE` | `coverage` and `coverage-console` targets | ON |
| `3DPE_USE_CLANG` | Clang instead of GCC | OFF |
| `3DPE_ENABLE_CLANG_TIDY` | Static analysis during the build | OFF |
| `3DPE_WARNINGS_AS_ERRORS` | Treat warnings as errors | ON |

### Scripts

[`scripts/`](scripts/) wraps the common tasks (`--help` on each, full reference in
[`scripts/USAGE.md`](scripts/USAGE.md)):

| Script | Does |
|---|---|
| `build.sh`, `run.sh`, `tests.sh`, `clean.sh` | Configure/build, run, test, clean |
| `coverage.sh` | Coverage report (HTML or console) |
| `benchmark.sh` | Performance sweeps and plots |
| `profile.sh` | perf + MAQAO + MALT profiling |
| `benchmark_and_profile.sh` | `benchmark.sh` then `profile.sh` |
| `physics_benchmark.sh` | Physics validation benchmarks and their notebook reports |

## Tools

### Graphical interface

`./build/PhysicsEngineGui` (built when Qt6 is found, run from the repository root) exposes the
interactive commands in a window: simulation control, configuration, adding and editing objects,
and three Plotly plots (trajectories, 3D scene, animation) built from the CSV output.

### 3D viewer

`./build/3DPhysicsViewer <csv_dir>` replays a simulation's CSV files to scale.
`cmake --build build --target Viewer` opens `output/CSV` (change it with
`-D3DPE_VIEWER_CSV_DIR=<dir>`).

| Control | Action |
|---|---|
| Space / R | Play-pause / restart |
| Up / Down | Double / halve the speed |
| Left / Right (hold), bottom bar | Seek |
| Left drag / right drag / wheel | Orbit / pan / zoom |

### Documentation

`doxygen Doxyfile` generates `docs/html/`. It is also published on every push to `main`:
[tomlaclavere.github.io/3DPhysicsEngine](https://tomlaclavere.github.io/3DPhysicsEngine/).

## Benchmarks

### Solver accuracy ([`benchmarks/physics/`](benchmarks/physics/))

`./scripts/physics_benchmark.sh` runs each scenario, then executes its Jupyter notebook, which
writes the figures and `results/benchmark_report.pdf` next to it.

| Scenario | What it tests | Report |
|---|---|---|
| [Free fall](benchmarks/physics/Free_Fall/benchmark_analysis.ipynb) | Integrator accuracy without collisions: contact time and energy drift vs dt | [PDF](benchmarks/physics/Free_Fall/results/benchmark_report.pdf) |
| [Bouncing](benchmarks/physics/Bouncing/benchmark_analysis.ipynb) | Impulse contacts with energy loss (e < 1): peak heights, bounce count, free-flight drift | [PDF](benchmarks/physics/Bouncing/results/benchmark_report.pdf) |
| [Bouncing conservative](benchmarks/physics/Bouncing_Conservative/benchmark_analysis.ipynb) | Same with e = 1: any energy change is numerical | [PDF](benchmarks/physics/Bouncing_Conservative/results/benchmark_report.pdf) |
| [Contact forces](benchmarks/physics/Contact_Forces/benchmark_analysis.ipynb) | Spring-damper contacts: convergence order, stability in the (k, dt) plane | [PDF](benchmarks/physics/Contact_Forces/results/benchmark_report.pdf) |

Each notebook explains how its figures are made and what theory predicts for them.

### Performance ([`benchmarks/performance/`](benchmarks/performance/))

`./scripts/benchmark.sh` measures the cost of a step against the number of objects (comparing
broad phases) and against dt (comparing solvers), in a scalar and an optimised build.
`./scripts/profile.sh` explains the results with perf, MAQAO and MALT. Guide:
[`BENCHMARK_TOOLS_GUIDE.md`](benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md); binary reference
and profiling tools installation: [`README.md`](benchmarks/performance/README.md).

## License

Copyright (C) 2025 Tom Laclavère. Licensed under the [GNU AGPL v3.0](LICENSE): any modified
version distributed or offered over a network must publish its source under the same license.
Commercial licenses for closed-source use are available on request.
