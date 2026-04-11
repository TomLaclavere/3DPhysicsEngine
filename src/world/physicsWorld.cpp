#include "world/physicsWorld.hpp"

#include "collision/collision_response.hpp"
#include "collision/contact.hpp"
#include "mathematics/math_io.hpp"
#include "mathematics/vector.hpp"
#include "objects/object.hpp"
#include "precision.hpp"
#include "world/integrateRK4.hpp"
#include "world/physics.hpp"

#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>

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
Config&  PhysicsWorld::getConfig() { return config; }
bool     PhysicsWorld::getIsRunning() const { return isRunning; }
decimal  PhysicsWorld::getTimeStep() const { return timeStep; }
decimal  PhysicsWorld::getGravityCst() const { return gravityCst; }
Vector3D PhysicsWorld::getGravityAcc() const { return gravityAcc; }
Solver   PhysicsWorld::getSolver() const { return solver; }

// ============================================================================
//  Setters
// ============================================================================
void PhysicsWorld::setSolver(const std::string& _solver)
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
void PhysicsWorld::initialise()
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

    for (auto* obj : objects)
    {
        applyGravityForce(*obj);
    }
}
void PhysicsWorld::applyContactForces(Object& obj, Object& other, Contact& contact)
{
    Vector3D totalForce = Physics::computeContactForce(obj, other, contact);
    if (!obj.getIsFixed())
        obj.addAcceleration(totalForce / obj.getMass());
    if (!other.getIsFixed())
        other.addAcceleration(-totalForce / other.getMass());
}
void PhysicsWorld::applyContact()
{
    const size_t n = objects.size();

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
            bool isCollidingBroad = A->checkCollision(*B);

            // Narrow phase
            if (isCollidingBroad)
            {
                Contact contact;
                bool    isCollidindNarrow = A->computeCollision(*B, contact);
                if (isCollidindNarrow)
                    applyContactForces(*A, *B, contact);
            }
        }
    }
}
void PhysicsWorld::applyForces()
{
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
                    Contact contact;
                    bool    isCollidindNarrow = obj1->computeCollision(*obj2, contact);
                    if (isCollidindNarrow)
                        applyContactForces(*obj1, *obj2, contact);
                }
            }
        }
    }
}
void PhysicsWorld::solveCollisions()
{
    const size_t n = objects.size();

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
            bool isCollidingBroad = A->checkCollision(*B);

            // Narrow phase
            if (isCollidingBroad)
            {
                Contact contact;
                bool    isCollidindNarrow = A->computeCollision(*B, contact);
                if (isCollidindNarrow)
                    reboundCollision(*A, *B, contact);
            }
        }
    }
}
Vector3D PhysicsWorld::computeAccelerationGravityOnly() { return gravityAcc; }
Vector3D PhysicsWorld::computeAcceleration(Object& obj)
{
    // Impulse-based collisions pipeline : collision are solved with velocities, contact forces don't enter in
    // integration
    if (config.getSimplifiedCollision())
        return computeAccelerationGravityOnly();

    Vector3D acc = gravityAcc;
    for (auto& other : objects)
    {
        if (!other || &obj == other)
            continue;
        if (obj.checkCollision(*other))
        {
            Contact contact;
            if (obj.computeCollision(*other, contact))
                acc += Physics::computeContactForce(obj, *other, contact) / obj.getMass();
        }
    }
    return acc;
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
void PhysicsWorld::integrateVerlet(decimal dt)
{
    const size_t n = objects.size();

    // 1) Accelerations at (t) from computeAcceleration
    std::vector<Vector3D> prevAcc(n, Vector3D(0_d));
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        prevAcc[i] = computeAcceleration(*obj);
    }

    // 2) Positions at (t+dt)
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        obj->setPosition(obj->getPosition() + obj->getVelocity() * dt + prevAcc[i] * (0.5_d * dt * dt));
    }

    // 3) Accelerations at (t+dt), from new positions
    std::vector<Vector3D> nextAcc(n, Vector3D(0_d));
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        nextAcc[i] = computeAcceleration(*obj);
    }

    // 4) Velocities at (t+dt)
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        obj->setVelocity(obj->getVelocity() + (prevAcc[i] + nextAcc[i]) * (0.5_d * dt));
    }
}
void PhysicsWorld::integrateRK4(decimal dt)
{
    const size_t n = objects.size();

    struct State
    {
        Vector3D pos, vel, acc;
    };
    std::vector<State>      s0(n);
    std::vector<Derivative> k1(n), k2(n), k3(n), k4(n);

    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        s0[i] = { .pos = obj->getPosition(), .vel = obj->getVelocity(), .acc = obj->getAcceleration() };
    }

    // k1
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        k1[i].derivativeX = s0[i].vel;
        k1[i].derivativeV = computeAcceleration(*obj);
    }

    // k2
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        obj->setPosition(s0[i].pos + k1[i].derivativeX * (0.5_d * dt));
        obj->setVelocity(s0[i].vel + k1[i].derivativeV * (0.5_d * dt));
    }
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        k2[i].derivativeX = obj->getVelocity();
        k2[i].derivativeV = computeAcceleration(*obj);
    }

    // k3
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        obj->setPosition(s0[i].pos + k2[i].derivativeX * (0.5_d * dt));
        obj->setVelocity(s0[i].vel + k2[i].derivativeV * (0.5_d * dt));
    }
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        k3[i].derivativeX = obj->getVelocity();
        k3[i].derivativeV = computeAcceleration(*obj);
    }

    // k4
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        obj->setPosition(s0[i].pos + k3[i].derivativeX * dt);
        obj->setVelocity(s0[i].vel + k3[i].derivativeV * dt);
    }
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;
        k4[i].derivativeX = obj->getVelocity();
        k4[i].derivativeV = computeAcceleration(*obj);
    }

    // Update
    for (size_t i = 0; i < n; ++i)
    {
        auto* obj = objects[i];
        if (!obj || obj->isFixed())
            continue;

        Vector3D dxdt =
            (k1[i].derivativeX + 2_d * k2[i].derivativeX + 2_d * k3[i].derivativeX + k4[i].derivativeX) *
            (1_d / 6_d);
        Vector3D dvdt =
            (k1[i].derivativeV + 2_d * k2[i].derivativeV + 2_d * k3[i].derivativeV + k4[i].derivativeV) *
            (1_d / 6_d);

        obj->setPosition(s0[i].pos + dxdt * dt);
        obj->setVelocity(s0[i].vel + dvdt * dt);
    }
}
void PhysicsWorld::integrateWithoutCollisions()
{
    if (!isRunning)
    {
        std::cout << "Simulation is not running. Run start() first.\n";
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
    switch (solver)
    {
    case Solver::Euler:
        for (auto* obj : objects)
        {
            if (!obj || obj->isFixed())
                continue;
            integrateEuler(*obj, timeStep);
        }
        break;
    case Solver::Verlet:
        integrateVerlet(timeStep);
        break;
    case Solver::RK4:
        integrateRK4(timeStep);
        break;
    default:
        std::cout << "Unknown solver: " << config.getSolver() << '\n';
    }

    // If collision : object stops moving
    const size_t n = objects.size();

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
            bool isCollidingBroad = A->checkCollision(*B);

            // Narrow phase
            if (isCollidingBroad)
            {
                Contact contact;
                bool    isCollidindNarrow = A->computeCollision(*B, contact);
                if (isCollidindNarrow)
                {
                    A->setVelocity(Vector3D(0_d));
                    A->setIsFixed(true);
                    B->setVelocity(Vector3D(0_d));
                    B->setIsFixed(true);
                }
            }
        }
    }
}
void PhysicsWorld::integrate()
{
    if (!isRunning)
    {
        std::cout << "Simulation is not running. Run start() first.\n";
        return;
    }

    if (config.getSimplifiedCollision())
    {
        // Impulse-based collisions pipeline : pre-compute gravity + apply impulsions before integration.
        for (auto* obj : objects)
        {
            if (!obj || obj->isFixed())
                continue;
            obj->setAcceleration(Vector3D(0_d));
        }
        applyGravityForces();
        solveCollisions();

        switch (solver)
        {
        case Solver::Euler:
            for (auto* obj : objects)
            {
                if (!obj || obj->isFixed())
                    continue;
                integrateEuler(*obj, timeStep);
            }
            break;
        case Solver::Verlet:
            integrateVerlet(timeStep);
            break;
        case Solver::RK4:
            integrateRK4(timeStep);
            break;
        default:
            std::cout << "Unknown solver: " << config.getSolver() << '\n';
        }
    }
    else
    {
        // Force-based collisions pipeline : computeAcceleration() compute everything internaly.
        switch (solver)
        {
        case Solver::Euler: {
            for (auto* obj : objects)
            {
                if (!obj || obj->isFixed())
                    continue;
                obj->setAcceleration(Vector3D(0_d));
            }
            applyGravityForces();
            applyContact();
            for (auto* obj : objects)
            {
                if (!obj || obj->isFixed())
                    continue;
                integrateEuler(*obj, timeStep);
            }
            break;
        }
        case Solver::Verlet:
            integrateVerlet(timeStep);
            break;
        case Solver::RK4:
            integrateRK4(timeStep);
            break;
        default:
            std::cout << "Unknown solver: " << config.getSolver() << '\n';
        }
    }
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

    // If save
    initCSV("output/CSV");
    saveObjectsCSV();

    while (cpt < maxIter + 1 && getIsRunning())
    {
        const decimal time = static_cast<decimal>(cpt) * timeStep;

        integrate();
        saveMotionCSV(time);

        // Printing
        if (config.getVerbose())
        {
            if (cpt % 25 == 0)
            {
                for (auto* obj : objects)
                {
                    if (!obj->isFixed())
                        std::cout << std::left << std::setw(col_obj) << obj->getType() << std::setw(col_time)
                                  << std::fixed << std::setprecision(3) << time << std::setw(col_vec)
                                  << formatVector(obj->getPosition()) << std::setw(col_vec)
                                  << formatVector(obj->getVelocity()) << "\n";
                }
                std::cout << std::string(n, '-') << '\n';
            }
        }
        cpt++;
    }

    // Close buffer
    closeCSV();
}

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
void PhysicsWorld::initCSV(const std::string& directory)
{
    if (!config.getSave())
        return;

    // Create CSV directory
    if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }

    // Object CSV
    objectFile.clear();
    objectFile.open(directory + "/objects.csv");
    if (!objectFile)
    {
        throw std::runtime_error("Cannot open objects.csv");
    }
    objectFile << "id" << "," << "name" << "," << "type" << "," << "mass" << "," << "pos(x)" << ","
               << "pos(y)" << "," << "pos(z)" << "," << "size(x)" << ","
               << "size(y)" << "," << "size(z)" << "," << "rota(x)" << "," << "rota(y)" << ","
               << "rota(z)" << "," << "fixed" << "\n";

    // Motion CSV
    motionFiles.clear();
    for (std::size_t idx = 0; idx < objects.size(); ++idx)
    {
        Object* obj = objects[idx];

        // if (obj->getIsFixed())
        //     continue;

        std::string   filepath = directory + "/motion_object_" + std::to_string(idx) + ".csv";
        std::ofstream file(filepath);

        obj->initMotionCSV(file);
        motionFiles.emplace(obj, std::move(file));
    }

    // Buffer
    motionBuffer.reserve(FLUSH_EVERY);
}
void PhysicsWorld::saveObjectsCSV()
{
    for (auto& object : objects)
    {
        object->saveObjectCSV(objectFile);
    }
    objectFile.close();
}
void PhysicsWorld::flushMotionBuffer()
{
    if (motionBuffer.empty())
        return;

    for (auto& snap : motionBuffer)
    {
        auto& file = motionFiles.at(snap.obj);

        file << snap.time << "," << snap.pos.getX() << "," << snap.pos.getY() << "," << snap.pos.getZ() << ","
             << snap.vel.getX() << "," << snap.vel.getY() << "," << snap.vel.getZ() << "," << snap.acc.getX()
             << "," << snap.acc.getY() << "," << snap.acc.getZ() << "\n";
    }

    motionBuffer.clear();
}
void PhysicsWorld::saveMotionCSV(decimal time)
{
    if (!config.getSave())
        return;

    for (auto& [obj, file] : motionFiles)
    {
        motionBuffer.push_back({ .time = time,
                                 .obj  = obj,
                                 .pos  = obj->getPosition(),
                                 .vel  = obj->getVelocity(),
                                 .acc  = obj->getAcceleration() });
    }

    if (motionBuffer.size() >= FLUSH_EVERY)
        flushMotionBuffer();
}
void PhysicsWorld::closeCSV()
{
    flushMotionBuffer(); // empty residual buffer
    for (auto& [obj, file] : motionFiles)
        file.close(); // flush + close
    objectFile.close();
}
