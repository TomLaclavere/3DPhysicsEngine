# To-Do List for 3D Physics Engine project

While the roadmap outlines the major phases and tasks, this To-Do list captures smaller, actionable items that need to be addressed throughout the project. These side tasks are essential for maintaining code quality, ensuring smooth development, and enhancing the overall project. 

## General Tasks
- [x] Clean physicsWorld
- [x] Reorganize methods in config and physicsWorld
- [ ] Add duration tracking (per iteration & total)

## Code Quality and Maintenance

## Simulation Management

### Parameter Control
- [ ] **Core Parameters Implementation**
  - [ ] Integration method selection
  - [ ] Collision detection precision
  - [ ] Maximum iteration count

- [ ] **Parameter Management System**
  - [ ] Runtime parameter modification ?
  - [ ] Parameter validation and bounds checking
  - [ ] Default fallback values

### Runtime Control
- [ ] **Simulation Control Commands**
  - [ ] Start/Stop simulation
  - [ ] Pause/Resume functionality
  - [ ] Step-by-step execution
  - [ ] Reset to initial state
  - [ ] Emergency stop

- [ ] **Runtime Interaction**
  - [ ] Real-time parameter modification
  - [ ] State saving/loading
  - [ ] Performance monitoring
  - [ ] Debug visualization toggles
  - [ ] Event logging system

### User Interface
- [ ] **Command Interface**
  - [ ] Interactive command prompt
  - [ ] Command history
  - [ ] Tab completion
  - [ ] Help system
  - [ ] Error handling and feedback

- [ ] **Parameter Display**
  - [ ] Current parameter values
  - [ ] Parameter modification history
  - [ ] Performance metrics
  - [ ] System status indicators

### Data Management
- [ ] **State Management**
  - [ ] Simulation state serialization
  - [ ] Checkpoint system
  - [ ] State restoration
  - [ ] Configuration presets

## CMake and Build System

## Documentation

## Unit Testing and CI/CD

- [ ] CI with Clang

## Done

## General Tasks

## Code Quality and Maintenance

## Simulation Management

### Parameter Control
- [ ] **Parameter Management System**
  - [x] Load from configuration file (YAML)
  - [x] Command-line parameter override

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
