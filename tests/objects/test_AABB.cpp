#include "objects/AABB.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

TEST(AABBTest, ConstructorsAndGetters)
{
    Vector3D position(1_d, 2_d, 3_d);
    Vector3D rotation(0_d, 0_d, 0_d);
    Vector3D scale(4_d, 5_d, 6_d);
    Vector3D velocity(0.1_d, 0.2_d, 0.3_d);
    Vector3D acceleration(0.01_d, 0.02_d, 0.03_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;

    AABB aabb(position, rotation, scale, velocity, acceleration, force, torque, mass);

    EXPECT_EQ(aabb.getPosition(), position);
    EXPECT_EQ(aabb.getRotation(), rotation);
    EXPECT_EQ(aabb.getScale(), scale);
    EXPECT_EQ(aabb.getVelocity(), velocity);
    EXPECT_EQ(aabb.getAcceleration(), acceleration);
    EXPECT_EQ(aabb.getForce(), force);
    EXPECT_EQ(aabb.getTorque(), torque);
    EXPECT_DECIMAL_EQ(aabb.getMass(), mass);
    EXPECT_EQ(aabb.getType(), ObjectType::AABB);

    EXPECT_EQ(aabb.getMin(), position - scale * 0.5_d);
    EXPECT_EQ(aabb.getMax(), position + scale * 0.5_d);
}

TEST(AABBTest, Setters)
{
    AABB aabb;

    Vector3D newPosition(4_d, 5_d, 6_d);
    Vector3D newScale(7_d, 8_d, 9_d);
    Vector3D newVelocity(0.4_d, 0.5_d, 0.6_d);
    Vector3D newAcceleration(0.04_d, 0.05_d, 0.06_d);
    Vector3D newForce(0.004_d, 0.005_d, 0.006_d);
    Vector3D newTorque(0.0004_d, 0.0005_d, 0.0006_d);
    decimal  newMass = 20.0_d;

    aabb.setPosition(newPosition);
    aabb.setScale(newScale);
    aabb.setVelocity(newVelocity);
    aabb.setAcceleration(newAcceleration);
    aabb.setForce(newForce);
    aabb.setTorque(newTorque);
    aabb.setMass(newMass);

    EXPECT_EQ(aabb.getPosition(), newPosition);
    EXPECT_EQ(aabb.getScale(), newScale);
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
    Vector3D scale(4_d, 5_d, 6_d);
    Vector3D velocity(-1_d, 0_d, 0.5_d);
    Vector3D acceleration(2_d, 5_d, -2_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;

    AABB    aabb(position, rotation, scale, velocity, acceleration, force, torque, mass);
    decimal dt = 2.0_d;

    aabb.integrate(dt);

    EXPECT_EQ(aabb.getPosition(), Vector3D(7_d, 22_d, -4_d));
    EXPECT_EQ(aabb.getVelocity(), Vector3D(3_d, 10_d, -3.5_d));
}

TEST(AABBTest, AABBCollision)
{
    AABB aabb1(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb2(Vector3D(1_d, 1_d, 1_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb3(Vector3D(3_d, 3_d, 3_d), Vector3D(2_d, 2_d, 2_d));

    EXPECT_TRUE(aabb1.checkCollision(aabb2));  // Overlapping
    EXPECT_FALSE(aabb1.checkCollision(aabb3)); // Not overlapping
}

TEST(AABBTest, AABBSphereCollision)
{
    AABB aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));

    // Basic cases
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere sphere_outside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere sphereTouchingFace(Vector3D(3_d, 1_d, 1_d), 4_d);

    EXPECT_TRUE(aabb.checkCollision(sphere_inside));
    EXPECT_FALSE(aabb.checkCollision(sphere_outside));
    EXPECT_TRUE(aabb.checkCollision(sphereTouchingFace));

    // Edge cases
    Sphere sphereTouching_edge(Vector3D(1_d + 1_d / std::sqrt(2.0_d), 1_d + 1_d / std::sqrt(2.0_d), 1_d),
                               2_d);
    Sphere sphereTouchingCorner(
        Vector3D(1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d)),
        2_d);

    EXPECT_TRUE(aabb.checkCollision(sphereTouching_edge));
    EXPECT_TRUE(aabb.checkCollision(sphereTouchingCorner));

    // Special cases
    Sphere sphereContainingAabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphere_zeroRadius(Vector3D(1_d, 1_d, 1_d), 0_d);

    decimal r      = 0.05_d;
    decimal offset = r / std::sqrt(3.0_d);
    Sphere  sphereAtCorner(Vector3D(1_d + offset, 1_d + offset, 1_d + offset), 0.1_d);

    EXPECT_TRUE(aabb.checkCollision(sphereContainingAabb));
    EXPECT_TRUE(aabb.checkCollision(sphere_zeroRadius));
    EXPECT_TRUE(aabb.checkCollision(sphereAtCorner));

    // Negative coordinate cases
    Sphere sphere_negative_outside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphere_negativeTouching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.checkCollision(sphere_negative_outside));
    EXPECT_TRUE(aabb.checkCollision(sphere_negativeTouching));
}
