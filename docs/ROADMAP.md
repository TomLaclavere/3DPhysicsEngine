# 3D Physics Engine - Roadmap

> **Goal:** Build a credible technical showcase for numerical simulation and HPC roles.
> Phases are ordered by : scientific rigour first, then CPU and GPU performance, then physics completeness, then distributed computing.

---

## Phase 1: Setup and General Architecture ✅

- [x] Initialize GitHub repository
- [x] Define project structure
- [x] Build all the project repositories (inspired by Project Chrono)
- [x] Configure CMake

---

## Phase 2: Setup & Maths Foundations ✅

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

## Phase 3: Rigid Body Dynamics & Physics World ✅

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

## Phase 4: Collisions *(validation pending)*

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
- [ ] **Validation & Release:**
  - [x] **Standard testing:** reference scenes, physical validity
  - [x] **Check energy conservation**
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
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
- [ ] **Validation & Release:**
  - [ ] Publish key results' table in README
  - [x] **Standard testing:** reference scenes updated with energy checks
  - [x] **Check energy conservation** (close Phase 4 validation item)
  - [x] **Documentation:**
    - [x] *Doxygen & GitHub Pages*
    - [x] *Benchmark report update*
    - [x] *GitHub notes*

---

## Phase 6: CPU Performance Engineering

- [ ] **Profiling:**
  - [ ] Identify hotspot with `perf` or Intel VTune
  - [ ] **Roofline model analysis:**
    - [ ] Measure arithmetic intensity (FLOP / byte) of main loops
    - [ ] Determine if code is compute-bound or memory-bandwidth-bound
    - [ ] Plot on Roofline chart
- [ ] **SIMD Vectorisation:**
  - [ ] Vectorise `Vector3D` inner operations (AVX2 / SSE)
  - [ ] Measure FLOP/s improvement vs scalar baseline
  - [ ] Evaluate auto-vectorisation via compiler reports (`-fopt-info-vec`)
  - [ ] Manual intrinsics if auto-vectorisation is insufficient
- [ ] **OpenMP Parallelisation:**
  - [ ] Parallelise force computation loop over objects
  - [ ] Parallelise broad-phase pair checks
  - [ ] Handle thread safety for shared state (contact list, Config)
  - [ ] **Strong scaling study:** fix total N, measure speedup vs thread count (1 → max cores)
  - [ ] Amdahl's law analysis: estimate parallel fraction
- [ ] **Memory Optimisation:**
  - [ ] Profile cache miss rate (`perf stat -e cache-misses`)
  - [ ] Evaluate Structure-of-Arrays (SoA) vs Array-of-Structures (AoS) for object data
  - [ ] Optimise memory layout if SoA improves cache efficiency
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
  - [ ] **Profiling:** NSight Compute / rocprof — thread occupancy, memory bandwidth
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
  - [ ] Choice of graphics layer: GLFW + OpenGL, Vulkan, or VTK / ParaView
  - [ ] **Basic geometry:** spheres, boxes, planes
  - [ ] **Basic shaders**
  - [ ] **Physics–rendering synchronisation:** decouple simulation FPS from rendering FPS
  - [ ] **Camera control:** WASD + mouse
  - [ ] **Graphical interface:**
    - [ ] Add / modify objects at runtime
    - [ ] Modify simulation parameters at runtime
  - [ ] **Debug tools:**
    - [ ] Overlay: FPS, object state, contact normals
    - [ ] Runtime parameter tweaking
    - [ ] Test scenes: towers, balls, pendulums — visual regression tests
- [ ] **Validation & Release:**
  - [ ] **Standard testing:** reference scenes, physical validity
  - [ ] **Documentation:**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

---

## Dropped / Deprioritised

**Fortran portage** — relevant only for legacy HPC centres (Météo-France, aerospace CFD, weather modelling). Low ROI for general numerical simulation roles. Can be reconsidered if a specific target requires it.
