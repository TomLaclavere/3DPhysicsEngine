#include "collision/collision_response.hpp"
#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>

TEST(CollisionResponse, ReturnCases)
{
    // Using spheres to test return cases for simplicity
    Sphere  A;
    Sphere  B;
    Contact contact;

    A.setMass(1_d);
    B.setMass(1_d);

    EXPECT_TRUE(A.computeCollision(B, contact));

    Vector3D posA = A.getPosition();
    Vector3D posB = B.getPosition();

    // penetration < slop
    decimal slop    = contact.penetration; // exactly equal → early return
    decimal percent = 0.8_d;

    positionCorrection(A, B, contact, percent, slop);

    EXPECT_VECTOR_EQ(posA, A.getPosition());
    EXPECT_VECTOR_EQ(posB, B.getPosition());

    // Sum of inverse masses <= 0
    A.setMass(0_d);
    B.setMass(-1_d);

    positionCorrection(A, B, contact, percent, slop);

    EXPECT_VECTOR_EQ(posA, A.getPosition());
    EXPECT_VECTOR_EQ(posB, B.getPosition());

    // Objects separating
    A.setPosition(Vector3D(1_d, 0_d, 0_d));
    A.setSize(Vector3D(2_d));
    B.setPosition(Vector3D(-1_d, 0_d, 0_d));
    B.setSize(Vector3D(2_d));

    Vector3D velA = Vector3D(-1_d, 0_d, 0_d);
    Vector3D velB = Vector3D(1_d, 0_d, 0_d);
    A.setVelocity(velA);
    B.setVelocity(velB);

    reboundCollision(A, B, contact, 1_d);

    EXPECT_VECTOR_EQ(velA, A.getVelocity());
    EXPECT_VECTOR_EQ(velB, B.getVelocity());
}

TEST(CollisionResponse, PenetrationResponse)
{
    Sphere A(Vector3D(-0.5_d, 0_d, 0_d), 1_d);
    Sphere B(Vector3D(0.5_d, 0_d, 0_d), 2_d);

    A.setMass(1_d);
    B.setMass(1_d);

    Contact contactBefore;
    EXPECT_TRUE(A.computeCollision(B, contactBefore));

    decimal initialPenetration = contactBefore.penetration;

    ASSERT_GT(initialPenetration, 0_d);

    positionCorrection(A, B, contactBefore, 1_d, 0_d);

    // Recompute contact after correction
    Contact contactAfter;
    EXPECT_TRUE(A.computeCollision(B, contactAfter));

    EXPECT_DECIMAL_EQ(contactAfter.penetration, 0_d);
}

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
