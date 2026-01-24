cmake_minimum_required(VERSION 3.22.1)

# =============================================
# Project Configuration
# =============================================
project(3DPhysicsEngine 
    VERSION 0.2.3 
    DESCRIPTION "A 3D Physics Engine for real-time simulations"
    HOMEPAGE_URL "https://github.com/TomLaclavere/3DPhysicsEngine"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set policies to avoid warnings and ensure modern behavior
cmake_policy(SET CMP0077 NEW)

# =============================================
# Compiler Selection
# =============================================
option(3DPE_USE_CLANG "Compile with Clang instead of GCC" OFF)

if(3DPE_USE_CLANG)
    message(STATUS "=> Using Clang/Clang++ as compiler")
    set(CMAKE_C_COMPILER clang CACHE STRING "Clang compiler" FORCE)
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "Clang++ compiler" FORCE)
else()
    message(STATUS "=> Using GCC/g++ as compiler")
    set(CMAKE_C_COMPILER gcc CACHE STRING "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER g++ CACHE STRING "C++ compiler" FORCE)
endif()

# =============================================
# Project Options
# =============================================
option(3DPE_USE_DOUBLE_PRECISION "Compile using double precision floating values" OFF)
option(3DPE_BUILD_EXAMPLES "Build example applications" ON)
option(3DPE_BUILD_TESTS "Build unit tests" ON)
option(3DPE_BUILD_BENCHMARKS "Build benchmark applications" ON)
option(3DPE_ENABLE_COVERAGE "Enable coverage reporting" ON)
option(3DPE_ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(3DPE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

# Advanced options
set(3DPE_GCC_EXTRA_FLAGS "" CACHE STRING "Extra flags for GCC")
set(3DPE_CLANG_EXTRA_FLAGS "" CACHE STRING "Extra flags for Clang")

# =============================================
# Find Dependencies
# =============================================
find_package(yaml-cpp REQUIRED)

if(TARGET yaml-cpp::yaml-cpp)
    set(YAML_CPP_TARGET yaml-cpp::yaml-cpp)
    message(STATUS "Found yaml-cpp: ${yaml-cpp_DIR} (imported target)")
else()
    set(YAML_CPP_TARGET yaml-cpp)
    message(STATUS "Found yaml-cpp: ${yaml-cpp_DIR}")
endif()

# =============================================
# Source Files Groups
# =============================================
set(ENGINE_MATH_SOURCES
    src/mathematics/vector.cpp
    src/mathematics/matrix.cpp
    src/mathematics/quaternion.cpp
    src/mathematics/math_io.cpp
)

set(ENGINE_OBJECT_SOURCES
    src/objects/object.cpp
    src/objects/aabb.cpp
    src/objects/sphere.cpp
    src/objects/plane.cpp
)

set(ENGINE_COLLISION_SOURCES
    src/collision/broad_collision.cpp
    src/collision/narrow_collision.cpp
    src/collision/collision_response.cpp
    )

set(ENGINE_UTILITIES_SOURCE
    src/utilities/timer.cpp
    src/utilities/command.cpp)

set(ENGINE_WORLD_SOURCES
    src/world/config.cpp
    src/world/physics.cpp
    src/world/physicsWorld.cpp)

set(ENGINE_EXTERNAL_SOURCES
    src/external/linenoise/linenoise.c
    src/external/linenoise/history.cpp
)

set(ENGINE_ALL_SOURCES
    ${ENGINE_CORE_SOURCES}
    ${ENGINE_MATH_SOURCES}
    ${ENGINE_OBJECT_SOURCES}
    ${ENGINE_COLLISION_SOURCES}
    ${ENGINE_UTILITIES_SOURCE}
    ${ENGINE_WORLD_SOURCES}
    ${ENGINE_EXTERNAL_SOURCES}
)

# =============================================
# Main Library Target
# =============================================
add_library(3DPhysicsEngine)

target_sources(3DPhysicsEngine
    PRIVATE
        ${ENGINE_ALL_SOURCES}
)

# Include handling
target_include_directories(3DPhysicsEngine
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>
)

# Dependency linking
target_link_libraries(3DPhysicsEngine 
    PUBLIC 
        ${YAML_CPP_TARGET}
)

# C++ standard specification
target_compile_features(3DPhysicsEngine PUBLIC cxx_std_23)

# Precision definition using generator expression
target_compile_definitions(3DPhysicsEngine
    PUBLIC 
        $<$<BOOL:${3DPE_USE_DOUBLE_PRECISION}>:IS_DOUBLE_PRECISION>
)

# =============================================
# Clang-Tidy Configuration
# =============================================
if(3DPE_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found!")
    endif()
endif()

# =============================================
# Strict compiler warnings
# =============================================
set(3DPE_COMMON_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wold-style-cast
    -Wunused
)


# =============================================
# Compiler-specific warnings and extra flags
# =============================================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(3DPE_ACTUAL_FLAGS ${3DPE_COMMON_WARNINGS} ${3DPE_GCC_EXTRA_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(3DPE_ACTUAL_FLAGS ${3DPE_COMMON_WARNINGS} ${3DPE_CLANG_EXTRA_FLAGS})
endif()

if(3DPE_WARNINGS_AS_ERRORS)
    list(APPEND 3DPE_ACTUAL_FLAGS -Werror)
endif()

target_compile_options(3DPhysicsEngine PRIVATE ${3DPE_ACTUAL_FLAGS})

# Desactivate warnings for external linenoise library
set_source_files_properties(
    src/external/linenoise/linenoise.c
    PROPERTIES COMPILE_FLAGS "-w"
)

# =============================================
# Main Application Executable
# =============================================
add_executable(PhysicsEngine src/main.cpp)
target_link_libraries(PhysicsEngine PRIVATE 3DPhysicsEngine)

add_custom_target(run
    COMMAND $<TARGET_FILE:PhysicsEngine>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS PhysicsEngine
    COMMENT "Running PhysicsEngine..."
)

# =============================================
# Examples
# =============================================
if(3DPE_BUILD_EXAMPLES)
    message(STATUS "Building examples")

    # ---------------------------------------------
    # Example: Free Fall Simulation
    # ---------------------------------------------
    add_executable(example_Free_Fall examples/Free_Fall/main.cpp)
    target_link_libraries(example_Free_Fall PRIVATE 3DPhysicsEngine)
    target_include_directories(example_Free_Fall PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    set_target_properties(example_Free_Fall PROPERTIES
        OUTPUT_NAME "Free_Fall"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
        FOLDER "Examples"
    )

    add_custom_target(Free_Fall
        COMMAND $<TARGET_FILE:example_Free_Fall>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS example_Free_Fall
        COMMENT "Running example: Free_Fall"
    )

    # ---------------------------------------------
    # Example: Projectiles Simulation
    # ---------------------------------------------
    add_executable(example_Projectiles examples/Projectiles/main.cpp)
    target_link_libraries(example_Projectiles PRIVATE 3DPhysicsEngine)
    target_include_directories(example_Projectiles PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    set_target_properties(example_Projectiles PROPERTIES
        OUTPUT_NAME "Projectiles"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
        FOLDER "Examples"
    )

    add_custom_target(Projectiles
        COMMAND $<TARGET_FILE:example_Projectiles>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS example_Projectiles
        COMMENT "Running example: Projectiles"
    )
endif()

# =============================================
# Testing Configuration
# =============================================
if(3DPE_BUILD_TESTS)
    include(CTest)
    
    message(STATUS "Building tests")
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0 
    )
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(tests)
endif()

# =============================================
# Output Directory Configuration
# =============================================
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# =============================================
# Coverage Analysis
# =============================================
if(3DPE_ENABLE_COVERAGE AND 3DPE_BUILD_TESTS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Code coverage is only supported for GCC and Clang compilers")
    else()
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            find_program(GCOVR_PATH gcovr)
            if(NOT GCOVR_PATH)
                message(WARNING "gcovr not found - coverage targets disabled")
            else()
                target_compile_options(3DPhysicsEngine PRIVATE --coverage -fprofile-arcs -ftest-coverage)
                target_link_libraries(3DPhysicsEngine PRIVATE --coverage)

                set(GCOVR_COMMON_OPTS
                    --exclude '.*\.hpp'
                    --exclude ".*/linenoise/.*"
                    --exclude-unreachable-branches
                    --exclude-throw-branches
                    --root=${CMAKE_SOURCE_DIR}
                    --object-directory=${CMAKE_BINARY_DIR}
                    --print-summary
                    --filter="${CMAKE_SOURCE_DIR}/src/.*"
                    --filter="${CMAKE_SOURCE_DIR}/lib/.*"
                )

                add_custom_target(coverage
                    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_report
                    COMMAND ${GCOVR_PATH}
                        ${GCOVR_COMMON_OPTS}
                        --html
                        --html-details
                        --html-theme github.dark-green
                        -o ${CMAKE_BINARY_DIR}/coverage_report/coverage_report.html
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running tests and generating HTML coverage report..."
                    DEPENDS 3DPhysicsEngine mathematics_test
                )

                add_custom_target(coverage-console
                    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                    COMMAND ${GCOVR_PATH} ${GCOVR_COMMON_OPTS}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running tests and generating console coverage report..."
                    DEPENDS 3DPhysicsEngine mathematics_test
                )
            endif()

        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # Find LLVM coverage tools
            find_program(LLVM_COV_PATH llvm-cov)
            find_program(LLVM_PROFDATA_PATH llvm-profdata)
            
            if(NOT LLVM_COV_PATH OR NOT LLVM_PROFDATA_PATH)
                message(WARNING "llvm-cov or llvm-profdata not found - coverage targets disabled")
            else()
                # Set Clang coverage compilation flags
                target_compile_options(3DPhysicsEngine PRIVATE -fprofile-instr-generate -fcoverage-mapping)
                target_link_libraries(3DPhysicsEngine PRIVATE -fprofile-instr-generate -fcoverage-mapping)

                # Common llvm-cov options 
                set(LLVM_COV_COMMON_OPTS
                    --ignore-filename-regex=".*_test\\..*"
                    --ignore-filename-regex=".*/tests?/.*"
                    --compilation-dir=${CMAKE_BINARY_DIR}
                    --instr-profile=${CMAKE_BINARY_DIR}/coverage/merged.profdata
                )

                # HTML coverage report target
                add_custom_target(coverage
                    # Create coverage directory first
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
                    # Run tests with environment variable set
                    COMMAND env LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/coverage/3DPhysicsEngine_%p.profraw ${CMAKE_CTEST_COMMAND} --output-on-failure
                    # Merge profiling data
                    COMMAND ${LLVM_PROFDATA_PATH} merge -sparse ${CMAKE_BINARY_DIR}/coverage/*.profraw -o ${CMAKE_BINARY_DIR}/coverage/merged.profdata
                    # Create report directory
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_report
                    # Generate HTML report
                    COMMAND ${LLVM_COV_PATH} show
                        ${LLVM_COV_COMMON_OPTS}
                        --format=html
                        --output-dir=${CMAKE_BINARY_DIR}/coverage_report
                        $<TARGET_FILE:3DPhysicsEngine>
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running tests and generating HTML coverage report..."
                    DEPENDS 3DPhysicsEngine mathematics_test
                )

                # Console coverage report target
                add_custom_target(coverage-console
                    # Create coverage directory first
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
                    # Run tests with environment variable set
                    COMMAND env LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/coverage/3DPhysicsEngine_%p.profraw ${CMAKE_CTEST_COMMAND} --output-on-failure
                    # Merge profiling data
                    COMMAND ${LLVM_PROFDATA_PATH} merge -sparse ${CMAKE_BINARY_DIR}/coverage/*.profraw -o ${CMAKE_BINARY_DIR}/coverage/merged.profdata
                    # Generate console report
                    COMMAND ${LLVM_COV_PATH} report
                        ${LLVM_COV_COMMON_OPTS}
                        $<TARGET_FILE:3DPhysicsEngine>
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running tests and generating console coverage report..."
                    DEPENDS 3DPhysicsEngine mathematics_test
                )
            endif()
        endif()
    endif()
endif()

# =============================================
# Summary
# =============================================
message(STATUS "")
message(STATUS "3DPhysicsEngine Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  C++ Standard: 23")
message(STATUS "  Double Precision: ${3DPE_USE_DOUBLE_PRECISION}")
message(STATUS "  Tests: ${3DPE_BUILD_TESTS}")
message(STATUS "  Coverage: ${3DPE_ENABLE_COVERAGE}")
message(STATUS "  Warnings as Errors: ${3DPE_WARNINGS_AS_ERRORS}")
message(STATUS "  Clang-Tidy: ${3DPE_ENABLE_CLANG_TIDY}")
message(STATUS "")
