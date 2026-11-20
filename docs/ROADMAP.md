# 3D Physics Engine - Roadmap

> **Goal:** Build a credible technical showcase for numerical simulation and HPC roles.
> Phases are ordered by: scientific rigour first, then CPU and GPU performance, then physics completeness, then distributed computing.

| Phase | Topic | Status |
|---|---|---|
| 1 | Setup and General Architecture | Done |
| 2 | Maths Foundations | Done, except CI with Clang |
| 3 | Rigid Body Dynamics & Physics World | Done |
| 4 | Collisions | Done |
| 5 | Scientific Validation & Benchmark Analysis | Done |
| 6 | CPU Performance Engineering | **In progress** (6.4 Data layout) |
| 7 | GPU Acceleration (CUDA / SYCL) | Planned |
| 8 | Rotations & Angular Dynamics | Planned |
| 9 | Constraint Solver (Sequential Impulse / LCP) | Planned |
| 10 | Distributed Parallelism (MPI) | Planned |
| 11 | Advanced Physics Features *(optional)* | Planned |
| 12 | Visualisation | Started (raylib CSV replay viewer) |

---

## Phase 1: Setup and General Architecture

- [x] Initialize GitHub repository
- [x] Define project structure
- [x] Build all the project repositories (inspired by Project Chrono)
- [x] Configure CMake

---

## Phase 2: Maths Foundations

- [x] **Maths Library:** Implement or integrate 3D maths types: `vec3` for 3D vectors, matrices, and quaternion for rotation.
  - [x] **Vector3D:** Implement basic vector operations (addition, subtraction, scalar multiplication, dot product, cross product).
  - [x] **Vector3D Tests:** Write unit tests to verify vector operations.
  - [x] **Matrix3x3:** Implement matrix operations (multiplication, transpose, inverse).
  - [x] **Matrix3x3 Tests:** Write unit tests to verify matrix operations.
  - [x] **Quaternion:** Implement quaternion operations (multiplication, normalization, inverse).
    - [x] Verify that all quaternion functions are correctly implemented
    - [x] **Euler angle:** avoid code duplication, in particular with Euler angle
  - [x] **Quaternion Tests:** Write unit tests to verify quaternion operations.
- [x] **Object Structs:**
  - [x] **Transform:** handle position and orientation of volumes
  - [x] **AABB:** Axis-Aligned Bounding Box = right block whose faces are parallel to the three axes of the reference frame
  - [x] **Bounding Sphere**
- [ ] **Testing:** Develop Unit Testing
  - [x] **Test Maths**
  - [x] **Test Objects**
  - [x] **Coverage:** Add coverage computing option during compilation.
  - [x] **Test CI:** Integration of tests procedure in GitHub.
    - [x] **CI with GCC**
    - [ ] **CI with Clang**
- [x] **Documentation:** Doxygen
  - [x] Document functions and utilities.
  - [x] Documentation CI.

---

## Phase 3: Rigid Body Dynamics & Physics World

- [x] **Core Setup:**
  - [x] **Physical constants**
    - [x] Define and implement physical constants
    - [x] **Design a way to define all the variables of the simulation:**
      - [x] Parameters file (yaml)
      - [x] Command lines
      - [x] Runtime commands
  - [x] **Time loop**
  - [x] **External forces:** gravity
  - [x] **Object management**
- [x] **Simple Motion:**
  - [x] **Linear Motion Basics**
    - [x] Position updates
    - [x] Velocity updates
    - [x] Basic force application
  - [x] **Basic Integration**
    - [x] **Semi-implicit Euler** (basic)
    - [x] **Time step handling**
    - [x] **Verlet** (optional)
    - [x] **RK4** (optional)
- [x] **Fundamental Test:** Add examples repository to implement testing cases for basic physical situations
  - [x] **Free fall**
  - [x] **Projectile motion**
- [x] **Validation & Release:**
  - [x] **Standard testing:** reference scenes, physical validity, ...
  - [x] **Documentation:**
    - [x] *Doxygen & GitHub Pages*
    - [x] *Installation & Usage guides*
    - [x] *GitHub notes*
    - [x] *Unit testing*

---

## Phase 4: Collisions

- [x] **Collision Detection:**
  - [x] Implement simple collision model
  - [x] **Broad-Phase:** AABB-based early rejection
  - [x] **Narrow Phase:**
    - [x] Sphere–Plane
    - [x] Sphere–Sphere
    - [x] AABB–AABB
- [x] **Collision Response:**
  - [x] **Impulse-based response:** instantaneous velocity correction via restitution coefficient + Coulomb friction
  - [x] **Contact forces mode:** Hooke restoring force + viscous damping + Coulomb friction
  - [x] **Position correction** to prevent interpenetration drift
  - [x] **Runtime switch** between impulse and contact forces modes
- [x] **Validation & Release:**
  - [x] **Standard testing:** reference scenes, physical validity
  - [x] **Check energy conservation**
  - [x] **Documentation:**
    - [x] *Doxygen & GitHub Pages*
    - [x] *Installation & Usage guides*
    - [x] *GitHub notes*

---

## Phase 5: Scientific Validation & Benchmark Analysis

*Close out Phase 4 with rigorous numerical evidence.*

- [x] **Convergence analysis:**
  - [x] Log-log plot of position/energy error vs dt for each integrator
    - [x] Verify slope ≈ 1 for Euler, ≈ 2 for Verlet, ≈ 4 for RK4
  - [x] Add convergence plots to Jupyter notebooks in `benchmarks/`
- [x] **Energy analysis:**
  - [x] Document solver-independent ~0.73% energy drift in contact-forces mode
    - [x] Explain: dissipation is a property of the spring-damper model, not the integrator
  - [x] Confirm near-zero energy drift for Verlet and RK4 in free-dynamics scenario
  - [x] Compare impulse mode vs contact forces mode energy behaviour
- [x] **Benchmark extension:**
  - [x] Add bounce count vs restitution coefficient sweep
  - [x] Automated benchmark runner (script that builds, runs all benchmarks, generates CSV)
- [x] **Validation & Release:**
  - [x] Publish key results' table in README
  - [x] **Standard testing:** reference scenes updated with energy checks
  - [x] **Check energy conservation** (close Phase 4 validation item)
  - [x] **Documentation:**
    - [x] *Doxygen & GitHub Pages*
    - [x] *Benchmark report update*
    - [x] *GitHub notes*

---

## Phase 6: CPU Performance Engineering

See [`docs/benchmark.md`](benchmark.md) for more details.

### 6.1 Method and tools

- [x] **Brainstorm on ROADMAP**
- [x] **Brainstorm on To-Do-List**
- [x] **Tools implementation:** (see `benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md`)
  - [x] Reproducible benchmark simulation script
    - [x] Simulation code (similar than examples)
    - [x] Recompilation with proper flags (compiler optimisation, release build, precision, ...)
    - [x] Benchmark parameters (Number of Simulation, Number of objects, timestep, integrator, ...)
    - [x] Saving tools
  - [x] **Analysis tools wrappers :**
    - [x] perf
    - [x] malt
    - [x] maqao
  - [x] Plotting tools
- [x] **Reference benchmarks:**
  - [x] Scalar reference case
  - [x] Compiler-optimised single-core
- [x] **Add GitHub flag to keep the current status of software as reference**
- [x] **Optimisation workflow:**
  - [x] Performance log (`docs/performance_logs.md`): one line per optimisation - before, after, gain, commit
- [ ] **Bonus:** Create InfPlane object, to simplify creation of the ground

### 6.2 Quick wins and broad phase

- [x] **Quick wins (from the baseline profile):**
  - [x] Remove the per-step heap allocations (`prevAcc` / `nextAcc` in `PhysicsWorld::integrateVerlet`; check Euler and RK4)
  - [x] Let small accessors inline (`Sphere::getRadius`, `getCenter`, `Object::getPosition`): move them to headers or enable LTO (`-flto`)
- [x] **Broad-phase acceleration** *(algorithmic: O(N²) → ~O(N), largest gain at large N)*:
  - [x] Uniform spatial grid or sweep-and-prune instead of testing every pair
  - [x] Extend the benchmark sweep to N = 10³ - 10⁴
### 6.3 Google Benchmark

- [x] **Google Benchmark** *(next step; compared with the in-house benchmark, `scripts/benchmark.sh`)*:
  - [x] Add it with CMake `FetchContent`, behind an option (`3DPE_BUILD_GBENCH`)
  - [x] Benchmark one simulation step on the same random 3D cloud scene, per broad phase, N from 1 to 10⁴: both tools call the same functions (`benchmarks/performance/scenario.hpp`), including the timed one (`runSteps`), same points run by `scripts/benchmark.sh`
  - [x] Compare with `scripts/benchmark.sh` at the same (solver, broad phase, dt, N): step time, run-to-run variation, total run duration; explain the differences (warm-up, number of iterations, timer, `DoNotOptimize`)
  - [x] Decide which tool answers which question, and document it in `benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md`
### 6.4 Data layout

Steps marked *bit-identical* must leave the physics regression test exactly unchanged.

- [ ] **Data layout optimisation** *(own branch `feature/soa-layout`; compare current pointers → contiguous AoS → SoA)*:
  - [ ] **Narrow phase and response on indices:** free functions per shape pair instead of virtual calls *(bit-identical)*
    - [x] Pair tests and narrow phase on shape values (`SphereShape`, `AABBShape`, `PlaneShape`); the `Object` overloads become wrappers, so existing tests are unchanged
    - [x] Remove the redundant `getNormalised()` calls on `PlaneShape` (already unit vectors)
    - [x] Contact response (`computeContactForce`, `reboundCollision`, `positionCorrection`) on values instead of `Object&`
    - [x] Remove `Contact::A` / `Contact::B` (written, never read)
  - [ ] **Contiguous hot data (AoS):** `std::vector<Body>` (position, velocity, acceleration, inverse mass, radius, flags) in `PhysicsWorld`, behind accessors `pos(i)`, `vel(i)`...
    - [ ] `Object` becomes a handle (world + index) with the same getters/setters; `id → index` map for `removeObject`
      - [ ] Handle first on the current storage *(bit-identical)*: `BodyId addObject(const Object&)`, `body(id)`; migrate GUI, CLI, `main.cpp`, examples, benchmarks, tests; remove `getObject()` returning `Object*`
    - [ ] Integrators (Euler, Verlet, RK4) loop over indices
    - [ ] Pair loop in the same order (pairs i < j) *(bit-identical)*; CSV output keyed by id instead of `Object*`
    - [ ] `removeObject` keeps the order of the other bodies; no add or remove during `run()`
    - [ ] Benchmark: pointers vs AoS
  - [ ] **Broad phases** read arrays (positions, radii) instead of `std::vector<Object*>`
  - [ ] **SoA:** same accessors, storage split into one array per field; benchmark AoS vs SoA *(bit-identical)*
  - [ ] **Split x / y / z** into aligned arrays, then check vectorisation (MAQAO, `-fopt-info-vec`) *(bit-identical)*
    - [ ] Preallocated RK4 / Verlet buffers in SoA form (`integrateRK4` still allocates 5 vectors per step)
    - [ ] Loops written with `std::span` + `__restrict`
  - [ ] **Collisions per shape pair** *(results change: validate on the physics benchmarks, not bit for bit)*
    - [ ] Two passes: detection per shape pair (vectorised sphere-sphere test) → contact list → sequential resolution
    - [ ] Contact forces mode: each pair computed once, ±F (Newton's third law)
    - [ ] Fixed bodies stored at the end of the arrays, so integrators loop without a test
  - [ ] **Clean-up:** remove the virtual collision methods, `Object::integrate` and the unused `force` / `torque`

### 6.5 SIMD and OpenMP

- [ ] **SIMD Vectorisation:**
  - [ ] Vectorise `Vector3D` inner operations (AVX2 / SSE)
  - [ ] Evaluate auto-vectorisation via compiler reports (`-fopt-info-vec`)
  - [ ] Manual intrinsics if auto-vectorisation is insufficient (Eve ?)
- [ ] **OpenMP Parallelisation:**
  - [ ] OpenMP / TBB
  - [ ] Add a thread-count dimension to the benchmark (`--threads`, `OMP_NUM_THREADS`, `OMP_PLACES=cores`, `OMP_PROC_BIND=close`): P-cores first, then hyper-threading and E-cores measured separately
  - [ ] Parallelise force computation loop over objects
  - [ ] Parallelise broad-phase pair checks
  - [ ] Handle thread safety for shared state (contact list, Config)
  - [ ] **Strong scaling study:** fix total N, measure speed-up vs thread count (1 → max cores)
  - [ ] Amdahl's law analysis: estimate parallel fraction
### 6.6 Benchmarks and release

- [ ] **Benchmarks:**
  - [ ] Scalability: CPU time vs N objects (N = 10², 10³, 10⁴, 10⁵)
  - [ ] Comparison table: scalar vs SIMD vs OpenMP
  - [ ] External comparison with ReactPhysics3D / Chrono *(optional)*
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** verify physics results unchanged after optimisation
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Performance report (Roofline plot, scaling plots)*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Phase 7: GPU Acceleration (CUDA / SYCL)

- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Software choice:** SYCL (vendor-agnostic, national lab standard) or CUDA
- [ ] **GPU Kernel:**
  - [ ] Broad-phase AABB pair test (embarrassingly parallel, good GPU fit)
  - [ ] Data transfer: host → device (object list), device → host (contact list)
  - [ ] Minimise host–device transfer overhead (pinned memory, async transfers)
- [ ] **Optimisation:**
  - [ ] **Profiling:** NSight Compute / rocprof - thread occupancy, memory bandwidth
  - [ ] **Tuning:** block sizes, shared memory usage, warp divergence
- [ ] **Benchmarks:**
  - [ ] Throughput: GPU vs CPU broad-phase for N = 10³, 10⁴, 10⁵ object pairs
  - [ ] Consistency check: GPU and CPU results match to numerical tolerance
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** reference scenes produce identical results on CPU and GPU paths
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *GPU performance report*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Phase 8: Rotations & Angular Dynamics

- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Extend RigidBody:**
  - [ ] **Orientation** (quaternion integration)
  - [ ] **Angular velocity**
  - [ ] **Inertia tensor** (analytical for sphere and box)
- [ ] **Add torque & angular acceleration:**
  - [ ] Torque application API
  - [ ] Angular integration in Euler, Verlet, RK4
- [ ] **Update collision response** to account for angular impulse at contact point
- [ ] **Tests:**
  - [ ] Rotating cube (constant torque)
  - [ ] Spinning sphere (no torque, verify angular momentum conservation)
  - [ ] Rolling ball (friction-driven rotation)
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** reference scenes, physical validity
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Phase 9: Constraint Solver (Sequential Impulse / LCP)

- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **LCP Solver:**
  - [ ] Implement projected Gauss-Seidel (PGS) for the Linear Complementarity Problem
  - [ ] Baumgarte stabilisation for constraint drift correction
  - [ ] Configurable iteration count and tolerance
- [ ] **Constraint types:**
  - [ ] Non-penetration constraint (replaces / complements current collision response)
  - [ ] Ball-socket joint
  - [ ] Hinge joint
- [ ] **Integration:**
  - [ ] Plug constraint solver into PhysicsWorld time loop
  - [ ] Runtime switch: spring-damper vs constraint solver
- [ ] **Benchmark:**
  - [ ] Stability comparison: spring-damper vs PGS at varying stiffness
  - [ ] Performance comparison: CPU time per timestep
- [ ] **Tests:**
  - [ ] Pendulum (ball-socket)
  - [ ] Articulated chain
  - [ ] Stack stability under gravity
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** reference scenes, physical validity
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Phase 10: Distributed Parallelism (MPI)

- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Domain decomposition:**
  - [ ] Partition simulation box into spatial subdomains (one per MPI rank)
  - [ ] Assign objects to ranks based on position
  - [ ] Ghost layer: exchange boundary objects between neighbouring ranks each timestep
- [ ] **Communication:**
  - [ ] `MPI_Sendrecv` for ghost layer exchange
  - [ ] `MPI_Allreduce` for global energy computation
  - [ ] Minimise synchronisation points
- [ ] **Load balancing:**
  - [ ] Monitor objects-per-rank distribution
  - [ ] Dynamic rebalancing if imbalance exceeds threshold *(optional)*
- [ ] **Scaling studies:**
  - [ ] **Weak scaling:** fix N/rank, increase rank count → measure parallel efficiency
  - [ ] **Strong scaling:** fix total N, increase rank count → measure speedup
  - [ ] Report communication overhead fraction
- [ ] **Validation & Release:**
  - [ ] Single-rank MPI run matches serial result
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Scaling report (weak/strong scaling plots)*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Phase 11: Advanced Physics Features *(optional)*

- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Advanced forces:**
  - [ ] Drag (linear and quadratic)
  - [ ] Soft-body springs between particles
- [ ] **Additional constraints:**
  - [ ] Pivot
  - [ ] Slider
- [ ] **Complex test scenes:**
  - [ ] Bouncing balls with mixed materials
  - [ ] Cannon ball vs box stack
  - [ ] Box tower under gravity
- [ ] **Performance:**
  - [ ] Benchmark & profiling
  - [ ] External comparison with Chrono / ReactPhysics3D *(optional)*
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** reference scenes, physical validity
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Phase 12: Visualisation

- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Output format:**
  - [ ] VTK or HDF5 output (ParaView-compatible) alongside existing CSV
  - [ ] HDF5 for large-scale runs (> 10⁴ objects, long durations)
- [ ] **Optional real-time rendering:**
  - [x] Choice of graphics layer: GLFW + OpenGL, Vulkan, or VTK / ParaView → raylib (CSV replay viewer, `src/viewer/`)
  - [x] **Basic geometry:** spheres, boxes, planes
  - [x] **Basic shaders**
  - [ ] **Physics–rendering synchronisation:** decouple simulation FPS from rendering FPS
  - [ ] **Camera control:** WASD + mouse
  - [ ] **Graphical interface:**
    - [ ] Add / modify objects at runtime
    - [ ] Modify simulation parameters at runtime
  - [ ] **Debug tools:**
    - [ ] Overlay: FPS, object state, contact normals
    - [ ] Runtime parameter tweaking
    - [ ] Test scenes: towers, balls, pendulums - visual regression tests
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** reference scenes, physical validity
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Dropped / Deprioritised

**Fortran portage** - relevant only for legacy HPC centres (Météo-France, aerospace CFD, weather modelling). Low ROI for general numerical simulation roles. Can be reconsidered if a specific target requires it.