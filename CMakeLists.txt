cmake_minimum_required(VERSION 3.22.1)

# Project Name and Version
project(3DPhysicsEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilation Options
option(USE_DOUBLE_PRECISION "Compile using double precision floating values" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)

# Output Directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include Directories
include_directories(
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/lib/mathematics
)

# Source Files
file(GLOB_RECURSE PHYSICS_ENGINE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/lib/*.hpp
)

# Add Library Target
add_library(3DPhysicsEngine STATIC ${PHYSICS_ENGINE_SOURCES})

# Enable double precision if necessary
if(USE_DOUBLE_PRECISION)
    target_compile_definitions(3DPhysicsEngine PUBLIC IS_USE_DOUBLE_PRECISION)
endif()

# Enable warnings if requested
if(ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(3DPhysicsEngine PRIVATE /W4 /permissive-)
    else()
        target_compile_options(3DPhysicsEngine PRIVATE -Wall -Wextra -pedantic)
    endif()
endif()

# Installation
install(TARGETS 3DPhysicsEngine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/lib/ DESTINATION include/3DPhysicsEngine FILES_MATCHING PATTERN "*.hpp")

# Testing (optional)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Example Executable (optional)
option(BUILD_EXAMPLES "Build example executables" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
