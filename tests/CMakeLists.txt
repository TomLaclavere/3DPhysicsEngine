# =============================================
# Testing Configuration
# =============================================

include(GoogleTest)

function(add_engine_test TEST_NAME)
    # Create the test executable
    add_executable(${TEST_NAME} ${ARGN})
    
    # Set output directory for tests
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )
    
    # Include handling
    target_include_directories(${TEST_NAME} 
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            $<TARGET_PROPERTY:3DPhysicsEngine,INTERFACE_INCLUDE_DIRECTORIES>
    )
    
    # Link with main library and GTest
    target_link_libraries(${TEST_NAME}
        PRIVATE 
            3DPhysicsEngine 
            GTest::gtest_main
    )
    
    # Inherit compile features from main library
    target_compile_features(${TEST_NAME} 
        PRIVATE 
            $<TARGET_PROPERTY:3DPhysicsEngine,COMPILE_FEATURES>
    )
    
    # Add to GoogleTest
    gtest_discover_tests(${TEST_NAME}
        PROPERTIES 
            LABELS "unit" 
            TIMEOUT 10 
            DISCOVERY_MODE PRE_TEST
    )
    
    # Add install target for test executables (optional, for debugging)
    install(TARGETS ${TEST_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
    
    message(STATUS "Added test: ${TEST_NAME}")
endfunction()

# =============================================
# Individual Test Targets
# =============================================

add_engine_test(mathematics_test
    mathematics/test_common.cpp
    mathematics/test_vector.cpp
    mathematics/test_matrix.cpp
    mathematics/test_quaternion.cpp
)

add_engine_test(object_test
    objects/test_sphere.cpp
    objects/test_aabb.cpp
    objects/test_object.cpp
    objects/test_plane.cpp
)

add_engine_test(utility_test
    utilities/test_timer.cpp
    utilities/test_command.cpp)

add_engine_test(world_test
    world/test_config.cpp
    world/test_physics.cpp
    world/test_physicsworld.cpp)

# =============================================
# Test Configuration Summary
# =============================================

message(STATUS "Configured tests:")
message(STATUS "  mathematics_test - Unit tests for mathematical components")
message(STATUS "  object_test - Unit tests for physical objects")
message(STATUS "  utility_test - Unit tests for utilities")
message(STATUS " world_test - Unit tests for world")
message(STATUS "  Test output directory: ${CMAKE_BINARY_DIR}/bin/tests")
