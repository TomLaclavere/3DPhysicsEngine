# 3D Physics Engine in C++ for HPC

[![C++](https://img.shields.io/badge/C++-23-blue?logo=c%2B%2B)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.22.1-064f8d?logo=cmake)](https://cmake.org/)
[![GitHub Actions](https://img.shields.io/github/actions/workflow/status/tomlaclavere/3DPhysicsEngine/ci.yml?branch=main&logo=githubactions)](https://github.com/tomlaclavere/3DPhysicsEngine/actions)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue?logo=readthedocs)](https://tomlaclavere.github.io/3DPhysicsEngine/)
[![License](https://img.shields.io/badge/license-AGPL%203.0-green)](LICENSE)

## Project Overview
A modular 3D physics engine written in modern C++, designed to combine scientific rigour with high-performance computing techniques.
The project focuses on numerical accuracy, clean architecture, and scalability, while remaining efficient on modern hardware.

This engine is primarily developed as a technical and scientific showcase,
demonstrating advanced C++ design, HPC-oriented optimizations, and physically sound simulation methods.

While initially a personal research and engineering project, the codebase is designed with long-term maintainability and extensibility in mind.

Contributions are welcome. Please read CONTRIBUTING.md and CLA.md before submitting a pull request.

---

## Table of Contents
 - [How to build and run](#how-to-build-and-run)
 - [Developer scripts](#developer-scripts)
 - [Doxygen Documentation](#doxygen-documentation)
 - [Tech Stack](#tech-stack)
 - [Deliverables](#deliverables)
 - [License](#license)

## How to build and run

```bash
mkdir build && cd build
cmake .. [-OPTIONS]
make -j$(nproc)
```

CMake takes multiple parameters to define different compilations :
- `-D3DPE_USE_CLANG` : uses clang++ to compile. By default, CMake uses g++.
- `-D3DPE_USE_DOUBLE_PRECISION` : uses double precision. By default, use simple float precision.
- `-D3DPE_BUILD_EXAMPLES` : compiles the unit example repository. To run an example, the executables are accessible at : `./build/examples/`.
- `-D3DPE_BUILD_TESTS` : compiles the unit testing repository. To run the test, you can use : `ctest -j$(nproc)`.
- `-D3DPE_ENABLE_COVERAGE` : allows performing coverage analysis. Two options are available, to run after build : `make coverage` which will save a detailed coverage reports as HTML file at the following location : `build/coverage_report/`; and `make coverage-console` to print the results in the console. Be careful: `gcovr` needs to be installed for GCC compilation.
- `-D3DPE_BUILD_BENCHMARKS` : compiles the benchmark repository. To run benchmarks, the executables are accessible at : `./build/benchmarks/` : NOT IMPLEMENTED YET.
- `-D3DPE_ENABLE_CLANG_TIDY` : enables static analysis with clang-tidy during the build. Clang-tidy must be installed on the system. By default, it is enabled.
- `-D3DPE_WARNINGS_AS_ERRORS` : treats all compiler warnings as errors. By default, it is enabled.

## Developer scripts

For convenience, bash scripts can be used to run common tasks : build, run, tests, coverage and clean. These executable files are provided in `scripts/` repository folder :

```
scripts/
├─ build.sh        # configure & build with CMake
├─ run.sh          # build (if needed) and run the executable
├─ tests.sh        # build (if needed) and run tests via CTest
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

- Testing & CI: GoogleTest, GitHub Actions

- Documentation: Doxygen

- Focus areas: numerical methods, performance, parallelism, modular design

## Deliverables

- Modular, well-documented C++ codebase

- Scientifically sound physics models

- High-performance and scalable architecture

- Doxygen-generated API documentation

- Benchmarks and example simulations

- Long-term roadmap

## License
This project is licensed under the GNU Affero General Public License v3.0 (AGPL-3.0).

You are free to use, modify, and redistribute this software under the terms of the AGPL-3.0.
Any modified version that is distributed or made available over a network must also make its source code available under the same license.

Commercial licensing

Commercial licenses are available for proprietary, closed-source, or commercial use.
Please contact the author to discuss licensing terms.
