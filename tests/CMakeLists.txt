# Include GoogleTest
include(GoogleTest)

# Add test executable
add_executable(vector_tests 
    mathematics/test_vector.cpp
)

# Link against library and GTest
target_link_libraries(vector_tests
    PRIVATE 
        3DPhysicsEngine
        GTest::gtest_main
)

# Enable test discovery
gtest_discover_tests(vector_tests
    PROPERTIES
        LABELS "unit"
        TIMEOUT 10
    DISCOVERY_MODE PRE_TEST
)
