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

TEST(AABBTest, BroadAABBCollision)
{
    AABB aabb1(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb2(Vector3D(1_d, 1_d, 1_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb3(Vector3D(3_d, 3_d, 3_d), Vector3D(2_d, 2_d, 2_d));

    EXPECT_TRUE(aabb1.checkAABBCollision(aabb2));  // Overlapping
    EXPECT_FALSE(aabb1.checkAABBCollision(aabb3)); // Not overlapping

    // Test over others dimensions
    AABB aabbY(Vector3D(0_d, 10_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabbZ(Vector3D(0_d, 0_d, -10_d), Vector3D(2_d, 2_d, 2_d));
    EXPECT_FALSE(aabb1.checkAABBCollision(aabbY));
    EXPECT_FALSE(aabb1.checkAABBCollision(aabbZ));
}

TEST(AABBTest, NarrowAABBCollision)
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

TEST(AABBTest, BroadAABBSphereCollision)
{
    AABB aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));

    // Basic cases
    Sphere  sphereInside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere  sphereOutside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere  sphereTouchingFace(Vector3D(3_d, 1_d, 1_d), 4_d);
    Contact contact;

    EXPECT_TRUE(aabb.checkCollision(sphereInside));
    EXPECT_FALSE(aabb.checkCollision(sphereOutside));
    EXPECT_TRUE(aabb.checkCollision(sphereTouchingFace));

    // Edge cases
    Sphere sphereTouchingEdge(Vector3D(1_d + 1_d / std::sqrt(2.0_d), 1_d + 1_d / std::sqrt(2.0_d), 1_d), 2_d);
    Sphere sphereTouchingCorner(
        Vector3D(1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d)),
        2_d);

    EXPECT_TRUE(aabb.checkCollision(sphereTouchingEdge));
    EXPECT_TRUE(aabb.checkCollision(sphereTouchingCorner));

    // Special cases
    Sphere sphereContainingAabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphereZeroRadius(Vector3D(1_d, 1_d, 1_d), 0_d);

    decimal r      = 0.05_d;
    decimal offset = r / std::sqrt(3.0_d);
    Sphere  sphereAtCorner(Vector3D(1_d + offset, 1_d + offset, 1_d + offset), 0.1_d);

    EXPECT_TRUE(aabb.checkCollision(sphereContainingAabb));
    EXPECT_TRUE(aabb.checkCollision(sphereZeroRadius));
    EXPECT_TRUE(aabb.checkCollision(sphereAtCorner));

    // Negative coordinate cases
    Sphere sphereNegativeOutside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphereNegativeTouching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.checkCollision(sphereNegativeOutside));
    EXPECT_TRUE(aabb.checkCollision(sphereNegativeTouching));

    // Test collisions which would be false in narrow phase
    Sphere sphereClose(Vector3D(0_d, 0_d, 2_d), 0.95_d);
    EXPECT_TRUE(aabb.checkCollision(sphereClose));
    EXPECT_FALSE(aabb.computeCollision(sphereClose, contact));
}

TEST(AABBTest, NarrowAABBSphereCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Sphere sphereInside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere sphereOutside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere sphereTouchingFace(Vector3D(3_d, 1_d, 1_d), 4_d);

    EXPECT_TRUE(aabb.computeCollision(sphereInside, contact));
    EXPECT_FALSE(aabb.computeCollision(sphereOutside, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereTouchingFace, contact));

    // Edge cases
    Sphere sphereTouchingEdge(Vector3D(1_d + 1_d / std::sqrt(2.0_d), 1_d + 1_d / std::sqrt(2.0_d), 1_d), 2_d);
    Sphere sphereTouchingCorner(
        Vector3D(1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d)),
        2_d);

    EXPECT_TRUE(aabb.computeCollision(sphereTouchingEdge, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereTouchingCorner, contact));

    // Special cases
    Sphere sphereContainingAabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphereZeroRadius(Vector3D(1_d, 1_d, 1_d), 0_d);

    decimal r      = 0.05_d;
    decimal offset = r / std::sqrt(3.0_d);
    Sphere  sphereAtCorner(Vector3D(1_d + offset, 1_d + offset, 1_d + offset), 0.1_d);

    EXPECT_TRUE(aabb.computeCollision(sphereContainingAabb, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereZeroRadius, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereAtCorner, contact));

    // Negative coordinate cases
    Sphere sphereNegativeOutside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphereNegativeTouching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.computeCollision(sphereNegativeOutside, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereNegativeTouching, contact));
}

TEST(AABBTest, BroadAABBPlaneCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Plane planeAbove(Vector3D(0_d, 0_d, 3_d));
    Plane planeBelow(Vector3D(0_d, 0_d, -3_d));
    Plane planeCenter(Vector3D(0_d, 0_d, 0_d));
    Plane planeLeftIn(Vector3D(-1.5_d, 0_d, 0_d));
    Plane planeRightIn(Vector3D(1.5_d, 0_d, 0_d));

    EXPECT_FALSE(aabb.checkCollision(planeAbove));
    EXPECT_FALSE(aabb.checkCollision(planeBelow));
    EXPECT_TRUE(aabb.checkCollision(planeCenter));
    EXPECT_TRUE(aabb.checkCollision(planeLeftIn));
    EXPECT_TRUE(aabb.checkCollision(planeRightIn));

    // Edge cases
    Plane planeUp(Vector3D(0_d, 0_d, 1_d));
    Plane planeDown(Vector3D(0_d, 0_d, -1_d));
    Plane planeLeft(Vector3D(-2_d, 0_d, 0_d));
    Plane planeRight(Vector3D(2_d, 0_d, 0_d));

    EXPECT_TRUE(aabb.checkCollision(planeUp));
    EXPECT_TRUE(aabb.checkCollision(planeDown));
    EXPECT_TRUE(aabb.checkCollision(planeLeft));
    EXPECT_TRUE(aabb.checkCollision(planeRight));

    // Test collisions which would be false in narrow phase
    Plane planeClose(Vector3D(3_d, 0_d, 0_d));
    EXPECT_TRUE(aabb.checkCollision(planeClose));
    EXPECT_FALSE(aabb.computeCollision(planeClose, contact));
}

TEST(AABBTest, NarrowAABBPlaneCollision)
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
