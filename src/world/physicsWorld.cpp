#include "world/physicsWorld.hpp"

#include "collision/collision_response.hpp"
#include "mathematics/math_io.hpp"
#include "world/integrateRK4.hpp"
#include "world/physics.hpp"

#include <iomanip>
#include <iostream>

// ============================================================================
//  Solvers
// ============================================================================
static Solver parseSolver(const std::string& name)
{
    if (name == "Euler")
        return Solver::Euler;
    if (name == "Verlet")
        return Solver::Verlet;
    if (name == "RK4")
        return Solver::RK4;
    return Solver::Unknown;
}

// ============================================================================
//  Getters
// ============================================================================
Config&  PhysicsWorld::getConfig() const { return config; }
bool     PhysicsWorld::getIsRunning() const { return isRunning; }
decimal  PhysicsWorld::getTimeStep() const { return timeStep; }
decimal  PhysicsWorld::getGravityCst() const { return gravityCst; }
Vector3D PhysicsWorld::getGravityAcc() const { return gravityAcc; }
Solver   PhysicsWorld::getSolver() const { return solver; }

// ============================================================================
//  Setters
// ============================================================================
void PhysicsWorld::setSolver(std::string _solver)
{
    solver = parseSolver(_solver);
    config.setSolver(_solver);
}
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

    solver     = parseSolver(config.getSolver());
    timeStep   = config.getTimeStep();
    gravityCst = config.getGravity();
    gravityAcc = Physics::computeGravityAcc(gravityCst);
}

// ============================================================================
//  Force application
// ============================================================================
void PhysicsWorld::applyGravityForce(Object& obj)
{
    if (!obj.getIsFixed())
        obj.addAcceleration(gravityAcc);
}
void PhysicsWorld::applyGravityForces()
{
    {
        for (auto* obj : objects)
            applyGravityForce(*obj);
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
void PhysicsWorld::computeAcceleration(Object& obj)
{
    // Reset Acceleration
    obj.setAcceleration(Vector3D(0_d));

    // Apply gravity
    applyGravityForce(obj);

    // Contact forces
    for (auto* other : objects)
    {
        if (!other || other == &obj)
            continue;

        if (obj.checkCollision(*other))
        {
            applyContactForces(obj, *other);
        }
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
                applyContactForces(*obj1, *obj2);
            }
        }
    }
}

void PhysicsWorld::solveCollisions()
{
    const size_t n = objects.size();
    Contact      contact;

    for (size_t i = 0; i < n; ++i)
    {
        Object* A = objects[i];
        if (!A)
            continue;

        for (size_t j = i + 1; j < n; ++j)
        {
            Object* B = objects[j];
            if (!B)
                continue;

            // Broad phase
            bool isColliding = A->computeCollision(*B, contact);

            // Narrow phase
            if (isColliding)
            {
                reboundCollision(*A, *B, contact);
            }
        }
    }
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
void PhysicsWorld::integrateVerlet(Object& obj, decimal dt)
{
    // Store current acceleration
    Vector3D currentAcc = obj.getAcceleration();

    // position
    Vector3D nextPos = obj.getPosition() + obj.getVelocity() * dt + obj.getAcceleration() * (0.5_d * dt * dt);
    obj.setPosition(nextPos);

    // acceleration from new position
    computeAcceleration(obj);
    Vector3D nextAcc = obj.getAcceleration();

    // velocity
    Vector3D nextVel = obj.getVelocity() + (currentAcc + nextAcc) * (0.5_d * dt);
    obj.setVelocity(nextVel);
}
Derivative PhysicsWorld::evaluateRK4(const Object& obj, const Derivative& d, decimal dt)
{
    Object tmp = obj; // copy object state

    tmp.setPosition(obj.getPosition() + d.derivativeX * dt);
    tmp.setVelocity(obj.getVelocity() + d.derivativeV * dt);

    // Recompute acceleration for the intermediate state
    computeAcceleration(tmp);

    Derivative out;
    out.derivativeX = tmp.getVelocity();
    out.derivativeV = tmp.getAcceleration();
    return out;
}
void PhysicsWorld::integrateRK4(Object& obj, decimal dt)
{
    Derivative k1, k2, k3, k4;

    // k1
    k1.derivativeX = obj.getVelocity();
    k1.derivativeV = obj.getAcceleration();
    // k2
    k2 = evaluateRK4(obj, k1, dt * 0.5_d);
    // k3
    k3 = evaluateRK4(obj, k2, dt * 0.5_d);
    // k4
    k4 = evaluateRK4(obj, k3, dt);

    // Weighted average derivative
    Vector3D dxdt =
        (k1.derivativeX + (2_d * k2.derivativeX) + (2_d * k3.derivativeX) + k4.derivativeX) * (1_d / 6_d);
    Vector3D dvdt =
        (k1.derivativeV + (2_d * k2.derivativeV) + (2_d * k3.derivativeV) + k4.derivativeV) * (1_d / 6_d);

    obj.setPosition(obj.getPosition() + dxdt * dt);
    obj.setVelocity(obj.getVelocity() + dvdt * dt);
}

void PhysicsWorld::integrate()
{
    if (!isRunning)
    {
        std::cout << "Simulation is not running. Run start() first." << std::endl;
        return;
    }

    setTimeStep(timeStep);

    // Reset accelerations
    for (auto* obj : objects)
    {
        if (!obj || obj->isFixed())
            continue;
        obj->setAcceleration(Vector3D(0_d));
    }

    // Compute gravity forces
    applyGravityForces();

    // Integrate motion
    for (auto* obj : objects)
    {
        if (!obj || obj->isFixed())
            continue;
        switch (solver)
        {
        case Solver::Euler:
            integrateEuler(*obj, timeStep);
            break;
        case Solver::Verlet:
            integrateVerlet(*obj, timeStep);
            break;
        case Solver::RK4:
            integrateRK4(*obj, timeStep);
            break;
        case Solver::Unknown:
            std::cout << "The following solver is not implemented : " << config.getSolver() << '\n';
            std::cout << "Please use one of the following solver : Euler, Verlet, RK4.\n";
            break;
        }
    }

    // Collision resolution
    solveCollisions();
}

void PhysicsWorld::run()
{
    const decimal timeStep = config.getTimeStep();
    const size_t  maxIter  = config.getMaxIterations();
    size_t        cpt      = 0;

    // Printing
    // Column widths
    constexpr int col_obj  = 10;
    constexpr int col_time = 10;
    constexpr int col_vec  = 40;
    size_t        n        = col_obj + col_time + 2 * col_vec;

    // Header
    if (config.getVerbose())
    {
        std::cout << std::left << std::setw(col_obj) << "Object" << std::setw(col_time) << "Time(s)"
                  << std::setw(col_vec) << "Position(x,y,z)" << std::setw(col_vec) << "Velocity(x,y,z)"
                  << "\n";
        std::cout << std::string(n, '-') << "\n";
    }

    while (cpt < maxIter + 1 && getIsRunning())
    {
        const decimal time = static_cast<decimal>(cpt) * timeStep;

        integrate();

        // Printing
        if (config.getVerbose())
        {
            if (cpt % 10 == 0)
            {
                for (auto* obj : getObject())
                {
                    if (!obj->isFixed())
                        std::cout << std::left << std::setw(col_obj) << obj->getType() << std::setw(col_time)
                                  << std::fixed << std::setprecision(3) << time << std::setw(col_vec)
                                  << formatVector(obj->getPosition()) << std::setw(col_vec)
                                  << formatVector(obj->getVelocity()) << "\n";
                }
                std::cout << std::string(n, '-') << '\n';
            }
            cpt++;
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
    std::cout << "  Solver: " << solver << "\n";
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
