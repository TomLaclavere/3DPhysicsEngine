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
- [x] **Object Structs :**
  - [x] **Transform :** handle position and orientation of volumes
  - [x] **AABB :** Axis-Aligned Bounding Box = right block whose faces are parallel to the three axes of the reference frame
  - [x] **Bounding Sphere**
- [ ] **Testing :** Develop Unit Testing
  - [x] **Test Maths**
  - [x] **Test Objects**
  - [x] **Coverage :** Add coverage computing option during compilation.
  - [x] **Test CI :** Integration of tests procedure in GitHub.
    - [x] **CI with GCC**
    - [ ] **CI with Clang :**
- [x] **Documentation :** Doxygen
  - [x] Document functions and utilities.
  - [x] Documentation CI.

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------

## Phase 3: Rigid Body Dynamics & Physics World
  - [ ] **Core Setup :** 
    - [x] **Physical constants**
        - [x] Define and implement physical constants
        - [x] **Design a way to define all the variables of the simulation :** 
          - [x] Parameters file (yaml)
          - [x] Command lines
          - [x] Runtime commands
    - [x] **Time loop**
    - [x] **External forces :** gravity
    - [x] **Object management**
- [x] **Simple Motion :**
  - [x] **Linear Motion Basics**
    - [x] Position updates
    - [x] Velocity updates
    - [x] Basic force application
  - [x] **Basic Integration**
    - [x] **Semi-implicit Euler** (basic) 
    - [x] **Time step handling**
    - [x] **Verlet** (optional)
    - [x] **RK4** (optional)
- [ ] **Fundamental Test :** Add examples repository to implement testing cases for basic physical situations
  - [x] **Free fall**
  - [x] **Projectile motion**
  - [ ] **Pendulum** (optional)
  - [ ] **Check Energy/momentum conservation**
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

## Phase 4: Rotations & Angular Dynamics
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Extend RigidBody :**
  - [ ] **Orientation** (quaternion or rotation matrix)
  - [ ] **Angular velocity**
  - [ ] **Inertia tensor**
- [ ] **Add torque & angular acceleration**
- [ ] **Tests :** 
  - [ ] Rotating cube 
  - [ ] Spinning sphere
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

## Phase 5: Collisions
IMPORTANT: this is only brainstormed ideas, need to be reorganise in the future !
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Collision Detection :**
  - [ ] **Broad-Phase :** BVH (large N) | Uniform Grid (Small N) | Sweep-and-Prune (medium N) -> researches needed
  - [ ] **Narrow Phase :**
    - [ ] Sphere - Plan
    - [ ] Sphere - Sphere
    - [ ] AABB - AABB
- [ ] **Collision Response :**
  - [ ] **Impulse resolution**
  - [ ] **Restitution coefficient** (bounciness)
  - [ ] **Basic friction**
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

## Phase 5.5: Advanced Physics features (optional)
IMPORTANT: this is only brainstormed ideas, need to be reorganise in the future !
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
- [ ] **Constraints :**
  - [ ] **Joints**
  - [ ] **Pivot**
- [ ] **Advanced forces :**
  - [ ] **Drag**
  - [ ] **Spring**
  - [ ] **Damping**
- [ ] **Tests, Validation & CI :**
  - [ ] **Bouncing balls**
  - [ ] **Canon ball vs box**
  - [ ] **Box tower**
- **Performances :**
  - [ ] **Benchmark & Profiling**
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

## Phase 6: Visualization
IMPORTANT: this is only brainstormed ideas, need to be reorganise in the future !
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
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

## Phase 7: First Optimisation (CPU & Memory)
IMPORTANT: this is only brainstormed ideas, need to be reorganise in the future !
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
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

## Phase 8: Second Optimisation (GPU)   
IMPORTANT: this is only brainstormed ideas, need to be reorganise in the future !
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
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

## Phase 9: Portage Fortran ?
IMPORTANT: this is only brainstormed ideas, need to be reorganise in the future !
- [ ] **Brainstorm on ROADMAP**
- [ ] **Brainstorm on To-Do-List**
