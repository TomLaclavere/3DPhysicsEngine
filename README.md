# 3D Physics Engine in C++ for HPC

## Project Overview
A modular 3D physics engine built in modern C++, designed to combine scientific rigor with extreme performance. Inspired by the architecture and best practices of Project Chrono reactphysics3d, this engine aims to strengthen C++ expertise and HPC techniques by delivering a complete, extensible, and highly optimized simulation core.

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

### CMake Parameters

CMake takes multiple parameters to define different compilations :
- `-DUSE_CLANG` : uses clang++ to compile. By default, CMake uses g++.
- `-DUSE_DOUBLE_PRECISION` : uses double precision. By default, use simple float precision.
- `-DBUILD_TESTS=ON` : compiles the unit testing repository. To run the test, you can use : `cd build && ctest -j$(nproc)`.
- `-DCOVERAGE` : allows to perform coverage analysis. Two options are available : `make coverage` which will save the coverage report as HTML file at the following location : `build/coverage_report.html`; and `make coverage-console` which will show the report in the shell where the command is ran.

## Tech Stack

- Language: C++23

- Build System: CMake

- Math Library: 

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
