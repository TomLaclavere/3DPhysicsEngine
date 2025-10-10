#include "objects/aabb.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>

TEST(SphereTest, ConstructorsAndGetters)
{
    Vector3D position(1_d, 2_d, 3_d);
    decimal  size(4_d);
    Vector3D rotation(0_d, 0_d, 0_d);
    Vector3D velocity(0.1_d, 0.2_d, 0.3_d);
    Vector3D acceleration(0.01_d, 0.02_d, 0.03_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;

    Sphere sphere(position, rotation, size, velocity, acceleration, force, torque, mass);

    EXPECT_EQ(sphere.getPosition(), position);
    EXPECT_EQ(sphere.getDiameter(), size);
    EXPECT_EQ(sphere.getRotation(), rotation);
    EXPECT_EQ(sphere.getVelocity(), velocity);
    EXPECT_EQ(sphere.getAcceleration(), acceleration);
    EXPECT_EQ(sphere.getForce(), force);
    EXPECT_EQ(sphere.getTorque(), torque);
    EXPECT_DECIMAL_EQ(sphere.getMass(), mass);
    EXPECT_EQ(sphere.getType(), ObjectType::Sphere);
}

TEST(SphereTest, Setters)
{
    Sphere sphere;

    Vector3D newPosition(4_d, 5_d, 6_d);
    Vector3D newVelocity(0.4_d, 0.5_d, 0.6_d);
    Vector3D newAcceleration(0.04_d, 0.05_d, 0.06_d);
    Vector3D newForce(0.004_d, 0.005_d, 0.006_d);
    Vector3D newTorque(0.0004_d, 0.0005_d, 0.0006_d);
    decimal  newMass = 20.0_d;

    sphere.setPosition(newPosition);
    sphere.setVelocity(newVelocity);
    sphere.setAcceleration(newAcceleration);
    sphere.setForce(newForce);
    sphere.setTorque(newTorque);
    sphere.setMass(newMass);

    EXPECT_EQ(sphere.getPosition(), newPosition);
    EXPECT_EQ(sphere.getVelocity(), newVelocity);
    EXPECT_EQ(sphere.getAcceleration(), newAcceleration);
    EXPECT_EQ(sphere.getForce(), newForce);
    EXPECT_EQ(sphere.getTorque(), newTorque);
    EXPECT_DECIMAL_EQ(sphere.getMass(), newMass);
}

TEST(SphereTest, Integrate)
{
    Sphere sphere(Vector3D(0_d, 0_d, 0_d), Vector3D(), 1_d, Vector3D(1_d, 0_d, 0_d),
                  Vector3D(0_d, -9.81_d, 0_d), Vector3D(), Vector3D(), 1.0_d);

    decimal timeStep = 1.0_d;
    sphere.integrate(timeStep);

    // Use approximate equality for floating-point comparisons
    EXPECT_DECIMAL_EQ(sphere.getPosition()[0], 1_d);
    EXPECT_DECIMAL_EQ(sphere.getPosition()[1], -9.81_d);
    EXPECT_DECIMAL_EQ(sphere.getPosition()[2], 0_d);

    EXPECT_DECIMAL_EQ(sphere.getVelocity()[0], 1_d);
    EXPECT_DECIMAL_EQ(sphere.getVelocity()[1], -9.81_d);
    EXPECT_DECIMAL_EQ(sphere.getVelocity()[2], 0_d);
}

TEST(SphereTest, SphereCollision)
{
    Sphere sphere(Vector3D(0_d, 0_d, 0_d), 2_d);
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphere_outside(Vector3D(5_d, 5_d, 5_d), 2_d);
    Sphere sphereTouching(Vector3D(2_d, 0_d, 0_d), 2_d);

    EXPECT_TRUE(sphere.checkCollision(sphere_inside));   // Overlapping
    EXPECT_FALSE(sphere.checkCollision(sphere_outside)); // Not overlapping
    EXPECT_TRUE(sphere.checkCollision(sphereTouching));  // Just touching
}

TEST(SphereTest, AABBSphereCollision)
{
    AABB   aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(4_d, 2_d, 2_d));
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 0.5_d);  // Center inside AABB
    Sphere sphere_outside(Vector3D(3_d, 3_d, 3_d), 0.5_d); // Completely outside
    Sphere sphereTouching(Vector3D(2.5_d, 1_d, 1_d), 1_d); // Touching right face

    EXPECT_TRUE(sphere_inside.checkCollision(aabb));   // Sphere inside AABB
    EXPECT_FALSE(sphere_outside.checkCollision(aabb)); // Sphere outside AABB
    EXPECT_TRUE(sphereTouching.checkCollision(aabb));  // Sphere touching AABB
}
