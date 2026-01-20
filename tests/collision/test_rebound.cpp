#include "collision/collision_response.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <cmath>
#include <gtest/gtest.h>

TEST(CollisionResponse, SphereSphereElasticCollision)
{
    // Along X axis
    Sphere A(Vector3D(-1_d, 0_d, 0_d), 2_d);
    Sphere B(Vector3D(1_d, 0_d, 0_d), 2_d);

    A.setMass(1_d);
    B.setMass(1_d);

    A.setVelocity(Vector3D(1_d, 0_d, 0_d));
    B.setVelocity(Vector3D(-1_d, 0_d, 0_d));

    Contact contactAB;
    EXPECT_TRUE(A.computeCollision(B, contactAB));

    reboundCollision(A, B, contactAB, 1_d); // restitution = 1

    EXPECT_DECIMAL_EQ(A.getVelocity().getX(), -1_d);
    EXPECT_DECIMAL_EQ(B.getVelocity().getX(), 1_d);

    // Different masses
    Sphere sphereC(Vector3D(0_d, 2_d, 0_d), 2_d, Vector3D(0_d, 4_d, 0_d), 3_d);
    Sphere sphereD(Vector3D(0_d, 4_d, 0_d), 2_d, Vector3D(0_d, -4_d, 0_d), 1_d);

    Contact contactCD;
    EXPECT_TRUE(sphereC.computeCollision(sphereD, contactCD));

    reboundCollision(sphereC, sphereD, contactCD, 1_d);

    EXPECT_DECIMAL_EQ(0_d, sphereC.getVelocity().getY());
    EXPECT_DECIMAL_EQ(8_d, sphereD.getVelocity().getY());

    // 3D collision
    Sphere sphere1(Vector3D(2_d, 2_d, 1_d), 2_d, Vector3D(-1_d), 10_d);
    Sphere sphere2(Vector3D(2_d, 2_d, -1_d), 2_d, Vector3D(1_d), 10_d);

    Contact contact12;
    EXPECT_TRUE(sphere1.computeCollision(sphere2, contact12));

    reboundCollision(sphere1, sphere2, contact12, 1_d);

    EXPECT_VECTOR_EQ(Vector3D(-1_d, -1_d, 1_d), sphere1.getVelocity());
    EXPECT_VECTOR_EQ(Vector3D(1_d, 1_d, -1_d), sphere2.getVelocity());
}
