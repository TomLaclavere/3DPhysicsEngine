cmake_minimum_required(VERSION 3.22.1)

# Project Name and Version
project(3DPhysicsEngine VERSION 0.2.1 LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilation Options
option(USE_DOUBLE_PRECISION "Compile using double precision floating values" OFF)
option(BUILD_TESTS "Build unit tests" ON)

# Create library target
add_library(3DPhysicsEngine
    # Source files
    src/mathematics/vector.cpp
    src/mathematics/matrix.cpp
    src/mathematics/quaternion.cpp
    
    # Header files
    lib/mathematics/vector.hpp
    lib/mathematics/matrix.hpp
    lib/mathematics/quaternion.hpp
    lib/mathematics/common.hpp
    lib/precision.hpp
    lib/config.hpp
)

# Set include directories
target_include_directories(3DPhysicsEngine
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Enable double precision if necessary
if(USE_DOUBLE_PRECISION)
    target_compile_definitions(3DPhysicsEngine PUBLIC IS_USE_DOUBLE_PRECISION)
endif()

# Testing setup
if(BUILD_TESTS)
    enable_testing()
    
    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Add tests directory
    add_subdirectory(tests)
endif()
