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
