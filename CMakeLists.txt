cmake_minimum_required(VERSION 3.22.1)

# =============================================
# Project Configuration
# =============================================
project(3DPhysicsEngine VERSION 0.2.1 LANGUAGES CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================
# Project Options
# =============================================
option(USE_DOUBLE_PRECISION "Compile using double precision floating values" OFF)
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_COVERAGE "Enable coverage reporting" ON)

# =============================================
# Main Library Target
# =============================================
add_library(3DPhysicsEngine
    src/mathematics/vector.cpp
    src/mathematics/matrix.cpp
    src/mathematics/quaternion.cpp
    
    lib/mathematics/vector.hpp
    lib/mathematics/matrix.hpp
    lib/mathematics/quaternion.hpp
    lib/mathematics/common.hpp
    lib/precision.hpp
    lib/config.hpp
)

target_include_directories(3DPhysicsEngine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# =============================================
# Precision
# =============================================
if(USE_DOUBLE_PRECISION)
    target_compile_definitions(3DPhysicsEngine PUBLIC IS_USE_DOUBLE_PRECISION)
endif()

# =============================================
# Testing Configuration
# =============================================
if(BUILD_TESTS)
    enable_testing()
    
    # GoogleTest Fetch
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(tests)
endif()

# =============================================
# Coverage Analysis
# =============================================
if(ENABLE_COVERAGE AND BUILD_TESTS)
    # Check compiler compatibility
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Code coverage is only supported for GCC and Clang compilers")
        return()
    endif()

    # Find gcovr
    find_program(GCOVR_PATH gcovr)
    if(NOT GCOVR_PATH)
        message(WARNING "gcovr not found - code coverage targets will not be available")
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
        COMMAND ${GCOVR_PATH}
            ${GCOVR_COMMON_OPTS}
            --html
            -o ${CMAKE_BINARY_DIR}/coverage_report.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests and generating coverage reports..."
        DEPENDS 3DPhysicsEngine mathematics_test
    )

    add_custom_target(coverage-console
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${GCOVR_PATH}
            ${GCOVR_COMMON_OPTS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests and generating console coverage report..."
        DEPENDS 3DPhysicsEngine mathematics_test
    )
endif()
