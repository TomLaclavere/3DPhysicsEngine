# Include GoogleTest
include(GoogleTest)

# Add test executable
add_executable(mathematics_test 
    mathematics/test_vector.cpp
    mathematics/test_matrix.cpp
    mathematics/test_quaternion.cpp
)

target_include_directories(mathematics_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link against library and GTest
target_link_libraries(mathematics_test
    PRIVATE 
        3DPhysicsEngine
        GTest::gtest_main
)

# Enable test discovery
gtest_discover_tests(mathematics_test
    PROPERTIES
        LABELS "unit"
        TIMEOUT 10
    DISCOVERY_MODE PRE_TEST
)
