#include "objects/sphere.hpp"
#include "test_functions.hpp"
#include "world/physics.hpp"
#include "world/physicsWorld.hpp"
#include "world/solver.hpp"

#include <gtest/gtest.h>
#include <string>

// --------------------------------------------------------------------------
//  Setters and Getters
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, SettersAndGetters)
{
    PhysicsWorld world;

    world.setTimeStep(0.05_d);
    EXPECT_DECIMAL_EQ(world.getTimeStep(), 0.05_d);

    world.setGravityCst(5_d);
    EXPECT_DECIMAL_EQ(world.getGravityCst(), 5_d);

    world.setGravityAcc(Vector3D(0_d, 0_d, -5_d));
    EXPECT_VECTOR_EQ(world.getGravityAcc(), Vector3D(0_d, 0_d, -5_d));

    EXPECT_FALSE(world.getIsRunning());
    world.start();
    EXPECT_TRUE(world.getIsRunning());
    world.stop();
    EXPECT_FALSE(world.getIsRunning());

    world.setSolver("Euler");
    EXPECT_EQ(world.getSolver(), Solver::Euler);
    world.setSolver("Verlet");
    EXPECT_EQ(world.getSolver(), Solver::Verlet);
    world.setSolver("RK4");
    EXPECT_EQ(world.getSolver(), Solver::RK4);
    world.setSolver("unknown_solver");
    EXPECT_EQ(world.getSolver(), Solver::Unknown);
}

// --------------------------------------------------------------------------
//  Object management
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, ObjectManagement)
{
    PhysicsWorld world;

    EXPECT_EQ(world.getObjectCount(), 0u);

    // addObject increments ID counter and count
    Sphere* s1 = new Sphere(Vector3D(0_d), 1_d, 1_d);
    Sphere* s2 = new Sphere(Vector3D(1_d, 0_d, 0_d), 1_d, 2_d);
    world.addObject(s1);
    world.addObject(s2);

    EXPECT_EQ(world.getObjectCount(), 2u);
    EXPECT_EQ(world.getObject(0), s1);
    EXPECT_EQ(world.getObject(1), s2);
    EXPECT_EQ(world.getObject(99), nullptr); // out of range

    EXPECT_EQ(s1->getId(), 0u);
    EXPECT_EQ(s2->getId(), 1u);
    EXPECT_EQ(world.getNextObjectId(), 2u);

    // findById
    EXPECT_EQ(world.findById(0), s1);
    EXPECT_EQ(world.findById(1), s2);
    EXPECT_EQ(world.findById(99), nullptr);

    // clearObjects deletes and empties
    world.clearObjects();
    EXPECT_EQ(world.getObjectCount(), 0u);
}

// --------------------------------------------------------------------------
//  initialise
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, Initialise)
{
    PhysicsWorld world;
    world.start();

    world.initialise();

    EXPECT_FALSE(world.getIsRunning());
    EXPECT_EQ(world.getObjectCount(), 0u);
    // Values come from singleton Config defaults
    EXPECT_DECIMAL_EQ(world.getTimeStep(), Config::get().getTimeStep());
    EXPECT_DECIMAL_EQ(world.getGravityCst(), Config::get().getGravity());
    EXPECT_VECTOR_EQ(world.getGravityAcc(), Vector3D(0_d, 0_d, -Config::get().getGravity()));
}

// --------------------------------------------------------------------------
//  Gravity application
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, ApplyGravityForces)
{
    PhysicsWorld world;
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    // Non-fixed sphere (mass > 0)
    Sphere* moving = new Sphere(Vector3D(0_d), 1_d, 1_d);
    moving->setAcceleration(Vector3D(0_d));
    world.addObject(moving);

    // Fixed sphere (mass = 0 → fixed by default)
    Sphere* fixed = new Sphere(Vector3D(5_d, 0_d, 0_d));
    fixed->setAcceleration(Vector3D(0_d));
    world.addObject(fixed);

    world.applyGravityForces();

    // Non-fixed: gravity added to acceleration
    EXPECT_VECTOR_EQ(moving->getAcceleration(), Vector3D(0_d, 0_d, -9.81_d));
    // Fixed: acceleration unchanged
    EXPECT_VECTOR_EQ(fixed->getAcceleration(), Vector3D(0_d));

    world.clearObjects();
}

// --------------------------------------------------------------------------
//  Euler integrator
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateEuler)
{
    PhysicsWorld world;

    Sphere sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
    sphere.setVelocity(Vector3D(0_d));
    sphere.setAcceleration(Vector3D(0_d, 0_d, -9.81_d));

    const decimal dt = 0.1_d;
    world.integrateEuler(sphere, dt);

    // v = v0 + a*dt = (0, 0, -0.981)
    EXPECT_DECIMAL_EQ(sphere.getVelocity().getZ(), -0.981_d);
    // p = p0 + v_new*dt = (0, 0, 10 - 0.0981) = (0, 0, 9.9019)
    EXPECT_DECIMAL_EQ(sphere.getPosition().getZ(), 9.9019_d);

    // Fixed object must not move when called via the loop
    Sphere fixed(Vector3D(0_d, 0_d, 5_d));
    fixed.setAcceleration(Vector3D(0_d, 0_d, -9.81_d));
    // integrateEuler itself has no fixed check - the caller's loop skips fixed;
    // here we just verify the math on a second call
    world.integrateEuler(fixed, dt);
    EXPECT_DECIMAL_EQ(fixed.getVelocity().getZ(), -0.981_d);
}

// --------------------------------------------------------------------------
//  Verlet integrator
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateVerlet)
{
    PhysicsWorld world;
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
    s->setVelocity(Vector3D(0_d));
    world.addObject(s);

    world.integrateVerlet(0.1_d);

    EXPECT_LT(s->getPosition().getZ(), 10_d);
    EXPECT_LT(s->getVelocity().getZ(), 0_d);

    world.clearObjects();
}

// --------------------------------------------------------------------------
//  RK4 integrator
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateRK4)
{
    PhysicsWorld world;
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
    s->setVelocity(Vector3D(0_d));
    world.addObject(s);

    world.integrateRK4(0.1_d);

    EXPECT_LT(s->getPosition().getZ(), 10_d);
    EXPECT_LT(s->getVelocity().getZ(), 0_d);

    world.clearObjects();
}

// --------------------------------------------------------------------------
//  integrate() dispatch
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateDispatch)
{
    // integrate() does nothing when not running
    {
        PhysicsWorld world;
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        world.addObject(s);

        world.stop();
        world.integrate(); // should do nothing
        EXPECT_DECIMAL_EQ(s->getPosition().getZ(), 10_d);

        world.clearObjects();
    }

    // Euler: position updates after start + integrate
    {
        PhysicsWorld world;
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        s->setVelocity(Vector3D(0_d));
        s->setAcceleration(Vector3D(0_d));
        world.addObject(s);

        world.start();
        world.integrate();
        EXPECT_LT(s->getPosition().getZ(), 10_d);

        world.clearObjects();
    }

    // Verlet dispatch
    {
        PhysicsWorld world;
        world.setSolver("Verlet");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        s->setVelocity(Vector3D(0_d));
        world.addObject(s);

        world.start();
        world.integrate();
        EXPECT_LT(s->getPosition().getZ(), 10_d);

        world.clearObjects();
    }

    // RK4 dispatch
    {
        PhysicsWorld world;
        world.setSolver("RK4");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        s->setVelocity(Vector3D(0_d));
        world.addObject(s);

        world.start();
        world.integrate();
        EXPECT_LT(s->getPosition().getZ(), 10_d);

        world.clearObjects();
    }

    // Unknown solver: integrate() prints warning but does not crash
    {
        PhysicsWorld world;
        world.setSolver("unknown_solver");
        world.setTimeStep(0.1_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        world.addObject(s);

        world.start();
        EXPECT_NO_THROW(world.integrate());

        world.clearObjects();
    }

    // Non-simplified Euler: covers the force-based integrate() branch
    {
        PhysicsWorld world;
        Config::get().setSimplifiedCollision(false);
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        s->setVelocity(Vector3D(0_d));
        s->setAcceleration(Vector3D(0_d));
        world.addObject(s);

        world.start();
        world.integrate();
        EXPECT_LT(s->getPosition().getZ(), 10_d);

        world.clearObjects();
        Config::get().setSimplifiedCollision(true); // restore default
    }
}

// --------------------------------------------------------------------------
//  Fixed-object branches in Verlet and RK4 (cover the continue paths)
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateVerletWithFixed)
{
    PhysicsWorld world;
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    Sphere* moving = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
    Sphere* fixed  = new Sphere(Vector3D(5_d, 0_d, 0_d)); // mass=0 → isFixed
    moving->setVelocity(Vector3D(0_d));
    world.addObject(moving);
    world.addObject(fixed);

    Vector3D fixedPosBefore = fixed->getPosition();
    world.integrateVerlet(0.1_d);

    EXPECT_LT(moving->getPosition().getZ(), 10_d);
    EXPECT_VECTOR_EQ(fixed->getPosition(), fixedPosBefore); // fixed unchanged

    world.clearObjects();
}

TEST(PhysicsWorldTest, IntegrateRK4WithFixed)
{
    PhysicsWorld world;
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    Sphere* moving = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
    Sphere* fixed  = new Sphere(Vector3D(5_d, 0_d, 0_d)); // mass=0 → isFixed
    moving->setVelocity(Vector3D(0_d));
    world.addObject(moving);
    world.addObject(fixed);

    Vector3D fixedPosBefore = fixed->getPosition();
    world.integrateRK4(0.1_d);

    EXPECT_LT(moving->getPosition().getZ(), 10_d);
    EXPECT_VECTOR_EQ(fixed->getPosition(), fixedPosBefore);

    world.clearObjects();
}

// --------------------------------------------------------------------------
//  integrateWithoutCollisions
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateWithoutCollisions)
{
    // Early return when not running
    {
        PhysicsWorld world;
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));
        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        world.addObject(s);
        world.stop();
        world.integrateWithoutCollisions(); // should return early
        EXPECT_DECIMAL_EQ(s->getPosition().getZ(), 10_d);
        world.clearObjects();
    }

    // Running path
    {
        PhysicsWorld world;
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));
        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        s->setVelocity(Vector3D(0_d));
        world.addObject(s);
        world.start();
        world.integrateWithoutCollisions();
        EXPECT_LT(s->getPosition().getZ(), 10_d);
        world.clearObjects();
    }
}

// --------------------------------------------------------------------------
//  Contact forces (applyContactForces, applyContact, applyForces, computeAcceleration non-simplified)
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, ApplyContactAndForces)
{
    Config::get().setSimplifiedCollision(false);

    PhysicsWorld world;
    world.setGravityCst(0_d);
    world.setGravityAcc(Vector3D(0_d));

    // Two overlapping spheres (diameter 2, touching at distance 1 → penetration = 1)
    Sphere* s1 = new Sphere(Vector3D(0_d, 0_d, 0_d), 2_d, 1_d);
    Sphere* s2 = new Sphere(Vector3D(1_d, 0_d, 0_d), 2_d, 1_d);
    s1->setStiffnessCst(100_d);
    s2->setStiffnessCst(100_d);
    s1->setAcceleration(Vector3D(0_d));
    s2->setAcceleration(Vector3D(0_d));
    world.addObject(s1);
    world.addObject(s2);

    // applyContact applies contact forces directly
    world.applyContact();
    EXPECT_NE(s1->getAcceleration().getNorm(), 0_d);
    EXPECT_NE(s2->getAcceleration().getNorm(), 0_d);

    // computeAcceleration in non-simplified mode includes contact
    s1->setAcceleration(Vector3D(0_d));
    Vector3D acc = world.computeAcceleration(*s1);
    EXPECT_NE(acc.getNorm(), 0_d);

    // applyForces runs both gravity and contact
    s1->setAcceleration(Vector3D(0_d));
    s2->setAcceleration(Vector3D(0_d));
    world.applyForces();
    EXPECT_NE(s1->getAcceleration().getNorm(), 0_d);

    world.clearObjects();
    Config::get().setSimplifiedCollision(true); // restore
}

// --------------------------------------------------------------------------
//  removeObject
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, RemoveObject)
{
    PhysicsWorld world;
    Sphere*      s1 = new Sphere(Vector3D(0_d), 1_d, 1_d);
    Sphere*      s2 = new Sphere(Vector3D(1_d, 0_d, 0_d), 1_d, 1_d);
    world.addObject(s1);
    world.addObject(s2);
    EXPECT_EQ(world.getObjectCount(), 2u);

    world.removeObject(s2); // deletes s2
    EXPECT_EQ(world.getObjectCount(), 1u);
    // s1 is deleted by world destructor
}

// --------------------------------------------------------------------------
//  printState
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, PrintState)
{
    PhysicsWorld world;
    world.setSolver("Euler");
    world.setTimeStep(0.01_d);
    world.setGravityCst(9.81_d);
    Sphere* s = new Sphere(Vector3D(0_d, 0_d, 5_d), 1_d, 2_d);
    world.addObject(s);
    EXPECT_NO_THROW(world.printState());
    world.clearObjects();
}

// --------------------------------------------------------------------------
//  run()
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, Run)
{
    Config::get().setMaxIterations(3);
    Config::get().setVerbose(false);
    Config::get().setSave(false);

    PhysicsWorld world;
    world.setSolver("Euler");
    world.setTimeStep(0.1_d);
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    Sphere* s = new Sphere(Vector3D(0_d, 0_d, 100_d), 1_d, 1_d);
    s->setVelocity(Vector3D(0_d));
    world.addObject(s);

    world.start();
    world.run();

    EXPECT_LT(s->getPosition().getZ(), 100_d);

    world.clearObjects();
}

// --------------------------------------------------------------------------
//  integrate() with fixed objects - covers continue branches (536, 548, 571, 579)
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, IntegrateWithFixedObjects)
{
    // Simplified Euler with a fixed sphere alongside the moving one
    {
        Config::get().setSimplifiedCollision(true);
        PhysicsWorld world;
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* moving = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        Sphere* fixed  = new Sphere(Vector3D(5_d, 0_d, 0_d)); // mass=0 → isFixed
        world.addObject(moving);
        world.addObject(fixed);

        world.start();
        world.integrate();

        EXPECT_VECTOR_EQ(fixed->getPosition(), Vector3D(5_d, 0_d, 0_d));
        world.clearObjects();
    }

    // Non-simplified Euler with a fixed sphere
    {
        Config::get().setSimplifiedCollision(false);
        PhysicsWorld world;
        world.setSolver("Euler");
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* moving = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        Sphere* fixed  = new Sphere(Vector3D(5_d, 0_d, 0_d)); // mass=0 → isFixed
        world.addObject(moving);
        world.addObject(fixed);

        world.start();
        world.integrate();

        EXPECT_VECTOR_EQ(fixed->getPosition(), Vector3D(5_d, 0_d, 0_d));
        world.clearObjects();
        Config::get().setSimplifiedCollision(true);
    }
}

// --------------------------------------------------------------------------
//  Non-simplified Verlet & RK4 (covers lines 584-591 in integrate())
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, NonSimplifiedVerletRK4)
{
    Config::get().setSimplifiedCollision(false);

    for (const char* solver : { "Verlet", "RK4" })
    {
        PhysicsWorld world;
        world.setSolver(solver);
        world.setTimeStep(0.1_d);
        world.setGravityCst(9.81_d);
        world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

        Sphere* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
        s->setVelocity(Vector3D(0_d));
        world.addObject(s);

        world.start();
        world.integrate();

        EXPECT_LT(s->getPosition().getZ(), 10_d);
        world.clearObjects();
    }

    Config::get().setSimplifiedCollision(true);
}

// --------------------------------------------------------------------------
//  solveCollisions() - called from integrate() when two spheres collide
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, SolveCollisions)
{
    Config::get().setSimplifiedCollision(true);
    PhysicsWorld world;
    world.setSolver("Euler");
    world.setTimeStep(0.01_d);
    world.setGravityCst(0_d);
    world.setGravityAcc(Vector3D(0_d));

    // Two overlapping spheres (radius 1 each, 1 unit apart → overlapping)
    auto* s1 = new Sphere(Vector3D(0_d, 0_d, 0_d), 2_d, 1_d);
    auto* s2 = new Sphere(Vector3D(1_d, 0_d, 0_d), 2_d, 1_d);
    s1->setRestitutionCst(0.5_d);
    s2->setRestitutionCst(0.5_d);
    world.addObject(s1);
    world.addObject(s2);

    world.start();
    EXPECT_NO_THROW(world.integrate()); // solveCollisions() triggers reboundCollision

    world.clearObjects();
}

// --------------------------------------------------------------------------
//  run() verbose path (covers print header and per-object rows)
// --------------------------------------------------------------------------

TEST(PhysicsWorldTest, RunVerbose)
{
    Config::get().setMaxIterations(2);
    Config::get().setVerbose(true);
    Config::get().setSave(false);

    PhysicsWorld world;
    world.setSolver("Euler");
    world.setTimeStep(0.1_d);
    world.setGravityCst(9.81_d);
    world.setGravityAcc(Vector3D(0_d, 0_d, -9.81_d));

    auto* s = new Sphere(Vector3D(0_d, 0_d, 10_d), 1_d, 1_d);
    s->setVelocity(Vector3D(0_d));
    world.addObject(s);

    testing::internal::CaptureStdout();
    world.start();
    world.run();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Time"), std::string::npos);

    world.clearObjects();
    Config::get().setVerbose(false);
}
