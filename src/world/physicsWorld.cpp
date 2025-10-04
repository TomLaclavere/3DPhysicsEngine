#include "world/physicsWorld.hpp"

// ============================================================================
//  Core simulation methods
// ============================================================================
void PhysicsWorld::initialize()
{
    isRunning = false;
    objects.clear();
    timeStep        = config.getTimeStep();
    gravityConstant = config.getGravity();
}

// ============================================================================
//  Time step methods
// ============================================================================
void PhysicsWorld::update(decimal dt)
{
    if (!isRunning)
        return;

    timeStep = dt;
    fixedUpdate();
}

void PhysicsWorld::fixedUpdate()
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
Vector3D PhysicsWorld::computeGravity() { return Vector3D(0_d, 0_d, gravityConstant); }
void     PhysicsWorld::applyGravity()
{
    for (auto* obj : objects)
    {
        if (!obj)
            continue;

        // Update acceleration
        obj->setAcceleration(gravityAcc);
    }
}

// ============================================================================
//  Objects management
// ============================================================================

// ============================================================================
//  Print & Save
// ============================================================================
