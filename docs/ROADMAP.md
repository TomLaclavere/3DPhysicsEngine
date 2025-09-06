# 3D Physics Engine - Roadmap

---

## Phase 1: Setup and General Architecture

- [x] Initialize GitHub repository
- [x] Define project structure
- [x] Build all the project repositories (inspired by Project Chrono)
- [x] Configure CMake

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 2: Setup & Maths Foundations  

- [x] **Maths Library :** Implement or integrate 3D maths types: `vec3` for 3D vectors, matrices, and quaternion for rotation.  
  - [x] **Vector3D :** Implement basic vector operations (addition, subtraction, scalar multiplication, dot product, cross product).
  - [x] **Vector3D Tests :** Write unit tests to verify vector operations.
  - [x] **Matrix3x3 :** Implement matrix operations (multiplication, transpose, inverse).
  - [x] **Matrix3x3 Tests :** Write unit tests to verify matrix operations.
  - [x] **Quaternion :** Implement quaternion operations (multiplication, normalization, inverse).
    - [x] Verify that all quaternion functions are correctly implemented
    - [x] **Euler angle :** avoid code duplication, in particular with Euler angle
  - [x] **Quaternion Tests :** Write unit tests to verify quaternion operations.
- [ ] **Object Structs :**
  - [ ] **Transform :** handle position and orientation of volumes
  - [ ] **AABB :** Axis-Aligned Bounding Box = right block whose faces are parallel to the three axes of the reference frame
  - [ ] **Bounding Sphere**
- [ ] **Testing :** Develop Unit Testing
  - [x] **Test Maths**
  - [ ] **Test Objects**
  - [ ] **Coverage :** Add coverage computing option during compilation
  - [ ] **Test CI :** Integration of tests procedure in GitHub
- [ ] **Documentation :** Document your maths types and utilities.

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 3: Rigid Body Dynamics
- [ ] **Core Model :**
  - [ ] **RigidBody :** mass, barycentre, inertia, torque, string, energy (kinetic and potential), hook, damping
  - [ ] **PhysicsWorld :** 
    - [ ] **Time loop* 
    - [ ] **External forces :** gravity, wind, ...
- [ ] **Equation of motion**
- [ ] **Rotation & Angular Dynamics** 
  - [ ] **Add Orientation :** Add quaternion or matrix rotation and inertia tensors.  
  - [ ] **Apply Torque :** Compute angular acceleration and update angular velocity.  
- [ ] **Temporal Integration :** 
  - [ ] **Semi-implicit Euler**
  - [ ] **Verlet** ?
- [ ] **Fundamental Test :** 
  - [ ] **Free fall**
  - [ ] **Projectile**
  - [ ] **Pendulum**
  - [ ] **Energy/momentum conservation**
- [ ] **Configuration parameters :** yaml/json
- [ ] **Performance Benchmark**
- [ ] **Validation & Release :**
  - [ ] **Standard testing :** reference scenes, physical validity, ...
  - [ ] **Documentation :**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Performance report*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 4: Collisions & Constraints
 - [ ] **Collision Detection :**
   - [ ] **Broad-Phase :** BVH (large N) | Uniform Grid (Small N) | Sweep-and-Prune (medium N) -> researches needed
   - [ ] **Narrow Phase :**
     - [ ] Sphere - Plan
     - [ ] Sphere - Sphere
     - [ ] AABB - AABB
 - [ ] **Collision Response :**
   - [ ] **Collider :** RigidBody reference, during broad phase. 
   - [ ] **CollisionInfo :** generating during narrow phase. Contact point, normal's contact penetration...
 - [ ] **Resolution :**
   - [ ] **Impulses :** restitution, friction
   - [ ] **Position corrections**
   - [ ] **Linear and angular speed corrections**
 - [ ] **Constraints :**
   - [ ] **Joints**
   - [ ] **Energy conservation**
   - [ ] **Pivot**
 - [ ] **Tests, Validation & CI :**
   - [ ] **Bouncing balls ?**
   - [ ] **Canon ball vs box ?**
   - [ ] **Box tower ?**
   - [ ] **Comparison with Chrono/ReactPhysics3D* ?*
- [ ] **Validation & Release :**
  - [ ] **Standard testing :** reference scenes, physical validity, ...
  - [ ] **Documentation :**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Performance report*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 5: 3D Visualization
 - [ ] **Choice of Graphics Layer :**
   - [ ] *GLFW + OpenGL*
   - [ ] *Vulkan*
   - [ ] *VTK / ParaView*
   - [ ] *?*
 - [ ] **Rendering :**
   - [ ] **Basic Geometry :** spheres, boxes, planes...
   - [ ] **Basic Shaders**
   - [ ] **Physics-Rendering Synchronisation :** synchronisation between simulation FPS and rendering FPS
 - [ ] **Camera Control :** WASD/Mouse
 - [ ] **Debug Tools :**
   - [ ] **Overlay Data :** FPS, objects state, ...
   - [ ] **Runtime Parameters**
   - [ ] **Test Scenes :** Towers, balls, pendulums — visual regression tests
- [ ] **Validation & Release :**
  - [ ] **Standard testing :** reference scenes, physical validity, ...
  - [ ] **Documentation :**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Performance report*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 6: First Optimisation (CPU & Memory)
 - [ ] **CPU Optimisation**
   - [ ] **Profiler :** Advisor, Maqao, IntelAdvisor, VTune
   - [ ] **Vectorisation "home-made"**
   - [ ] **External tools :** MKL, TBB, SYCL, OpenMP, Eve, ...
 - [ ] **Memory Optimisation :**
   - [ ] **Profiler :** Malt    
   - [ ] **Structure of Arrays**
   - [ ] **Optimisation :** caches, data splitting
 - [ ] **Benchmarks :**
   - [ ] **Scalability :** with the numbers of objects N
   - [ ] **Comparison :** naive, vectorised, parallel, ...
   - [ ] **External Comparison :** **Comparison with Chrono/ReactPhysics3D* ?*
- [ ] **Validation & Release :**
  - [ ] **Standard testing :** reference scenes, physical validity, ...
  - [ ] **Documentation :**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Performance report*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 7: Second Optimisation (GPU)   
 - [ ] **GPU Implementation** 
   - [ ] **Software choice :** CUDA, SYCL, OpenCL, ...
   - [ ] **GPU Implementation**
 - [ ] **Optimisation :**
   - [ ] **Profiling :** NSight, ...
   - [ ] **Data Transfer**
   - [ ] **Tuning :** threads, block sizes, memory, ...
 - [ ] **Hybrid CPU - GPU :** *Star PU*
 - [ ] **Benchmarks :**
   - [ ] **CPU - GPU consistency**
   - [ ] **CPU vs GPU**
   - [ ] **High testing**
- [ ] **Validation & Release :**
  - [ ] **Standard testing :** reference scenes, physical validity, ...
  - [ ] **Documentation :**
    - [ ] *Doxygen & GitHub Pages*
    - [ ] *Performance report*
    - [ ] *Installation & Usage guides*
    - [ ] *GitHub notes*

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 8: Portage Fortran ?
