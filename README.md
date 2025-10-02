# 3D Physics Engine in C++ for HPC

[![CMake](https://img.shields.io/badge/CMake-3.22.1-064f8d?logo=cmake)](https://cmake.org/)
[![GitHub Actions](https://img.shields.io/github/actions/workflow/status/tomlaclavere/3DPhysicsEngine/ci.yml?branch=main&logo=githubactions)](https://github.com/tomlaclavere/3DPhysicsEngine/actions)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/deef48eb6ab64d118def430438d08f0c)](https://app.codacy.com/gh/TomLaclavere/3DPhysicsEngine/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/deef48eb6ab64d118def430438d08f0c)](https://app.codacy.com/gh/TomLaclavere/3DPhysicsEngine/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue?logo=readthedocs)](https://tomlaclavere.github.io/3DPhysicsEngine/)
[![Licence](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![C++](https://img.shields.io/badge/C++-23-blue?logo=c%2B%2B)](https://isocpp.org/)

## Project Overview
A modular 3D physics engine built in modern C++, designed to combine scientific rigour with extreme performance. Inspired by the architecture and best practices of Project Chrono reactphysics3d, this engine aims to strengthen C++ expertise and HPC techniques by delivering a complete, extensible, and highly optimized simulation core.

## Goals

1. Basic Physical Simulation

   - Implement rigid bodies subject to gravity and external forces.

   - Experiment with multiple time integrators (Euler, Verlet, semi-implicit).

2. Real-Time 3D Rendering

    - Integrate OpenGL (using GLFW + GLAD) to visualize the scene.

    - Provide a controllable camera system and debug interface (ImGui).

3. Collision Detection & Constraints

    - Broad-phase detection (spatial grid or tree) and narrow-phase (spheres, boxes, planes).

    - Impulse-based resolution with restitution and friction, plus rigid constraints (joints, springs).

4. HPC Optimizations

    - CPU Multithreading: std::thread / OpenMP for parallelizing the simulation.

    - Intel TBB: to compare efficiency and load balancing.

    - MPI: domain decomposition and halo exchange for distributed simulation.

    - GPU Acceleration: CUDA/OpenCL offloading of critical phases (collision detection, constraint solver).

5. Benchmarking & Performance Analysis

    - Stress-test scenarios (hundreds to thousands of bodies, stacked piles, complex systems).

    - Automated benchmark scripts for each implementation (sequential, OpenMP, TBB, MPI, GPU).

    - Graphical reports and comparative analysis in the documentation.

## How to run ?

```
mkdir build && cd build
cmake .. [-OPTIONS]
make -j$(nproc)
```

CMake takes multiple parameters to define different compilations :
- `-D3DPE_USE_CLANG` : uses clang++ to compile. By default, CMake uses g++.
- `-D3DPE_USE_DOUBLE_PRECISION` : uses double precision. By default, use simple float precision.
- `-D3DPE_BUILD_TESTS` : compiles the unit testing repository. To run the test, you can use : `ctest -j$(nproc)`.
- `-D3DPE_ENABLE_COVERAGE` : allows performing coverage analysis. Two options are available, to run after build : `make coverage` which will save a detailed coverage reports as HTML file at the following location : `build/coverage_report/`; and `make coverage-console` to print the results in the console. Be careful: `gcovr` needs to be installed for GCC compilation.

## Developer scripts

For convenience, bash scripts can be used to run common tasks : build, run, tests, coverage and clean. These executable files are provided in `scripts/` repository folder :

```
scripts/
├─ build.sh        # configure & build with CMake
├─ run_app.sh      # build (if needed) and run the executable
├─ run_tests.sh    # build (if needed) and run tests via CTest
├─ coverage.sh     # build+generate coverage (html or console)
└─ clean.sh        # remove the build directory
```

These scripts are designed to be simple and CI-friendly. They accept arguments and environment variables to adapt to different workflows.

### Make sure scripts are executable
The executable files should be committed to Git. If it is not yet set, run locally : 

```bash
chmod +x scripts/*.sh
```
If someone clones the repo on a system that does not preserve the executable bit (Windows), they can still run scripts with `bash scripts/build.sh`.

### How to use ?

A detailed explanation can be found in `scripts/USAGE.md`, to explain the different options, parameters and options to use these bash scripts.

## Doxygen Documentation

To generate the project documentation using Doxygen, run the following command :
```bash
doxygen Doxyfile
```

It will generate two repositories in `docs/` : `html/` and `latex/`. To visualise the documentation, use the command : 
```bash
xdg-open docs/html/index.html      # Linux
open docs/html/index.html          # macOS
start docs/html/index.html         # Windows
```

The documentation is generated at each commit on the main branch, and accessible at : [![Documentation](https://img.shields.io/badge/docs-online-brightgreen)](https://tomlaclavere.github.io/3DPhysicsEngine/)


## Tech Stack

- Language: C++23

- Build System: CMake

- Maths Library: 

- Rendering: 

- Testing & CI: GoogleTest + GitHub Actions

- Profiling: 

- Parallelism Frameworks: 

## Deliverables

- Modular, well-commented source code

- User guide and API documentation (Doxygen)

- Detailed comparative benchmarks

- Example simulations with visual output

- Roadmap and contribution guide for the open-source community
