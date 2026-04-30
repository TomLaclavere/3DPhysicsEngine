# 3D Physics Engine in C++ for HPC

[![C++](https://img.shields.io/badge/C++-23-blue?style=flat&logo=c%2B%2B)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.22.1-064f8d?style=flat&logo=cmake)](https://cmake.org/)
[![GitHub Actions](https://img.shields.io/github/actions/workflow/status/tomlaclavere/3DPhysicsEngine/ci.yml?branch=main&logo=githubactions&style=flat)](https://github.com/tomlaclavere/3DPhysicsEngine/actions)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue?logo=readthedocs&style=flat)](https://tomlaclavere.github.io/3DPhysicsEngine/)
[![License](https://img.shields.io/badge/license-AGPL%203.0-green?style=flat)](LICENSE)
[![GitHub Repo Size](https://img.shields.io/github/repo-size/tomlaclavere/3DPhysicsEngine?style=flat)](https://github.com/tomlaclavere/3DPhysicsEngine)
[![Last Commit](https://img.shields.io/github/last-commit/tomlaclavere/3DPhysicsEngine?style=flat)](https://github.com/tomlaclavere/3DPhysicsEngine/commits/main)

## Project Overview

A modular rigid-body physics engine written in modern C++23, built from scratch with a focus on numerical accuracy and HPC scalability.

The project covers the full simulation stack: custom 3D math library, configurable numerical integrators, a two-phase collision pipeline, two collision response models, and a systematic benchmark suite comparing solvers across accuracy and performance metrics. It is designed as a technical showcase for numerical simulation and high-performance computing, with a roadmap toward parallelisation (OpenMP, CUDA/SYCL, MPI).

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) and [CLA.md](CLA.md) before submitting a pull request.

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Benchmark Results](#benchmark-results)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Build and Run](#build-and-run)
- [Developer Scripts](#developer-scripts)
- [Doxygen Documentation](#doxygen-documentation)
- [Tech Stack](#tech-stack)
- [License](#license)

---

## Features

**Numerical Integrators**
- Semi-implicit Euler — O(dt), symplectic, fast
- Störmer-Verlet — O(dt²), near-perfect energy conservation in free dynamics
- Runge-Kutta 4 — O(dt⁴), highest accuracy, 4th-order convergence verified

**Physics & Forces**
- Gravity with configurable constant
- Two collision response modes, selectable at runtime:
  - **Impulse-based**: instantaneous velocity correction via restitution coefficient + Coulomb friction
  - **Contact forces**: continuous spring-damper model (Hooke restoring force + viscous damping + Coulomb friction)
- Position correction to prevent interpenetration drift

**Collision Pipeline**
- Broad phase: AABB-based early rejection
- Narrow phase: analytical contact computation for Sphere–Sphere, Sphere–Plane, AABB–AABB
- Contact struct: normal, penetration depth, contact point

**Material System**
- Per-object material properties: density, stiffness, damping ratio, restitution, friction
- YAML-defined materials (steel, aluminium, rubber, wood) loaded at runtime

**Precision & Configuration**
- Compile-time precision switch: `float` (default) or `double` via `-D3DPE_USE_DOUBLE_PRECISION`
- YAML config file + command-line overrides + runtime interactive CLI
- Solver and collision mode selectable at runtime

**Validation & Testing**
- 5 GoogleTest targets — ~3,700 lines across mathematics, collision, world, objects, utilities
- Energy monitoring (kinetic + potential) at every timestep
- CSV output for post-processing and comparison with analytical solutions
- 5 example simulations: free fall, projectile motion, bouncing ball (3 materials), rolling

**Benchmarks**
- Systematic solver comparison: 150+ (solver × timestep) configurations per scenario
- Metrics: contact time error, max energy drift, final energy drift, CPU time
- Scenarios: free fall (no collision) and bouncing ball (contact forces mode)
- Jupyter notebooks + PDF reports in [`benchmarks/`](benchmarks/)

---

## Architecture

```
3DPhysicsEngine/
│
├── lib/                        # Public API (headers)
│   ├── mathematics/            # Vector3D, Matrix3D, Quaternion, precision types
│   ├── objects/                # Object (base), Sphere, AABB, Plane, Material
│   ├── collision/              # BroadCollision, NarrowCollision, CollisionResponse, Contact
│   ├── world/                  # PhysicsWorld, Config (singleton), Physics (forces), integrators
│   └── utilities/              # Timer, CSV I/O, command parser
│
├── src/                        # Implementations (mirrors lib/)
├── examples/                   # 5 standalone simulation programs
├── benchmarks/                 # Solver accuracy & performance benchmarks
├── tests/                      # GoogleTest suites (5 targets)
└── data/materials/             # YAML material files
```

**Module dependencies:**

```
PhysicsWorld
  ├── Config          (YAML + CLI + runtime, singleton)
  ├── Object          (Sphere / AABB / Plane + Material)
  ├── Physics         (gravity, contact forces, impulse response)
  ├── Collision       (BroadPhase → NarrowPhase → Response)
  └── Integrators     (Euler / Verlet / RK4)
```

The math layer (`Vector3D`, `Matrix3D`, `Quaternion`) has no external dependencies — all types are implemented from scratch using `std::array` with constexpr helpers and a user-defined `decimal` literal for compile-time precision control.

---

## Benchmark Results

### Free Fall (no collision)

Measures energy conservation and contact time accuracy across solvers.

| Solver      | dt (s)  | Max energy drift      | Contact time error | CPU time |
|-------------|---------|----------------------|--------------------|----------|
| Euler       | 1e-3    | ~0.2%                | ~0.008 s           | ~108 ms  |
| Verlet      | 1e-3    | < 1e-12 (negligible) | < 0.001 s          | ~159 ms  |
| RK4         | 1e-3    | < 1e-12 (negligible) | < 0.001 s          | ~332 ms  |

Verlet and RK4 achieve near-exact energy conservation in free dynamics, consistent with their symplectic/high-order properties. Euler introduces a systematic drift proportional to dt.

### Bouncing Ball (contact forces mode)

Measures solver behaviour under repeated collisions with the spring-damper contact model.

| Solver      | dt (s)  | Max height error | Energy drift | CPU time |
|-------------|---------|-----------------|--------------|----------|
| Euler       | 1e-3    | ~0.008 m        | ~0.73%       | ~1.1 ms  |
| Verlet      | 1e-3    | < 0.001 m       | ~0.73%       | ~1.8 ms  |
| RK4         | 1e-3    | < 0.001 m       | ~0.73%       | ~3.9 ms  |

The ~0.73% energy drift is solver-independent, which isolates the dissipation source to the spring-damper contact model itself rather than the integrator — a physically meaningful result.

Full benchmark data, convergence plots and analysis: [`benchmarks/`](benchmarks/)

---

## Quick Start

### Run an example

```bash
# Clone and build
git clone git@github.com:TomLaclavere/3DPhysicsEngine.git
cd 3DPhysicsEngine
mkdir build && cd build
cmake .. -D3DPE_BUILD_EXAMPLES=ON -D3DPE_USE_DOUBLE_PRECISION=ON
make -j$(nproc)

# Run the bouncing ball example
./bin/Bouncing
```

### Configure a simulation

Each simulation reads a YAML config file at startup. Parameters can also be overridden from the command line:

```bash
./bin/Bouncing --solver=rk4 --dt=0.001 --duration=5.0 --save=true
```

Available solvers: `euler`, `verlet`, `rk4`.

### Run the benchmarks

```bash
cmake .. -D3DPE_BUILD_BENCHMARKS=ON
make -j$(nproc)
./benchmarks/Free_Fall
./benchmarks/Bouncing
# Results written to CSV — open benchmarks/*/benchmark_analysis.ipynb to analyse
```

### Run the tests

```bash
cmake .. -D3DPE_BUILD_TESTS=ON
make -j$(nproc)
ctest -j$(nproc) --output-on-failure
```

---

## Installation

### Get the project

```bash
git clone git@github.com:TomLaclavere/3DPhysicsEngine.git
cd 3DPhysicsEngine
```

### Install dependencies

#### Build system

- CMake >= 3.22
- GCC (included in `build-essential`) or Clang
- (optional) Ninja

#### External libraries

- yaml-cpp

#### Tests

- git (for `FetchContent`, used to install GoogleTest automatically)

#### Coverage tools

- gcovr (for GCC)
- llvm-cov + llvm-profdata (for Clang)

#### Documentation

- Doxygen + Graphviz

#### Installation commands

```bash
# Ubuntu
sudo apt update
sudo apt install -y build-essential cmake git libyaml-cpp-dev gcovr

# Arch Linux
sudo pacman -Syu --needed base-devel cmake git yaml-cpp gcovr
```

For Clang and optional Ninja:

```bash
# Ubuntu
sudo apt install -y clang ninja-build

# Arch Linux
sudo pacman -S --needed clang ninja llvm
```

For documentation:

```bash
# Ubuntu
sudo apt-get install -y doxygen graphviz

# Arch Linux
sudo pacman -S --needed doxygen graphviz
```

---

## Build and Run

```bash
mkdir build && cd build
cmake .. [OPTIONS]
make -j$(nproc)
```

### CMake options

| Option | Description | Default |
|---|---|---|
| `-D3DPE_USE_CLANG` | Use Clang instead of GCC | OFF |
| `-D3DPE_USE_DOUBLE_PRECISION` | Use `double` instead of `float` | OFF |
| `-D3DPE_BUILD_EXAMPLES` | Build the 5 example simulations | OFF |
| `-D3DPE_BUILD_TESTS` | Build GoogleTest suite, run with `ctest` | OFF |
| `-D3DPE_BUILD_BENCHMARKS` | Build solver benchmark executables | OFF |
| `-D3DPE_ENABLE_COVERAGE` | Generate coverage report (`make coverage` or `make coverage-console`) | OFF |
| `-D3DPE_ENABLE_CLANG_TIDY` | Enable static analysis during build | OFF |
| `-D3DPE_WARNINGS_AS_ERRORS` | Treat all warnings as errors | ON |

---

## Developer Scripts

Bash scripts for common tasks are provided in `scripts/`:

```
scripts/
├─ build.sh        # configure & build with CMake
├─ run.sh          # build (if needed) and run the executable
├─ tests.sh        # build (if needed) and run tests via CTest
├─ coverage.sh     # build and generate coverage (HTML or console)
└─ clean.sh        # remove the build directory
```

These scripts are CI-friendly and accept arguments and environment variables. See `scripts/USAGE.md` for the full option reference.

Make sure they are executable:

```bash
chmod +x scripts/*.sh
```

---

## Doxygen Documentation

Generate locally:

```bash
doxygen Doxyfile
xdg-open docs/html/index.html   # Linux
```

The documentation is automatically generated on each commit to `main` and published at:

[![Documentation](https://img.shields.io/badge/docs-online-brightgreen)](https://tomlaclavere.github.io/3DPhysicsEngine/)

---

## Tech Stack

| | |
|---|---|
| Language | C++23 |
| Build | CMake 3.22+ |
| Math | Custom (`Vector3D`, `Matrix3D`, `Quaternion`) — no external dependency |
| Configuration | yaml-cpp |
| Testing | GoogleTest (fetched via CMake FetchContent) |
| CI | GitHub Actions |
| Documentation | Doxygen + GitHub Pages |
| Analysis | Python, Jupyter, Matplotlib (benchmarks) |
| Planned | OpenMP, CUDA/SYCL, MPI |

---

## License

This project is licensed under the GNU Affero General Public License v3.0 (AGPL-3.0).

You are free to use, modify, and redistribute this software under the terms of the AGPL-3.0. Any modified version that is distributed or made available over a network must also make its source code available under the same license.

**Commercial licensing:** Commercial licenses are available for proprietary or closed-source use. Please contact the author to discuss terms.
