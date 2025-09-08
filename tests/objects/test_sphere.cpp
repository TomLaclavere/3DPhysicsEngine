#include "objects/AABB.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

TEST(Sphere_Test, ConstructorsAndGetters)
{
    Vector3D position(1_d, 2_d, 3_d);
    decimal  scale(4_d);
    Vector3D rotation(0_d, 0_d, 0_d);
    Vector3D velocity(0.1_d, 0.2_d, 0.3_d);
    Vector3D acceleration(0.01_d, 0.02_d, 0.03_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;

    Sphere sphere(position, rotation, scale, velocity, acceleration, force, torque, mass);

    EXPECT_EQ(sphere.get_position(), position);
    EXPECT_EQ(sphere.get_scale(), scale);
    EXPECT_EQ(sphere.get_rotation(), rotation);
    EXPECT_EQ(sphere.get_velocity(), velocity);
    EXPECT_EQ(sphere.get_acceleration(), acceleration);
    EXPECT_EQ(sphere.get_force(), force);
    EXPECT_EQ(sphere.get_torque(), torque);
    EXPECT_DECIMAL_EQ(sphere.get_mass(), mass);
    EXPECT_EQ(sphere.get_type(), ObjectType::Sphere);
}

TEST(Sphere_Test, Setters)
{
    Sphere sphere;

    Vector3D newPosition(4_d, 5_d, 6_d);
    Vector3D newVelocity(0.4_d, 0.5_d, 0.6_d);
    Vector3D newAcceleration(0.04_d, 0.05_d, 0.06_d);
    Vector3D newForce(0.004_d, 0.005_d, 0.006_d);
    Vector3D newTorque(0.0004_d, 0.0005_d, 0.0006_d);
    decimal  newMass = 20.0_d;

    sphere.set_position(newPosition);
    sphere.set_velocity(newVelocity);
    sphere.set_acceleration(newAcceleration);
    sphere.set_force(newForce);
    sphere.set_torque(newTorque);
    sphere.set_mass(newMass);

    EXPECT_EQ(sphere.get_position(), newPosition);
    EXPECT_EQ(sphere.get_velocity(), newVelocity);
    EXPECT_EQ(sphere.get_acceleration(), newAcceleration);
    EXPECT_EQ(sphere.get_force(), newForce);
    EXPECT_EQ(sphere.get_torque(), newTorque);
    EXPECT_DECIMAL_EQ(sphere.get_mass(), newMass);
}

TEST(Sphere_Test, Integrate)
{
    Sphere sphere(Vector3D(0_d, 0_d, 0_d), Vector3D(), 1_d, Vector3D(1_d, 0_d, 0_d),
                  Vector3D(0_d, -9.81_d, 0_d), Vector3D(), Vector3D(), 1.0_d);

    decimal timeStep = 1.0_d;
    sphere.integrate(timeStep);

    // Use approximate equality for floating-point comparisons
    EXPECT_DECIMAL_EQ(sphere.get_position()[0], 1_d);
    EXPECT_DECIMAL_EQ(sphere.get_position()[1], -9.81_d);
    EXPECT_DECIMAL_EQ(sphere.get_position()[2], 0_d);

    EXPECT_DECIMAL_EQ(sphere.get_velocity()[0], 1_d);
    EXPECT_DECIMAL_EQ(sphere.get_velocity()[1], -9.81_d);
    EXPECT_DECIMAL_EQ(sphere.get_velocity()[2], 0_d);
}

TEST(Sphere_Test, Sphere_Collision)
{
    Sphere sphere(Vector3D(0_d, 0_d, 0_d), 2_d);
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphere_outside(Vector3D(5_d, 5_d, 5_d), 2_d);
    Sphere sphere_touching(Vector3D(2_d, 0_d, 0_d), 2_d);

    EXPECT_TRUE(sphere.check_collision(sphere_inside));   // Overlapping
    EXPECT_FALSE(sphere.check_collision(sphere_outside)); // Not overlapping
    EXPECT_TRUE(sphere.check_collision(sphere_touching)); // Just touching
}

TEST(Sphere_Test, AABB_Sphere_Collision)
{
    AABB   aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(4_d, 2_d, 2_d));
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 0.5_d);   // Center inside AABB
    Sphere sphere_outside(Vector3D(3_d, 3_d, 3_d), 0.5_d);  // Completely outside
    Sphere sphere_touching(Vector3D(2.5_d, 1_d, 1_d), 1_d); // Touching right face

    EXPECT_TRUE(sphere_inside.check_collision(aabb));   // Sphere inside AABB
    EXPECT_FALSE(sphere_outside.check_collision(aabb)); // Sphere outside AABB
    EXPECT_TRUE(sphere_touching.check_collision(aabb)); // Sphere touching AABB
}
