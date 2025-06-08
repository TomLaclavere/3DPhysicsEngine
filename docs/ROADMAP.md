# 3D Physics Engine - Roadmap

Based on [winter](https://winter.dev/articles/physics-engine#:~:text=struct%20Object%20,vec3%20Force%3B%20float%20Mass%3B) and [Project Chrono](https://github.com/projectchrono/chrono)

---

## Phase 1: Setup and General Architecture

- [x] Initialize GitHub repository
- [ ] Define project structure
- [ ] Build all the project repositories (inspired by Project Chrono)
- [ ] Configure CMake for cross-platform compilation

---

## Phase 2: Setup & Math Foundations  

- [ ] **Project Skeleton :** Create a Git repository and initialize the project (set up a `main.cpp`, `CMakeLists.txt` or your build system).  
- [ ] **Math Library :** Implement or integrate 3D math types: `vec3` for 3D vectors, matrices, and quaternion for rotation.  
- [ ] **Utility Types :** Define data structures like `Transform` and bounding volumes (AABB, sphere).  
- [ ] **Documentation :** Document your math types and utilities.
- [ ] **Test Math :** Write tests or small applications to validate math operations.

---

## Phase 3: Visualization & 3D Rendering

- [ ] **Choose a Rendering Library :** SDL, OpenGL, Vulkan, or another 3D graphics API.
- [ ] **Render Basic Shapes :** Render spheres, boxes, planes for physics bodies
- [ ] **3D Camera Control :** Implement orbit/trackball or WASD-style camera
- [ ] **Sync Physics & Render Loops :** Use fixed time steps with interpolation if needed
- [ ] **Overlay Data :** Integrate ImGui for debugging (FPS, object states, toggles)
- [ ] **Input Handling :** Add simple controls to spawn/move bodies for testing
- [ ] **Create Test Scenes :** Towers, balls, pendulums — visual regression tests


## Phase 4: Core Rigid-Body Dynamics  

- [ ] **Physics Object Class :** Create a `Body` struct or class with position, velocity, force, and mass.  
- [ ] **Physics World Loop :** Create a `PhysicsWorld` with a list of bodies and `Step(float dt)` method.  
- [ ] **Apply Global Forces :** Apply forces like gravity each step.  
- [ ] **Integrate Motion :** Use simple Euler or semi-implicit Euler integration.  
- [ ] **Reset Forces :** Clear force accumulator each step.

---

## Phase 5: Rotation & Angular Dynamics  

- [ ] **Add Orientation :** Add quaternion or matrix rotation and inertia tensors.  
- [ ] **Apply Torque :** Compute angular acceleration and update angular velocity.  
- [ ] **Integrate Rotation :** Integrate orientation using quaternion derivative.  
- [ ] **Reset Torques :** Clear torque accumulator after each update.

---

## Phase 6: Collision Detection  

- [ ] **Collider Classes :** Define collider types (Sphere, Plane, etc.) with tags.  
- [ ] **Broad-Phase Culling :** Use bounding volumes or spatial partitioning to reduce pair tests.  
- [ ] **Narrow-Phase Tests :** Implement `Sphere–Sphere`, `Sphere–Plane`, etc.  
- [ ] **Collision Data :** Create `CollisionInfo` to store normal, penetration depth, etc.

---

## Phase 7: Collision Response  

- [ ] **Impulse Calculation :** Apply impulses based on relative velocity and restitution.  
- [ ] **Angular Impulses :** Update angular velocity based on impulse and inertia.  
- [ ] **Position Correction :** Correct small penetrations using push-back or split impulse.  
- [ ] **Restitution & Friction :** Add coefficients for realistic collisions.  
- [ ] **Verify Conservation :** Test simple interactions to validate behavior.

---

## Phase 8: Constraints & Springs  

- [ ] **Fixed & Hinge Joints :** Implement positional or rotational constraints between bodies.  
- [ ] **Springs (Hooke's Law) :** Add force-based springs with damping.  
- [ ] **Global Gravity :** Make gravity configurable.  
- [ ] **Damping :** Add velocity damping for realism and stability.

---

## Phase 9: Broad-Phase & Performance Optimization  

- [ ] **Bounding Volume Hierarchy :** Use spatial trees (octree, BVH) for collision efficiency.  
- [ ] **Profiling :** Add profiling to measure performance bottlenecks.  
- [ ] **Parallelism :** Multithread independent operations safely.  
- [ ] **Data Layout :** Use SoA or cache-optimized memory layouts.  
- [ ] **Stress Testing :** Run benchmarks with varying object counts.

---

## Phase 10: GPU Acceleration & Benchmarking  

- [ ] **GPU Prototyping :** Move physics steps to CUDA, OpenCL, or compute shaders.  
- [ ] **Data Transfer :** Minimize and optimize CPU–GPU data flows.  
- [ ] **Benchmark GPU vs CPU :** Compare performance across implementations.  
- [ ] **Validate GPU Results :** Ensure accuracy is consistent with CPU version.  
- [ ] **Tuning :** Optimize GPU threads, block sizes, and memory layout.

---

## Phase 11: Testing, Documentation & Polishing  

- [ ] **Unit Tests :** Create tests for math, collisions, forces, etc.  
- [ ] **Integration Tests :** Create real-world scenarios (e.g. towers, ragdolls).  
- [ ] **Documentation :** Comment your code and write README/API docs.  
- [ ] **Performance Reports :** Record benchmarks and publish findings.  
- [ ] **Polish & Release :** Clean code, tag release, and open-source if desired.

---

## Phase 12: Basic Physics Engine

- [ ] **Implement :** equations of motion for isolated rigid body
- [ ] **Add :** gravity and external forces
- [ ] **Basic :** time integrator (Explicit Euler or Verlet)
- [ ] **Test cases :** free fall, projectile, simple pendulum
- [ ] **Ensure :** numerical stability and conservation of physical quantities
- [ ] **Automated :** tests to validate physical behavior

---

## Phase 13: 3D Rendering Integration

- [ ] **Set up :** OpenGL window (GLFW/SDL)
- [ ] **Develop :** simple vertex + fragment shaders
- [ ] **3D camera :** system (WASD + mouse)
- [ ] **Synchronize :** physics loop and render loop
- [ ] **Display :** physical parameters via ImGui
- [ ] **Test scenes :** stacking cubes, miniature solar system
- [ ] **Compilation :** and execution documentation

---

## Phase 14: Collisions and Constraints

- [ ] **Implement :** broad-phase collision detection (grid, spatial trees)
- [ ] **Narrow-phase :** collision tests (sphere-sphere, AABB, plane)
- [ ] **Impulse-based :** collision resolution (restitution, friction)
- [ ] **Rigid constraints :** fixed joints, pivots, springs
- [ ] **Validate :** physics with stacking and joints
- [ ] **Documentation :** and tests for collisions and constraints

---

## Phase 15: Optimization Modules (CPU/GPU/MPI)

- [ ] **Multi-core :** optimization: std::thread, OpenMP
- [ ] **Intel TBB :** experimentation
- [ ] **Dedicated :** Git branches for each optimization strategy
- [ ] **MPI :** distributed computing implementation (halo exchange)
- [ ] **GPU :** acceleration (CUDA/OpenCL) for broad-phase
- [ ] **Profiling :** tuning, reliability testing

---

## Phase 16: Test Benches and Benchmarks

- [ ] **Design :** representative test scenarios (falls, stacking, constraints)
- [ ] **Automated :** benchmark framework
- [ ] **Performance :** data collection and graphs
- [ ] **Analysis :** and results publication

---

## Phase 17: Documentation and Publication

- [ ] **Complete :** README (installation, usage, contribution)
- [ ] **API :** documentation (Doxygen)
- [ ] **Usage :** examples and tutorials
- [ ] **GitHub :** releases with version notes
- [ ] **CI :** with GitHub Actions (build + tests)
- [ ] **Issue/pull :** request templates, contribution guide

---
