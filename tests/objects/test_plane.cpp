#include "objects/aabb.hpp"
#include "objects/object.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>

// ——————————————————————————————————————————————————————————————————————————
//  Constructors, Getters, and Setters
// ——————————————————————————————————————————————————————————————————————————

TEST(PlaneTest, ConstructorsAndGetters)
{
    Vector3D position(1_d, 2_d, 3_d);
    Vector3D rotation(0_d, 0_d, 0_d);
    Vector3D size(4_d, 5_d, 6_d);
    Vector3D velocity(0.1_d, 0.2_d, 0.3_d);
    Vector3D acceleration(0.01_d, 0.02_d, 0.03_d);
    Vector3D force(0.001_d, 0.002_d, 0.003_d);
    Vector3D torque(0.0001_d, 0.0002_d, 0.0003_d);
    decimal  mass = 10.0_d;
    Vector3D normal(0_d, 0_d, 1_d);

    Plane plane(position, rotation, size, velocity, acceleration, force, torque, mass, normal);

    EXPECT_EQ(plane.getPosition(), position);
    EXPECT_EQ(plane.getRotation(), rotation);
    EXPECT_EQ(plane.getSize(), size);
    EXPECT_EQ(plane.getVelocity(), velocity);
    EXPECT_EQ(plane.getAcceleration(), acceleration);
    EXPECT_EQ(plane.getForce(), force);
    EXPECT_EQ(plane.getTorque(), torque);
    EXPECT_DECIMAL_EQ(plane.getMass(), mass);
    EXPECT_EQ(plane.getType(), ObjectType::Plane);

    EXPECT_EQ(plane.getU(), Vector3D(1_d, 0_d, 0_d));
    EXPECT_EQ(plane.getV(), Vector3D(0_d, 1_d, 0_d));
}

TEST(PlaneTest, Setters)
{
    Plane plane;

    Vector3D newPosition(4_d, 5_d, 6_d);
    decimal  newHalfHeight(7_d);
    decimal  newHalfWidth(-2_d);
    Vector3D newVelocity(0.4_d, 0.5_d, 0.6_d);
    Vector3D newAcceleration(0.04_d, 0.05_d, 0.06_d);
    Vector3D newForce(0.004_d, 0.005_d, 0.006_d);
    Vector3D newTorque(0.0004_d, 0.0005_d, 0.0006_d);
    decimal  newMass = 20.0_d;

    plane.setPosition(newPosition);
    plane.setSize(newHalfWidth, newHalfHeight);
    plane.setVelocity(newVelocity);
    plane.setAcceleration(newAcceleration);
    plane.setForce(newForce);
    plane.setTorque(newTorque);
    plane.setMass(newMass);

    EXPECT_EQ(plane.getPosition(), newPosition);
    EXPECT_EQ(plane.getHalfHeight(), newHalfHeight);
    EXPECT_EQ(plane.getHalfWidth(), newHalfWidth);
    EXPECT_EQ(plane.getVelocity(), newVelocity);
    EXPECT_EQ(plane.getAcceleration(), newAcceleration);
    EXPECT_EQ(plane.getForce(), newForce);
    EXPECT_EQ(plane.getTorque(), newTorque);
    EXPECT_DECIMAL_EQ(plane.getMass(), newMass);
}

// ——————————————————————————————————————————————————————————————————————————
//  Utilities
// ——————————————————————————————————————————————————————————————————————————

TEST(PlaneTest, updateLocalAxes)
{
    // Normal along Z
    Plane           p1(Vector3D(0_d, 0_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    const Vector3D& n1 = p1.getNormal();
    const Vector3D& u1 = p1.getU();
    const Vector3D& v1 = p1.getV();

    // Orthogonality
    EXPECT_DECIMAL_EQ(n1.dotProduct(u1), 0_d);
    EXPECT_DECIMAL_EQ(n1.dotProduct(v1), 0_d);
    EXPECT_DECIMAL_EQ(u1.dotProduct(v1), 0_d);

    // Normalisation
    EXPECT_DECIMAL_EQ(n1.getNorm(), 1_d);
    EXPECT_DECIMAL_EQ(u1.getNorm(), 1_d);
    EXPECT_DECIMAL_EQ(v1.getNorm(), 1_d);

    // Now try a tilted normal
    Plane           p2(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 0_d).getNormalised());
    const Vector3D& n2 = p2.getNormal();
    const Vector3D& u2 = p2.getU();
    const Vector3D& v2 = p2.getV();

    EXPECT_DECIMAL_EQ(n2.dotProduct(u2), 0_d);
    EXPECT_DECIMAL_EQ(n2.dotProduct(v2), 0_d);
    EXPECT_DECIMAL_EQ(u2.dotProduct(v2), 0_d);
}

TEST(PlaneTest, ProjectPoint)
{
    Plane p(Vector3D(0_d, 0_d, 0_d), Vector3D(0_d, 0_d, 1_d));

    // Point above plane (z = 5)
    Vector3D pt(1_d, 2_d, 5_d);
    Vector3D proj = p.projectPoint(pt);

    // Projection should be (1,2,0)
    EXPECT_DECIMAL_EQ(proj[0], 1_d);
    EXPECT_DECIMAL_EQ(proj[1], 2_d);
    EXPECT_DECIMAL_EQ(proj[2], 0_d);

    // Distance between point and projection equals height (5)
    EXPECT_DECIMAL_EQ((pt - proj).getNorm(), 5_d);
}

TEST(PlaneTest, ContainsPoint)
{
    Plane p(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 4_d, 0_d), Vector3D(0_d, 0_d, 1_d));

    // Inside bounds
    EXPECT_TRUE(p.containsPoint(Vector3D(0.5_d, 0.5_d, 0_d)));

    // On the edge
    EXPECT_TRUE(p.containsPoint(Vector3D(1_d, 0_d, 0_d)));

    // Outside bounds
    EXPECT_FALSE(p.containsPoint(Vector3D(2.1_d, 0_d, 0_d)));
    EXPECT_FALSE(p.containsPoint(Vector3D(0_d, -2.1_d, 0_d)));
}
