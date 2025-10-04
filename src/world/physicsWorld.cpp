#include "world/physicsWorld.hpp"

void PhysicsWorld::initialize()
{
    isRunning = false;
    objects.clear();
    timeStep        = config.getTimeStep();
    gravityConstant = config.getGravity();
}

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
        applyGravity();
        obj->setVelocity(obj->getVelocity() + obj->getAcceleration() * timeStep);
        obj->setPosition(obj->getPosition() + obj->getVelocity() * timeStep);
    }
}

void PhysicsWorld::applyGravity()
{
    for (auto* obj : objects)
    {
        if (!obj)
            continue;

        // Force = mass * acceleration
        Vector3D gravityForce(0_d, 0_d, gravityConstant * obj->getMass());

        // Reset force before applying gravity
        obj->setForce(Vector3D(0_d, 0_d, 0_d));
        obj->setForce(gravityForce);

        // Update acceleration
        obj->setAcceleration(obj->getForce() / obj->getMass());
    }
}

void PhysicsWorld::reset() { initialize(); }
