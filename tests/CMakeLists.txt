# Include GoogleTest
include(GoogleTest)

# Add test executable
add_executable(mathematics_test 
    mathematics/test_vector.cpp
    mathematics/test_matrix.cpp
)

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
