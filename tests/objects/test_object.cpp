#include "objects/object.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>

struct TestObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       checkCollision(const Object&) override { return false; }
};

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
}
