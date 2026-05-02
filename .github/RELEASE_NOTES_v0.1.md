# v0.1 — Initial Release

This is the first public release of **3DPhysicsEngine**, a modular rigid-body physics engine written in C++23, built from scratch with a focus on numerical accuracy and HPC readiness.

---

## What's included

**Custom math library**
- `Vector3D`, `Matrix3D`, `Quaternion` — no external dependencies, implemented over `std::array` with `constexpr` helpers
- Compile-time precision switch: build with `float` (default) or `double` via `-D3DPE_USE_DOUBLE_PRECISION`

**Numerical integrators**
- Semi-implicit Euler — O(dt), symplectic, fast
- Störmer-Verlet — O(dt²), near-perfect energy conservation in free dynamics
- Runge-Kutta 4 — O(dt⁴), fourth-order convergence verified

**Collision pipeline**
- Broad phase: AABB-based early rejection
- Narrow phase: analytical contact detection for Sphere–Sphere, Sphere–Plane, and AABB–AABB
- Two response modes selectable at runtime:
  - **Impulse-based** — instantaneous velocity correction with restitution and Coulomb friction
  - **Contact forces** — continuous spring-damper model (Hooke + viscous damping + Coulomb friction)

**Material system**
- Per-object properties: density, stiffness, damping ratio, restitution, friction
- YAML-defined materials (steel, aluminium, rubber, wood) loaded at runtime

**Configuration**
- YAML config file + command-line argument overrides + interactive runtime CLI
- Solver and collision mode selectable without recompiling

**Testing**
- 5 GoogleTest targets, ~3,700 lines covering math, collision, world, objects, and utilities
- Energy monitoring (kinetic + potential) at every timestep

**Benchmarks**
- 4 scenarios: free fall, bouncing ball (impulse), bouncing ball (elastic), contact forces
- 150+ (solver × timestep) configurations per scenario
- Metrics: contact-time error, energy drift, CPU time
- Jupyter notebooks + PDF reports in [`benchmarks/`](../benchmarks/)

**Examples**
- 5 standalone simulations: free fall, projectile motion, bouncing ball (3 materials), rolling

---

## Key benchmark insights

| Scenario | Best solver | Takeaway |
|---|---|---|
| Free fall | Verlet / RK4 | Both conserve energy at machine precision; Euler injects energy linearly with dt |
| Bouncing (e = 0.9) | Any | ~68% energy drift is physical (6–7 bounces at e = 0.9), not numerical |
| Bouncing (e = 1.0) | RK4 | Recovers O(dt²) — integrates the quadratic trajectory exactly |
| Contact forces | RK4 | 10–100× more accurate than Euler; tolerates 41% larger timesteps at a given stiffness |

---

## Quick start

```bash
git clone git@github.com:TomLaclavere/3DPhysicsEngine.git
cd 3DPhysicsEngine
mkdir build && cd build
cmake .. -D3DPE_BUILD_EXAMPLES=ON -D3DPE_USE_DOUBLE_PRECISION=ON
make -j$(nproc)
./bin/Bouncing --solver=rk4 --dt=0.001
```

Full build options and developer scripts are documented in the [README](../README.md).

---

## Roadmap

Planned for future releases:
- CPU parallelisation with **OpenMP**
- GPU acceleration with **CUDA / SYCL**
- Distributed simulation with **MPI**
- Broadened narrow-phase geometry support
- Extended benchmark coverage

---

## Requirements

- C++23 compiler (GCC or Clang)
- CMake ≥ 3.22
- yaml-cpp
- GoogleTest (fetched automatically via CMake FetchContent)

---

**License:** AGPL-3.0 — commercial licenses available, contact [tomlaclavere@gmail.com](mailto:tomlaclavere@gmail.com).

**Documentation:** [tomlaclavere.github.io/3DPhysicsEngine](https://tomlaclavere.github.io/3DPhysicsEngine/)
