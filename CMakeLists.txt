cmake_minimum_required(VERSION 3.22.1)

# =============================================
# Compiler Selection
# =============================================
option(USE_CLANG "Compile with Clang instead of GCC" OFF)
set(GCC_EXTRA_FLAGS "" CACHE STRING "Extra flags for GCC")
set(CLANG_EXTRA_FLAGS "" CACHE STRING "Extra flags for Clang")

if(USE_CLANG)
    message(STATUS "=> Using Clang/Clang++ as compiler")
    set(CMAKE_C_COMPILER   clang  CACHE STRING "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler" FORCE)
else()
    message(STATUS "=> Using GCC/g++ as compiler")
    set(CMAKE_C_COMPILER   gcc   CACHE STRING "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER g++   CACHE STRING "C++ compiler" FORCE)
endif()

# =============================================
# Project Configuration
# =============================================
project(3DPhysicsEngine VERSION 0.2.3 LANGUAGES CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================
# Project Options
# =============================================
option(USE_DOUBLE_PRECISION "Compile using double precision floating values" OFF)
option(TESTS                "Build unit tests"                               ON)
option(COVERAGE             "Enable coverage reporting"                      ON)

# =============================================
# Main Library Target
# =============================================
add_library(3DPhysicsEngine
    src/mathematics/vector.cpp
    src/mathematics/matrix.cpp
    src/mathematics/quaternion.cpp

    src/objects/object.cpp
    src/objects/AABB.cpp
    src/objects/sphere.cpp
)

target_include_directories(3DPhysicsEngine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# =============================================
# Precision Definition
# =============================================
if(DOUBLE_PRECISION)
    target_compile_definitions(3DPhysicsEngine PUBLIC IS_DOUBLE_PRECISION)
endif()

# =============================================
# Compiler‑specific extra flags
# =============================================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(3DPhysicsEngine PRIVATE ${GCC_EXTRA_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(3DPhysicsEngine PRIVATE ${CLANG_EXTRA_FLAGS})
endif()

# =============================================
# Testing Configuration
# =============================================
if(TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()

# =============================================
# Coverage Analysis
# =============================================
if(COVERAGE AND TESTS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Code coverage is only supported for GCC and Clang compilers")
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        find_program(GCOVR_PATH gcovr)
        if(NOT GCOVR_PATH)
            message(WARNING "gcovr not found - coverage targets disabled")
            return()
        endif()

        target_compile_options(3DPhysicsEngine PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_libraries(3DPhysicsEngine PRIVATE --coverage)

        set(GCOVR_COMMON_OPTS
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

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Find LLVM coverage tools
        find_program(LLVM_COV_PATH llvm-cov)
        find_program(LLVM_PROFDATA_PATH llvm-profdata)
        
        if(NOT LLVM_COV_PATH OR NOT LLVM_PROFDATA_PATH)
            message(WARNING "llvm-cov or llvm-profdata not found - coverage targets disabled")
            return()
        endif()

        # Set Clang coverage compilation flags - must be added to both compile and link
        target_compile_options(3DPhysicsEngine PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_libraries(3DPhysicsEngine PRIVATE -fprofile-instr-generate -fcoverage-mapping)

        # Set environment variable for profiling output using set(ENV{VAR} VALUE) syntax
        set(ENV{LLVM_PROFILE_FILE} "${CMAKE_BINARY_DIR}/coverage/3DPhysicsEngine_%p.profraw")

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
