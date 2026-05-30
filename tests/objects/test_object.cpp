#include "objects/object.hpp"
#include "test_functions.hpp"

#include <fstream>
#include <gtest/gtest.h>
#include <numbers>
#include <string>

struct TestObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       checkCollision(const Object&) override { return false; }
};

TEST(ObjectTest, constructors)
{
    Vector3D position     = Vector3D(1_d, 0_d, -3_d);
    Vector3D normal       = Vector3D(0_d, 0_d, -2.1_d);
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
    Object obj_2(position, normal, size, velocity, acceleration, force, torque, mass);
}

TEST(ObjectTest, setters)
{
    TestObject obj;

    obj.setPosition(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getPosition() == Vector3D(1_d, 2_d, -5_d));

    obj.setNormal(Vector3D(1_d, 2_d, -5_d));
    EXPECT_TRUE(obj.getNormal() == Vector3D(1_d, 2_d, -5_d));

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

    // Check is fixed if mass < 0
    obj.setMass(-1_d);
    obj.checkFixed();
    EXPECT_TRUE(obj.isFixed());

    // Check getType
    EXPECT_EQ(obj.getType(), ObjectType::Generic);

    // ID
    obj.setId(3);
    EXPECT_EQ(obj.getId(), 3);
}

TEST(ObjectTest, ForceAndAcceleration)
{
    TestObject obj;
    obj.setMass(2_d);

    // applyForce accumulates
    obj.applyForce(Vector3D(1_d, 0_d, 0_d));
    obj.applyForce(Vector3D(0_d, 2_d, 0_d));
    EXPECT_VECTOR_EQ(obj.getForce(), Vector3D(1_d, 2_d, 0_d));

    // applyTorque accumulates
    obj.applyTorque(Vector3D(0_d, 0_d, 3_d));
    obj.applyTorque(Vector3D(1_d, 0_d, 0_d));
    EXPECT_VECTOR_EQ(obj.getTorque(), Vector3D(1_d, 0_d, 3_d));

    // resetForces zeroes force and torque
    obj.resetForces();
    EXPECT_VECTOR_EQ(obj.getForce(), Vector3D(0_d));
    EXPECT_VECTOR_EQ(obj.getTorque(), Vector3D(0_d));

    // addAcceleration accumulates
    obj.setAcceleration(Vector3D(0_d));
    obj.addAcceleration(Vector3D(1_d, 0_d, 0_d));
    obj.addAcceleration(Vector3D(0_d, 2_d, 0_d));
    EXPECT_VECTOR_EQ(obj.getAcceleration(), Vector3D(1_d, 2_d, 0_d));
}

TEST(ObjectTest, MaterialAndEnergy)
{
    TestObject obj;
    obj.setMass(2_d);
    obj.setPosition(Vector3D(0_d, 0_d, 10_d));
    obj.setVelocity(Vector3D(3_d, 4_d, 0_d));

    // Material constants
    obj.setStiffnessCst(500_d);
    EXPECT_DECIMAL_EQ(obj.getStiffnessCst(), 500_d);

    obj.setDampingCst(10_d);
    EXPECT_DECIMAL_EQ(obj.getDampingCst(), 10_d);

    obj.setFrictionCst(0.3_d);
    EXPECT_DECIMAL_EQ(obj.getFrictionCst(), 0.3_d);

    obj.setRestitutionCst(0.8_d);
    EXPECT_DECIMAL_EQ(obj.getRestitutionCst(), 0.8_d);

    // setMaterial round-trip
    Material mat("steel", 7800_d, 200000_d, 1000_d, 0.3_d, 0.5_d);
    obj.setMaterial(mat);
    EXPECT_EQ(obj.getMaterial().getName(), "steel");
    EXPECT_DECIMAL_EQ(obj.getStiffnessCst(), 200000_d);

    // getName / setName
    obj.setName("sphere_0");
    EXPECT_EQ(obj.getName(), "sphere_0");

    // Energy: mass=2, vel=(3,4,0) → KE = 0.5*2*(9+16) = 25
    obj.setMass(2_d);
    obj.setVelocity(Vector3D(3_d, 4_d, 0_d));
    EXPECT_DECIMAL_EQ(obj.getKineticEnergy(), 25_d);

    // PE = mass * g * z = 2 * 9.81 * 10
    EXPECT_DECIMAL_EQ(obj.getPotentielEnergy(9.81_d), 2_d * 9.81_d * 10_d);

    // Total energy
    EXPECT_DECIMAL_EQ(obj.getTotalEnergy(9.81_d), obj.getKineticEnergy() + obj.getPotentielEnergy(9.81_d));
}

TEST(ObjectTest, BaseVolumeAndType)
{
    Object obj;
    obj.setSize(Vector3D(1_d, 1_d, 1_d));
    // Base getVolume = size.getNorm() * pi
    decimal expected = Vector3D(1_d, 1_d, 1_d).getNorm() * std::numbers::pi_v<decimal>;
    EXPECT_DECIMAL_EQ(obj.getVolume(), expected);

    // Object::getType() returns Generic when not overridden
    EXPECT_EQ(obj.getType(), ObjectType::Generic);
}

TEST(ObjectTest, CsvMethods)
{
    TestObject obj;
    obj.setMass(2_d);
    obj.setPosition(Vector3D(1_d, 2_d, 3_d));
    obj.setVelocity(Vector3D(0.1_d, 0.2_d, 0.3_d));
    obj.setAcceleration(Vector3D(0.01_d, 0.02_d, 0.03_d));
    obj.setName("test_obj");
    obj.setId(0);

    const std::string motionPath = "test_motion_obj.csv";
    const std::string objectPath = "test_object_obj.csv";

    // initMotionCSV writes a header line
    {
        std::ofstream f(motionPath);
        obj.initMotionCSV(f);
        EXPECT_TRUE(f.good());
    }
    std::remove(motionPath.c_str());

    // saveObjectCSV writes object properties
    {
        std::ofstream f(objectPath);
        EXPECT_TRUE(obj.saveObjectCSV(f));
    }
    std::remove(objectPath.c_str());

    // saveMotionCSV writes kinematics at a given time
    {
        std::ofstream f(motionPath);
        EXPECT_TRUE(obj.saveMotionCSV(f, 0.5_d));
    }
    std::remove(motionPath.c_str());

    // saveObjectCSV / saveMotionCSV with failbit set returns false
    {
        std::ofstream bad;
        bad.setstate(std::ios::failbit);
        EXPECT_FALSE(obj.saveObjectCSV(bad));
        EXPECT_FALSE(obj.saveMotionCSV(bad, 0.0_d));
    }
}

TEST(ObjectTest, Transforms)
{
    TestObject obj;
    obj.setPosition(Vector3D(1_d, 2_d, 3_d));
    obj.setSize(Vector3D(2_d, 2_d, 2_d));

    // applyTranslation shifts position
    obj.applyTranslation(Vector3D(1_d, 0_d, -1_d));
    EXPECT_VECTOR_EQ(obj.getPosition(), Vector3D(2_d, 2_d, 2_d));

    // applyScaling scales size element-wise
    obj.applyScaling(Vector3D(2_d, 0.5_d, 3_d));
    EXPECT_VECTOR_EQ(obj.getSize(), Vector3D(4_d, 1_d, 6_d));

    // addAcceleration from zero
    obj.setAcceleration(Vector3D(1_d, 0_d, 0_d));
    obj.addAcceleration(Vector3D(0_d, 0_d, -9.81_d));
    EXPECT_VECTOR_EQ(obj.getAcceleration(), Vector3D(1_d, 0_d, -9.81_d));
}
