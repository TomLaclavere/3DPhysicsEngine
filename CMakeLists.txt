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
        # Clang-specific coverage flags
        target_compile_options(3DPhysicsEngine PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(3DPhysicsEngine PRIVATE -fprofile-instr-generate)

        # Find LLVM tools with better error handling
        find_program(LLVM_PROFDATA_PATH llvm-profdata
            DOC "Path to llvm-profdata utility"
            REQUIRED  # This will fail if not found
        )
        find_program(LLVM_COV_PATH llvm-cov
            DOC "Path to llvm-cov utility"
            REQUIRED  # This will fail if not found
        )
        
        if(NOT LLVM_PROFDATA_PATH OR NOT LLVM_COV_PATH)
            message(FATAL_ERROR "llvm-profdata or llvm-cov not found - please install LLVM coverage tools")
        endif()

        # HTML coverage report for Clang
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LLVM_PROFDATA_PATH} merge -sparse default.profraw -o coverage.profdata
            COMMAND ${LLVM_COV_PATH} show
                $<TARGET_FILE:mathematics_test>
                -instr-profile=coverage.profdata
                -format=html
                -output-dir=${CMAKE_BINARY_DIR}/coverage_report
            COMMAND ${CMAKE_COMMAND} -E rename 
                ${CMAKE_BINARY_DIR}/coverage_report/index.html
                ${CMAKE_BINARY_DIR}/coverage_report/coverage_report.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating HTML coverage report for Clang..."
            DEPENDS 3DPhysicsEngine mathematics_test
        )

        # Console coverage report for Clang
        add_custom_target(coverage-console
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LLVM_PROFDATA_PATH} merge -sparse default.profraw -o coverage.profdata
            COMMAND ${LLVM_COV_PATH} report
                $<TARGET_FILE:mathematics_test>
                -instr-profile=coverage.profdata
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating console coverage report for Clang..."
            DEPENDS 3DPhysicsEngine mathematics_test
        )
    endif()
endif()
