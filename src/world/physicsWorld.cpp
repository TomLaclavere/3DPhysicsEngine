#include "world/physicsWorld.hpp"

// ============================================================================
//  Getters
// ============================================================================
Config&  PhysicsWorld::getConfig() const { return config; }
bool     PhysicsWorld::getIsRunning() const { return isRunning; }
decimal  PhysicsWorld::getTimeStep() const { return timeStep; }
decimal  PhysicsWorld::getGravityCst() const { return gravityCst; }
Vector3D PhysicsWorld::getGravityAcc() const { return gravityAcc; }

// ============================================================================
//  Setters
// ============================================================================
void PhysicsWorld::setTimeStep(decimal ind) { timeStep = ind; }
void PhysicsWorld::setGravityCst(decimal g) { gravityCst = g; }
void PhysicsWorld::setGravityAcc(const Vector3D& acc) { gravityAcc = acc; }

// ============================================================================
//  Core simulation methods
// ============================================================================
/// Clear Object array and initialise physical and simulation parameters from config
void PhysicsWorld::initialize()
{
    isRunning = false;
    objects.clear();
    timeStep   = config.getTimeStep();
    gravityCst = config.getGravity();
}

// ============================================================================
//  Time step methods
// ============================================================================
void PhysicsWorld::update(decimal dt)
{
    if (!isRunning)
        return;

    timeStep = dt;
    integrateEuler();
}
/// Update position and velocity of each Object in PhysicalWorld using semi-explicite Euler integration.
void PhysicsWorld::integrateEuler()
{
    // Update each object's position and velocity
    for (auto* obj : objects)
    {
        if (!obj)
            continue; // Skip null pointers

        // Semi-implicit Euler integration
        applyForces();
        obj->setVelocity(obj->getVelocity() + obj->getAcceleration() * timeStep);
        obj->setPosition(obj->getPosition() + obj->getVelocity() * timeStep);
    }
}

// ============================================================================
//  Force computation
// ============================================================================
Vector3D PhysicsWorld::computeGravity() { return Vector3D(0_d, 0_d, -gravityCst); }
/// Apply gravtiational force to all Object in Physical World by updating their acceleration.
void PhysicsWorld::applyGravity()
{
    for (auto* obj : objects)
    {
        if (!obj)
            continue;

        // Update acceleration
        obj->setAcceleration(gravityAcc);
    }
}
void PhysicsWorld::applyForces() { applyGravity(); }

// ============================================================================
//  Objects management
// ============================================================================

// ============================================================================
//  Print & Save
// ============================================================================
