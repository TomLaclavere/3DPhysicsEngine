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
        target_link_libraries   (3DPhysicsEngine PRIVATE --coverage)

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
    endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(3DPhysicsEngine PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(3DPhysicsEngine PRIVATE -fprofile-instr-generate)

        set(LLVM_PROFDATA llvm-profdata)
        set(LLVM_COV llvm-cov)
        set(PROFRAW_FILE default.profraw)
        set(PROFDATA_FILE coverage.profdata)
        set(COVERAGE_REPORT_DIR ${CMAKE_BINARY_DIR}/coverage_report)
        set(COVERAGE_HTML_FILE ${COVERAGE_REPORT_DIR}/coverage_report.html)

        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LLVM_PROFDATA} merge -sparse ${PROFRAW_FILE} -o ${PROFDATA_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_REPORT_DIR}
            COMMAND ${LLVM_COV} show
                $<TARGET_FILE:mathematics_test>
                -instr-profile=${PROFDATA_FILE}
                -format=html
                -output-dir=${COVERAGE_REPORT_DIR}
                -Xdemangler c++filt
                -Xreducer=none
                -ignore-filename-regex='.*(test|external).*'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating HTML coverage report with llvm-cov..."
            DEPENDS 3DPhysicsEngine mathematics_test
        )

        add_custom_target(coverage-console
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LLVM_PROFDATA} merge -sparse ${PROFRAW_FILE} -o ${PROFDATA_FILE}
            COMMAND ${LLVM_COV} report
                $<TARGET_FILE:mathematics_test>
                -instr-profile=${PROFDATA_FILE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating console coverage report with llvm-cov..."
            DEPENDS 3DPhysicsEngine mathematics_test
        )
    endif()

