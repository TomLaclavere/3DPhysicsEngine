#include "world/physicsWorld.hpp"

#include "world/physics.hpp"

#include <iomanip>
#include <iostream>

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
void PhysicsWorld::initialize()
{
    isRunning = false;
    objects.clear();

    timeStep   = config.getTimeStep();
    gravityCst = config.getGravity();
    gravityAcc = Physics::computeGravityAcc(gravityCst);
}

// ============================================================================
//  Integration
// ============================================================================
void PhysicsWorld::integrateEuler(Object& obj, decimal dt)
{
    // v_{t+dt} = v_t + a_t * dt
    obj.setVelocity(obj.getVelocity() + obj.getAcceleration() * dt);
    // x_{t+dt} = x_t + v_{t+dt} * dt
    obj.setPosition(obj.getPosition() + obj.getVelocity() * dt);
}
void PhysicsWorld::integrate(decimal dt)
{
    if (!isRunning)
    {
        std::cout << "Simulation is not running. Run start() first." << std::endl;
        return;
    }

    setTimeStep(dt);

    // Reset accelerations
    for (auto* obj : objects)
    {
        if (obj)
        {
            obj->setAcceleration(Vector3D(0_d));
        }
    }

    // Compute all forces
    applyForces();

    // Integrate movable objects
    for (auto* obj : objects)
    {
        if (!obj || obj->isFixed())
            continue;
        integrateEuler(*obj, dt);
    }
}
void PhysicsWorld::run()
{
    const decimal timeStep = config.getTimeStep();
    const size_t  maxIter  = config.getMaxIterations();
    size_t        cpt      = 0;

    // Printing
    // Column widths
    constexpr int col_obj = 10;
    constexpr int col_vec = 40;
    size_t        n       = col_obj + 2 * col_vec;

    // Header
    std::cout << std::left << std::setw(col_obj) << "Object" << std::setw(col_vec) << "Position(x,y,z)"
              << std::setw(col_vec) << "Velocity(x,y,z)" << "\n";
    std::cout << std::string(n, '-') << "\n";

    while (cpt < maxIter && getIsRunning())
    {
        integrate(timeStep);
        cpt++;

        // Printing
        if (cpt % 25 == 0)
        {
            for (auto* obj : getObject())
            {
                if (!obj->isFixed())
                    std::cout << std::left << std::setw(col_obj) << obj->getType() << std::fixed
                              << std::setprecision(3) << std::setw(col_vec)
                              << obj->getPosition().formatVector() << std::setw(col_vec)
                              << obj->getVelocity().formatVector() << "\n";
            }
            std::cout << std::string(n, '-') << '\n';
        }
    }
}

// ============================================================================
//  Force application
// ============================================================================
void PhysicsWorld::applyGravityForces()
{
    {
        for (auto* obj : objects)
            if (obj && !obj->getIsFixed())
                obj->addAcceleration(gravityAcc);
    }
}

void PhysicsWorld::applySpringForces(Object& obj, Object& other)
{
    if (!obj.getIsFixed())
    {
        Vector3D springForce = Physics::computeSpringForce(obj, other);
        obj.addAcceleration(springForce / obj.getMass());
    }
}

void PhysicsWorld::applyDamplingForces(Object& obj, Object& other)
{
    if (!obj.getIsFixed())
    {
        Vector3D dampingForce = Physics::computeDampingForce(obj, other);
        obj.addAcceleration(dampingForce / obj.getMass());
    }
}

void PhysicsWorld::applyFrictionForces(Object& obj, Object& other)
{
    if (!obj.getIsFixed())
    {
        Vector3D frictionForce = Physics::computeFrictionForce(obj, other);
        obj.addAcceleration(frictionForce / obj.getMass());
    }
}

void PhysicsWorld::applyContactForces(Object& obj, Object& other)
{
    if (obj.getIsFixed() && other.getIsFixed())
        return;

    Vector3D springForce   = Physics::computeSpringForce(obj, other);
    Vector3D dampingForce  = Physics::computeDampingForce(obj, other);
    Vector3D frictionForce = Physics::computeFrictionForce(obj, other);
    Vector3D totalForce    = springForce + dampingForce + frictionForce;

    if (!obj.getIsFixed())
        obj.addAcceleration(totalForce / obj.getMass());
    if (!other.getIsFixed())
        other.addAcceleration(-totalForce / other.getMass());
}

void PhysicsWorld::avoidOverlap(Object& obj, Object& other)
{
    if (obj.getIsFixed() && other.getIsFixed() || !obj.checkCollision(other))
        return;

    if (!obj.getIsFixed())
    {
        obj.setAcceleration(Vector3D(0_d));
        obj.setVelocity(Vector3D(0_d));
    }
    if (!other.getIsFixed())
    {
        other.setAcceleration(Vector3D(0_d));
        other.setVelocity(Vector3D(0_d));
    }
}

void PhysicsWorld::applyForces()
{
    // 1. Gravity (applies to all objects)
    applyGravityForces();

    // 2. Contact forces (between object pairs)
    const size_t n = objects.size();
    for (size_t i = 0; i < n; ++i)
    {
        Object* obj1 = objects[i];
        if (!obj1)
            continue;

        for (size_t j = i + 1; j < n; ++j)
        {
            Object* obj2 = objects[j];
            if (!obj2)
                continue;

            // Only apply contact forces if objects are colliding
            if (obj1->checkCollision(*obj2))
            {
                // applyContactForces(*obj1, *obj2);
                avoidOverlap(*obj1, *obj2);
            }
        }
    }
}

// ============================================================================
//  Object management
// ============================================================================

// ============================================================================
//  Print & Save
// ============================================================================
void PhysicsWorld::printState() const
{
    std::cout << "PhysicsWorld state:\n";
    std::cout << "  Running: " << std::boolalpha << isRunning << "\n";
    std::cout << "  TimeStep: " << timeStep << " s\n";
    std::cout << "  Gravity: " << gravityCst << " m/s²\n";
    std::cout << "  Objects: " << objects.size() << "\n";

    // Print each object's state
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i])
        {
            std::cout << "  Object " << i << ": pos=" << objects[i]->getPosition()
                      << ", vel=" << objects[i]->getVelocity() << "\n";
        }
    }
}
