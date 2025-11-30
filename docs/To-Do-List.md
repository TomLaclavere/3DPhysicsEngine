# To-Do List for 3D Physics Engine project

While the roadmap outlines the major phases and tasks, this To-Do list captures smaller, actionable items that need to be addressed throughout the project. These side tasks are essential for maintaining code quality, ensuring smooth development, and enhancing the overall project. 

## General Tasks

- [ ] Improve collision algorithm (precision)
  - [x] Look at different collision algorithms
  - [ ] Improve integration method (to higher order)
  - [ ] Implement continuous collision detection (CCD)
  - [ ] Add penetration instead of freezing objects when a collision is detected
- [ ] Add script to run examples

## Code Quality and Maintenance

## Simulation Management

### Data Management
- [ ] **State Management**
  - [ ] Simulation state serialization
  - [ ] Checkpoint system
  - [ ] State restoration
  - [ ] Configuration presets

### Parameter Control
- [ ] **Core Parameters Implementation**
  - [ ] Integration method selection
  - [x] Collision detection precision
  - [x] Maximum iteration count

- [ ] **Parameter Management System**
  - [x] Runtime parameter modification ?
  - [ ] Parameter validation and bounds checking
  - [x] Default fallback values
  - [ ] Add objects from YAML

### Runtime Control
- [ ] **Simulation Control Commands**
  - [ ] Start/Stop simulation
  - [ ] Pause/Resume functionality
  - [x] Step-by-step execution
  - [x] Reset to initial state
  - [ ] Emergency stop

- [ ] **Runtime Interaction**
  - [ ] Real-time parameter modification
  - [ ] State saving/loading
  - [ ] Performance monitoring
  - [ ] Debug visualization toggles
  - [ ] Event logging system

### User Interface
- [ ] **Command Interface**
  - [x] Interactive command prompt
  - [x] Command history
  - [x] Tab completion
  - [x] Help system
  - [ ] Error handling and feedback
  - [x] Make it works as a terminal

- [ ] **Parameter Display**
  - [x] Current parameter values
  - [ ] Parameter modification history
  - [ ] Performance metrics
  - [ ] System status indicators

## CMake and Build System

## Documentation

## Unit Testing and CI/CD

- [ ] CI with Clang

# Done

## General Tasks
- [x] Clean physicsWorld
- [x] Write documentation for physicsWorld
- [x] Reorganize methods in config and physicsWorld
- [x] Add duration tracking (per iteration & total)
- [x] Document common.hpp 
- [x] Use functions from common.hpp for comparison
- [x] Unit test for Config
- [x] Unit test for PhysicsWorld
- [x] Remove Codacy
- [x] Test Physics
- [x] Test PhysicsWorld
- [x] Build running test example to verify the physical accuracy of simulation
  - [x] Free Fall
- [x] Verify scripts/ (check parameters and re-compile conditions)
- [x] Add printing method in PhysicsWorld
  - [x] Add print when simulation is running (see free fall example)
  - [x] Improve object printing

- [x] linenoise
  - [x] Clear the oldest lines in the history when the history file reaches a given size on quitting.
  - [x] Add autocompletion and persist new accepted commands to a file when they are entered 

## Code Quality and Maintenance

## Simulation Management

### Parameter Control
- [x] **Parameter Management System**
  - [x] Load from configuration file (YAML)
  - [x] Command-line parameter override
- [x] **Objects :** need to improve the construction and collision of all objects
  - [x] **Object**
  - [x] **Sphere**
  - [x] **AABB**
  - [x] **Implement Plane object**
  - [x] **Test Plane collisions**

### Runtime Control

### User Interface

### Data Management

## CMake and Build System

- [x] Build executable for main
- [x] Clean and refine CMakeLists.txt files
- [x] Clean/organize generation of build/
- [x] Add bash scripts for common commands :
    - [x] tests.sh
    - [x] coverage.sh (with argument for in console or HTML)
    - [x] build.sh
    - [x] clean.sh
    - [x] run.sh

## Documentation

## Unit Testing and CI/CD

- [x] Add Doxygen badge to README.md
- [x] Add CMake badge to README.md
- [x] Add GitHub Actions badge to README.md
