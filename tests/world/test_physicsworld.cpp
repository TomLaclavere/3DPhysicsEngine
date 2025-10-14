#include "objects/object.hpp"
#include "test_functions.hpp"
#include "world/physicsWorld.hpp"

#include <gtest/gtest.h>

// Dummy Object implementation for testing
struct DummyObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       checkCollision(const Object&) override { return false; }
};

// ============================================================================
//  Fixture
// ============================================================================
class PhysicsWorldTest : public ::testing::Test
{
protected:
    PhysicsWorld world;
    DummyObject  obj1;
    DummyObject  obj2;

    void SetUp() override
    {
        Config::get();
        world.clearObjects();
        obj1.setIsFixed(false);
        obj2.setIsFixed(true);
        world.addObject(&obj1);
        world.addObject(&obj2);
        world.addObject(nullptr); // cover null branch
    }
};

// ============================================================================
//  Tests
// ============================================================================

TEST_F(PhysicsWorldTest, InitializeSetsConfigValues)
{
    Config::get();
    char  arg0[] = "prog";
    char  arg1[] = "--gravity";
    char  arg2[] = "12.0";
    char  arg3[] = "--timestep";
    char  arg4[] = "0.05";
    char  arg5[] = "--iters";
    char  arg6[] = "5";
    char* argv[] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6 };
    int   argc   = 7;

    Config::get().overrideFromCommandLine(argc, argv);
    world.initialize();

    EXPECT_FALSE(world.getIsRunning());
    EXPECT_DECIMAL_EQ(world.getObjectCount(), 0);
    EXPECT_DECIMAL_EQ(world.getTimeStep(), 0.05_d);
    EXPECT_DECIMAL_EQ(world.getGravityCst(), 12.0_d);
    EXPECT_VECTOR_EQ(world.getGravityAcc(), Vector3D(0_d, 0_d, -12.0_d));
}

TEST_F(PhysicsWorldTest, ApplyGravityAffectsAcceleration)
{
    world.start();
    world.applyGravityForces();
    EXPECT_VECTOR_EQ(obj1.getAcceleration(), Vector3D(0_d, 0_d, -9.81_d));
    EXPECT_VECTOR_EQ(obj2.getAcceleration(), Vector3D(0_d, 0_d, 0_d));

    world.resetAcc();
    world.setGravityCst(5_d);
    world.setGravityAcc(Physics::computeGravityAcc(world.getGravityCst()));
    world.applyForces();
    EXPECT_VECTOR_EQ(obj1.getAcceleration(), Vector3D(0_d, 0_d, -5_d));
    EXPECT_VECTOR_EQ(obj2.getAcceleration(), Vector3D(0_d, 0_d, 0_d));
}

TEST_F(PhysicsWorldTest, UpdateDoesNothingWhenNotRunning)
{
    world.stop();
    obj1.setVelocity(Vector3D(1_d, 0_d, 0_d));
    obj1.setAcceleration(Vector3D(0_d, 1_d, 0_d));
    world.integrate(0.1_d);
    EXPECT_VECTOR_EQ(obj1.getPosition(), Vector3D(0_d, 0_d, 0_d)); // unchanged
}

TEST_F(PhysicsWorldTest, IntegrateEulerUpdatesObjects)
{
    world.resetAcc();
    world.start();
    world.setTimeStep(0.1_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    obj1.setVelocity(Vector3D(0_d, 0_d, 0_d));
    obj1.setPosition(Vector3D(0_d, 0_d, 10_d));
    obj1.setAcceleration(Vector3D(0_d, 0_d, -9.81_d));

    world.integrateEuler(obj1, world.getTimeStep());
    world.integrateEuler(obj2, world.getTimeStep());

    // v = v + a * dt = (0,0,-0.981)
    // p = p + v * dt = (0,0,9.9019)
    EXPECT_DECIMAL_EQ(obj1.getVelocity().getZ(), -0.981_d);
    EXPECT_DECIMAL_EQ(obj1.getPosition().getZ(), 9.9019_d);
    EXPECT_DECIMAL_EQ(obj1.getAcceleration().getZ(), -9.81_d);
    EXPECT_DECIMAL_EQ(obj2.getVelocity().getZ(), 0_d);
    EXPECT_DECIMAL_EQ(obj2.getPosition().getZ(), 0_d);
    EXPECT_DECIMAL_EQ(obj2.getAcceleration().getZ(), 0_d);
}

TEST_F(PhysicsWorldTest, IntegrateEulerSkipsNullObjects)
{
    world.resetAcc();
    world.addObject(nullptr); // explicit null coverage
    world.integrate(world.getTimeStep());

    SUCCEED();
}
