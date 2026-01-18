#include "objects/aabb.hpp"
#include "objects/object.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <cmath>
#include <gtest/gtest.h>

TEST(AABBTest, ConstructorsAndGetters)
{
    Vector3D position(1_d, 2_d, 3_d);
    Vector3D rotation(0_d, 0_d, 0_d);
    Vector3D size(4_d, 5_d, 6_d);
    Vector3D velocity(0.1_d, 0.2_d, 0.3_d);
    Vector3D acceleration(0.01_d, 0.02_d, 0.03_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;

    AABB aabb(position, rotation, size, velocity, acceleration, force, torque, mass);

    EXPECT_EQ(aabb.getPosition(), position);
    EXPECT_EQ(aabb.getRotation(), rotation);
    EXPECT_EQ(aabb.getSize(), size);
    EXPECT_EQ(aabb.getVelocity(), velocity);
    EXPECT_EQ(aabb.getAcceleration(), acceleration);
    EXPECT_EQ(aabb.getForce(), force);
    EXPECT_EQ(aabb.getTorque(), torque);
    EXPECT_DECIMAL_EQ(aabb.getMass(), mass);
    EXPECT_EQ(aabb.getType(), ObjectType::AABB);

    EXPECT_EQ(aabb.getMin(), position - size * 0.5_d);
    EXPECT_EQ(aabb.getMax(), position + size * 0.5_d);

    AABB aabb_(position, size, mass);
    EXPECT_EQ(aabb_.getTorque(), Vector3D());

    AABB aabb_test(position, size, velocity, mass);
    EXPECT_EQ(aabb_test.getRotation(), Vector3D());
}

TEST(AABBTest, Setters)
{
    AABB aabb;

    Vector3D newPosition(4_d, 5_d, 6_d);
    Vector3D newsize(7_d, 8_d, 9_d);
    Vector3D newVelocity(0.4_d, 0.5_d, 0.6_d);
    Vector3D newAcceleration(0.04_d, 0.05_d, 0.06_d);
    Vector3D newForce(0.004_d, 0.005_d, 0.006_d);
    Vector3D newTorque(0.0004_d, 0.0005_d, 0.0006_d);
    decimal  newMass = 20.0_d;

    aabb.setPosition(newPosition);
    aabb.setSize(newsize);
    aabb.setVelocity(newVelocity);
    aabb.setAcceleration(newAcceleration);
    aabb.setForce(newForce);
    aabb.setTorque(newTorque);
    aabb.setMass(newMass);

    EXPECT_EQ(aabb.getPosition(), newPosition);
    EXPECT_EQ(aabb.getSize(), newsize);
    EXPECT_EQ(aabb.getVelocity(), newVelocity);
    EXPECT_EQ(aabb.getAcceleration(), newAcceleration);
    EXPECT_EQ(aabb.getForce(), newForce);
    EXPECT_EQ(aabb.getTorque(), newTorque);
    EXPECT_DECIMAL_EQ(aabb.getMass(), newMass);
}

TEST(AABBTest, integrate)
{
    Vector3D position(1_d, 2_d, 3_d);
    Vector3D rotation(0_d, 0_d, 0_d);
    Vector3D size(4_d, 5_d, 6_d);
    Vector3D velocity(-1_d, 0_d, 0.5_d);
    Vector3D acceleration(2_d, 5_d, -2_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;

    AABB    aabb(position, rotation, size, velocity, acceleration, force, torque, mass);
    decimal dt = 2.0_d;

    aabb.integrate(dt);

    EXPECT_EQ(aabb.getPosition(), Vector3D(7_d, 22_d, -4_d));
    EXPECT_EQ(aabb.getVelocity(), Vector3D(3_d, 10_d, -3.5_d));
}

TEST(AABBTest, AABBCollision)
{
    AABB    aabb1(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB    aabb2(Vector3D(1_d, 1_d, 1_d), Vector3D(2_d, 2_d, 2_d));
    AABB    aabb3(Vector3D(3_d, 3_d, 3_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    EXPECT_TRUE(aabb1.computeCollision(aabb2, contact));  // Overlapping
    EXPECT_FALSE(aabb1.computeCollision(aabb3, contact)); // Not overlapping

    // Test over others dimensions
    AABB aabbY(Vector3D(0_d, 10_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabbZ(Vector3D(0_d, 0_d, -10_d), Vector3D(2_d, 2_d, 2_d));
    EXPECT_FALSE(aabb1.computeCollision(aabbY, contact));
    EXPECT_FALSE(aabb1.computeCollision(aabbZ, contact));
}

TEST(AABBTest, AABBSphereCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere sphere_outside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere sphereTouchingFace(Vector3D(3_d, 1_d, 1_d), 4_d);

    EXPECT_TRUE(aabb.computeCollision(sphere_inside, contact));
    EXPECT_FALSE(aabb.computeCollision(sphere_outside, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereTouchingFace, contact));

    // Edge cases
    Sphere sphereTouching_edge(Vector3D(1_d + 1_d / std::sqrt(2.0_d), 1_d + 1_d / std::sqrt(2.0_d), 1_d),
                               2_d);
    Sphere sphereTouchingCorner(
        Vector3D(1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d)),
        2_d);

    EXPECT_TRUE(aabb.computeCollision(sphereTouching_edge, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereTouchingCorner, contact));

    // Special cases
    Sphere sphereContainingAabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphere_zeroRadius(Vector3D(1_d, 1_d, 1_d), 0_d);

    decimal r      = 0.05_d;
    decimal offset = r / std::sqrt(3.0_d);
    Sphere  sphereAtCorner(Vector3D(1_d + offset, 1_d + offset, 1_d + offset), 0.1_d);

    EXPECT_TRUE(aabb.computeCollision(sphereContainingAabb, contact));
    EXPECT_TRUE(aabb.computeCollision(sphere_zeroRadius, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereAtCorner, contact));

    // Negative coordinate cases
    Sphere sphere_negative_outside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphere_negativeTouching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.computeCollision(sphere_negative_outside, contact));
    EXPECT_TRUE(aabb.computeCollision(sphere_negativeTouching, contact));
}

TEST(AABBTest, AABBPlaneCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Plane planeAbove(Vector3D(0_d, 0_d, 3_d));
    Plane planeBelow(Vector3D(0_d, 0_d, -3_d));
    Plane planeCenter(Vector3D(0_d, 0_d, 0_d));
    Plane planeLeftOut(Vector3D(-3_d, 0_d, 0_d));
    Plane planeLeftIn(Vector3D(-1.5_d, 0_d, 0_d));
    Plane planeRightOut(Vector3D(5_d, 0_d, 0_d));
    Plane planeRightIn(Vector3D(1.5_d, 0_d, 0_d));

    EXPECT_FALSE(aabb.computeCollision(planeAbove, contact));
    EXPECT_FALSE(aabb.computeCollision(planeBelow, contact));
    EXPECT_TRUE(aabb.computeCollision(planeCenter, contact));
    EXPECT_FALSE(aabb.computeCollision(planeLeftOut, contact));
    EXPECT_FALSE(aabb.computeCollision(planeRightOut, contact));
    EXPECT_TRUE(aabb.computeCollision(planeLeftIn, contact));
    EXPECT_TRUE(aabb.computeCollision(planeRightIn, contact));

    // Edge cases
    Plane planeUp(Vector3D(0_d, 0_d, 1_d));
    Plane planeDown(Vector3D(0_d, 0_d, -1_d));
    Plane planeLeft(Vector3D(-2_d, 0_d, 0_d));
    Plane planeRight(Vector3D(2_d, 0_d, 0_d));

    EXPECT_TRUE(aabb.computeCollision(planeUp, contact));
    EXPECT_TRUE(aabb.computeCollision(planeDown, contact));
    EXPECT_TRUE(aabb.computeCollision(planeLeft, contact));
    EXPECT_TRUE(aabb.computeCollision(planeRight, contact));
}

// Dummy class to simulate an unknown object type
struct DummyObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       computeCollision(const Object&, Contact& contact) override { return false; }
};

TEST(AABB, computeCollision_DefaultCase)
{
    AABB        box(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 1_d));
    DummyObject dummy;
    Contact     contact;

    bool result = box.computeCollision(dummy, contact);

    EXPECT_FALSE(result); // Default case should return false
}
