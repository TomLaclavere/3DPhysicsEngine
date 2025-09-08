#include "objects/AABB.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

TEST(AABB_Test, ConstructorsAndGetters)
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

    EXPECT_EQ(aabb.get_position(), position);
    EXPECT_EQ(aabb.get_rotation(), rotation);
    EXPECT_EQ(aabb.get_scale(), scale);
    EXPECT_EQ(aabb.get_velocity(), velocity);
    EXPECT_EQ(aabb.get_acceleration(), acceleration);
    EXPECT_EQ(aabb.get_force(), force);
    EXPECT_EQ(aabb.get_torque(), torque);
    EXPECT_DECIMAL_EQ(aabb.get_mass(), mass);
    EXPECT_EQ(aabb.get_type(), ObjectType::AABB);

    EXPECT_EQ(aabb.get_min(), position - scale * 0.5_d);
    EXPECT_EQ(aabb.get_max(), position + scale * 0.5_d);
}

TEST(AABB_Test, Setters)
{
    AABB aabb;

    Vector3D newPosition(4_d, 5_d, 6_d);
    Vector3D newScale(7_d, 8_d, 9_d);
    Vector3D newVelocity(0.4_d, 0.5_d, 0.6_d);
    Vector3D newAcceleration(0.04_d, 0.05_d, 0.06_d);
    Vector3D newForce(0.004_d, 0.005_d, 0.006_d);
    Vector3D newTorque(0.0004_d, 0.0005_d, 0.0006_d);
    decimal  newMass = 20.0_d;

    aabb.set_position(newPosition);
    aabb.set_scale(newScale);
    aabb.set_velocity(newVelocity);
    aabb.set_acceleration(newAcceleration);
    aabb.set_force(newForce);
    aabb.set_torque(newTorque);
    aabb.set_mass(newMass);

    EXPECT_EQ(aabb.get_position(), newPosition);
    EXPECT_EQ(aabb.get_scale(), newScale);
    EXPECT_EQ(aabb.get_velocity(), newVelocity);
    EXPECT_EQ(aabb.get_acceleration(), newAcceleration);
    EXPECT_EQ(aabb.get_force(), newForce);
    EXPECT_EQ(aabb.get_torque(), newTorque);
    EXPECT_DECIMAL_EQ(aabb.get_mass(), newMass);
}

TEST(AABB_Test, integrate)
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

    EXPECT_EQ(aabb.get_position(), Vector3D(7_d, 22_d, -4_d));
    EXPECT_EQ(aabb.get_velocity(), Vector3D(3_d, 10_d, -3.5_d));
}

TEST(AABB_Test, AABB_Collision)
{
    AABB aabb1(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb2(Vector3D(1_d, 1_d, 1_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb3(Vector3D(3_d, 3_d, 3_d), Vector3D(2_d, 2_d, 2_d));

    EXPECT_TRUE(aabb1.check_collision(aabb2));  // Overlapping
    EXPECT_FALSE(aabb1.check_collision(aabb3)); // Not overlapping
}

TEST(AABB_Test, AABB_Sphere_Collision)
{
    AABB aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));

    // Basic cases
    Sphere sphere_inside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere sphere_outside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere sphere_touching_face(Vector3D(3_d, 1_d, 1_d), 1_d);

    EXPECT_TRUE(aabb.check_collision(sphere_inside));
    EXPECT_FALSE(aabb.check_collision(sphere_outside));
    EXPECT_TRUE(aabb.check_collision(sphere_touching_face));

    // Edge cases
    Sphere sphere_touching_edge(Vector3D(3_d, 3_d, 1_d), 1.4143_d);   // sqrt(2) ≈ 1.4142
    Sphere sphere_touching_corner(Vector3D(3_d, 3_d, 3_d), 1.7321_d); // sqrt(3) ≈ 1.7320

    EXPECT_TRUE(aabb.check_collision(sphere_touching_edge));
    EXPECT_TRUE(aabb.check_collision(sphere_touching_corner));

    // Special cases
    Sphere sphere_containing_aabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphere_zero_radius(Vector3D(1_d, 1_d, 1_d), 0_d);
    Sphere sphere_at_corner(Vector3D(2_d, 2_d, 2_d), 0.1_d);

    EXPECT_TRUE(aabb.check_collision(sphere_containing_aabb));
    EXPECT_TRUE(aabb.check_collision(sphere_zero_radius));
    EXPECT_TRUE(aabb.check_collision(sphere_at_corner));

    // Negative coordinate cases
    Sphere sphere_negative_outside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphere_negative_touching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.check_collision(sphere_negative_outside));
    EXPECT_TRUE(aabb.check_collision(sphere_negative_touching));
}
