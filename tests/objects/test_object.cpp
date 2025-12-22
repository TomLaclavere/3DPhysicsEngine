#include "objects/object.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>

struct TestObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       checkCollision(const Object&) override { return false; }
};

TEST(ObjectTest, constructors)
{
    Vector3D position     = Vector3D(1_d, 0_d, -3_d);
    Vector3D rotation     = Vector3D(0_d, 0_d, -2.1_d);
    Vector3D size         = Vector3D(1.1_d, 0_d, 5_d);
    Vector3D velocity     = Vector3D(1.3_d, 3_d, -2_d);
    Vector3D acceleration = Vector3D(1_d, -1_d, 0_d);
    Vector3D force        = Vector3D(-1_d, 0_d, 0_d);
    Vector3D torque       = Vector3D(0_d, -3_d, 2_d);
    decimal  mass         = 2_d;

    Object object(mass);
    Object obj(position);
    Object obj_(position, size);
    Object obj_0(position, size, mass);
    Object obj_1(position, size, velocity, mass);
    Object obj_2(position, rotation, size, velocity, acceleration, force, torque, mass);
}

TEST(ObjectTest, setters)
{
    TestObject obj;

    obj.setPosition(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getPosition() == Vector3D(1_d, 2_d, -5_d));

    obj.setRotation(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getRotation() == Vector3D(1_d, 2_d, -5_d));

    obj.setSize(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getSize() == Vector3D(1_d, 2_d, -5_d));

    obj.setVelocity(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getVelocity() == Vector3D(1_d, 2_d, -5_d));

    obj.setAcceleration(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getAcceleration() == Vector3D(1_d, 2_d, -5_d));

    obj.setForce(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getForce() == Vector3D(1_d, 2_d, -5_d));

    obj.setTorque(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getTorque() == Vector3D(1_d, 2_d, -5_d));

    obj.setMass(100000000_d);
    EXPECT_TRUE(obj.getMass() == 100000000_d);
    EXPECT_TRUE(obj.getType() == ObjectType::Generic);

    obj.setIsFixed(true);
    EXPECT_TRUE(obj.getIsFixed());

    obj.setRestitutionCst(1_d);
    EXPECT_EQ(obj.getRestitutionCst(), 1_d);

    obj.setStiffnessCst(4_d);
    EXPECT_EQ(obj.getStiffnessCst(), 4_d);

    obj.setFrictionCst(0.5_d);
    EXPECT_EQ(obj.getFrictionCst(), 0.5_d);

    // Check is fixed if mass < 0
    obj.setMass(-1_d);
    obj.checkFixed();
    EXPECT_TRUE(obj.isFixed());

    // Check getType
    EXPECT_EQ(obj.getType(), ObjectType::Generic);
}
