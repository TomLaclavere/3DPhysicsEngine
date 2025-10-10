#include "objects/aabb.hpp"
#include "objects/object.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>

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

    EXPECT_EQ(plane.getU(), Vector3D(0_d, 1_d, 0_d));
    EXPECT_EQ(plane.getV(), Vector3D(-1_d, 0_d, 0_d));
}

TEST(planeTest, Setters)
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

TEST(planeTest, planeCollision)
{
    Plane plane1(Vector3D(0_d, 0_d, 0_d), Vector3D(10_d, 10_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    Plane plane2(Vector3D(0_d, 0_d, 0.5_d), Vector3D(10_d, 10_d, 0_d), Vector3D(0_d, 1_d, 0_d));
    Plane plane3(Vector3D(3_d, 3_d, 3_d));

    EXPECT_TRUE(plane1.checkCollision(plane2));  // Overlapping
    EXPECT_FALSE(plane1.checkCollision(plane3)); // Not overlapping
}

TEST(PlaneTest, PlaneSphereCollision)
{
    Plane plane(Vector3D(0_d, 0_d, 0_d), Vector3D(10_d, 10_d, 0_d), Vector3D(0_d, 0_d, 1_d));

    Sphere sphereInside(Vector3D(0_d, 0_d, 0_d), 0.5_d);
    Sphere sphereAbove(Vector3D(0_d, 0_d, 4_d), 1_d);
    Sphere sphereBelow(Vector3D(0_d, 0_d, -3_d), 1_d);

    EXPECT_TRUE(plane.checkCollision(sphereInside));
    EXPECT_FALSE(plane.checkCollision(sphereAbove));
    EXPECT_FALSE(plane.checkCollision(sphereBelow));

    Sphere sphereTouchingEdge(Vector3D(4_d, 0_d, 0_d), 1_d);
    Sphere sphereTouchingCorner(Vector3D(4_d, 4_d, 0_d), 1_d);

    EXPECT_TRUE(plane.checkCollision(sphereTouchingEdge));
    EXPECT_TRUE(plane.checkCollision(sphereTouchingCorner));

    // Special cases
    Sphere sphereContainingplane(Vector3D(0_d, 0_d, 0_d), 5_d);
    Sphere sphereZeroRadius(Vector3D(0_d, 0_d, 0_d), 0_d);

    EXPECT_TRUE(plane.checkCollision(sphereContainingplane));
    EXPECT_TRUE(plane.checkCollision(sphereZeroRadius));
}

TEST(planeTest, planeAABBCollision)
{
    AABB aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));

    // Basic cases
    Plane planeAbove(Vector3D(0_d, 0_d, 3_d));
    Plane planeBelow(Vector3D(0_d, 0_d, -3_d));
    Plane planeCenter(Vector3D(0_d, 0_d, 0_d));
    Plane planeLeftOut(Vector3D(-3_d, 0_d, 0_d));
    Plane planeLeftIn(Vector3D(-1.5_d, 0_d, 0_d));
    Plane planeRightOut(Vector3D(5_d, 0_d, 0_d));
    Plane planeRightIn(Vector3D(1.5_d, 0_d, 0_d));

    EXPECT_FALSE(planeAbove.checkCollision(aabb));
    EXPECT_FALSE(planeBelow.checkCollision(aabb));
    EXPECT_TRUE(planeCenter.checkCollision(aabb));
    EXPECT_FALSE(planeLeftOut.checkCollision(aabb));
    EXPECT_FALSE(planeRightOut.checkCollision(aabb));
    EXPECT_TRUE(planeLeftIn.checkCollision(aabb));
    EXPECT_TRUE(planeRightIn.checkCollision(aabb));

    // Edge cases
    Plane planeUp(Vector3D(0_d, 0_d, 1_d));
    Plane planeDown(Vector3D(0_d, 0_d, -1_d));
    Plane planeLeft(Vector3D(-2_d, 0_d, 0_d));
    Plane planeRight(Vector3D(2_d, 0_d, 0_d));

    EXPECT_TRUE(planeUp.checkCollision(aabb));
    EXPECT_TRUE(planeDown.checkCollision(aabb));
    EXPECT_TRUE(planeLeft.checkCollision(aabb));
    EXPECT_TRUE(planeRight.checkCollision(aabb));
}

// Dummy class to simulate an unknown object type
struct DummyObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       checkCollision(const Object&) override { return false; }
};

TEST(plane, CheckCollision_DefaultCase)
{
    Plane       plane(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 1_d));
    DummyObject dummy;

    bool result = plane.checkCollision(dummy);

    EXPECT_FALSE(result); // Default case should return false
}
