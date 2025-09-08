include(GoogleTest)

function(add_engine_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/lib
    )
    target_link_libraries(${TEST_NAME}
        PRIVATE 3DPhysicsEngine GTest::gtest_main
    )
    gtest_discover_tests(${TEST_NAME}
        PROPERTIES LABELS "unit" TIMEOUT 10 DISCOVERY_MODE PRE_TEST
    )
endfunction()

add_engine_test(mathematics_test
    mathematics/test_vector.cpp
    mathematics/test_matrix.cpp
    mathematics/test_quaternion.cpp
)

add_engine_test(object_test
    objects/test_sphere.cpp
    objects/test_AABB.cpp
)
