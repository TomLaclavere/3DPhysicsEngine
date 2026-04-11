/**
 * @file physicsWorld.hpp
 * @brief Definition of the Physical World used in physics simulation.
 *
 */
#pragma once
#include "mathematics/vector.hpp"
#include "objects/object.hpp"
#include "utilities/csv.hpp"
#include "world/config.hpp"
#include "world/integrateRK4.hpp"
#include "world/physics.hpp"
#include "world/solver.hpp"

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <vector>

/**
 * @class PhysicsWorld
 * @brief Represents the Physical World for physics simulation.
 *
 * The PhysicsWorld is constructed with a configuration file, containing the physical and simulation
 * parameters. It contains methods for Object management, forces computation and to integrate the equation of
 * motion.
 */
struct PhysicsWorld
{
private:
    // World
    Config&                                    config = Config::get();
    std::vector<Object*>                       objects;
    std::ofstream                              objectFile;
    std::unordered_map<Object*, std::ofstream> motionFiles;

    // Simulation
    bool     isRunning = false;
    Solver   solver;
    decimal  timeStep   = config.getTimeStep();
    decimal  gravityCst = config.getGravity();
    Vector3D gravityAcc = Physics::computeGravityAcc(gravityCst);

    // Utilites
    unsigned int            nextObjectId = 0;
    static constexpr size_t FLUSH_EVERY  = 500;
    std::vector<MotionCSV>  motionBuffer;
    void                    flushMotionBuffer();

public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    PhysicsWorld() = default;
    explicit PhysicsWorld(Config& _config)
        : config(_config)
    {
        (*this).initialise();
    }
    ~PhysicsWorld() { clearObjects(); };
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    Config&      getConfig();
    bool         getIsRunning() const;
    decimal      getTimeStep() const;
    decimal      getGravityCst() const;
    Vector3D     getGravityAcc() const;
    Solver       getSolver() const;
    unsigned int getNextObjectId() const { return nextObjectId; }
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setSolver(const std::string& _solver);
    void setTimeStep(decimal step);
    void setGravityCst(decimal g);
    void setGravityAcc(const Vector3D& acc);
    /// @}

    // ============================================================================
    /// @name Core simulation methods
    // ============================================================================
    /// @{

    /// Initialise the physics world: reset objects, time step, and gravity.
    void initialise();
    void start() { isRunning = true; }
    void stop() { isRunning = false; }
    /// Re-initialise PhysicsWorld
    void resetAcc()
    {
        for (auto* obj : objects)
        {
            obj->setAcceleration(Vector3D(0_d));
        }
    }

    /// @}

    // ============================================================================
    /// @name Force computation
    // ============================================================================
    /// @{

    /// Apply gravitational force to one object.
    void applyGravityForce(Object& obj);
    /// Apply gravitational force to all movable objects.
    void applyGravityForces();
    /// Apply contact forces (spring + damping + friction) between two objects.
    void applyContactForces(Object& obj, Object& other, Contact& contact);
    /// Compute and apply contact forces for the current physics step.
    void applyContact();
    /// Apply all forces to all objects.
    void applyForces();
    /// Solve collisions between objects.
    void solveCollisions();
    /// Compute gravitational force only.
    Vector3D computeAccelerationGravityOnly();
    /// Compute acceleration from forces.
    Vector3D computeAcceleration(Object& obj);
    /// @}

    // ============================================================================
    /// @name Time step methods
    // ============================================================================
    /// @{

    /// Semi-implicit Euler integrator for one object.
    void integrateEuler(Object& obj, decimal dt);
    /// Verlet integrator for all objects.
    void integrateVerlet(decimal dt);
    /// RK4 integrator for all objects.
    void integrateRK4(decimal dt);
    /// @brief Integrate all objects over one time step without collision resolution.
    /// Only for testing purposes.
    void integrateWithoutCollisions();
    /// @brief Integrate all objects over one time step.
    /// Resets accelerations, applies forces, and moves objects using semi-implicit Euler.
    void integrate();
    /// Run simulation over all iterations.
    void run();
    /// @}

    // ============================================================================
    /// @name Object management
    // ============================================================================
    /// @{

    /// Add Object in the PhysicaWorld.
    void addObject(Object* obj)
    {
        if (obj)
        {
            obj->setId(nextObjectId++);
            objects.push_back(obj);
        }
    }
    void removeObject(Object* obj)
    {
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
    }
    /// Clear Object array
    void    clearObjects() { objects.clear(); }
    size_t  getObjectCount() const { return objects.size(); }
    Object* getObject(size_t index) const { return (index < objects.size()) ? objects[index] : nullptr; }
    Object* getObject(size_t index) { return (index < objects.size()) ? objects[index] : nullptr; }
    std::vector<Object*> getObject() { return objects; }
    /// @}

    // ============================================================================
    /// @name Printing & Saving
    // ============================================================================
    /// @{

    /// Print the current state of the physics world to stdout.
    void printState() const;
    void initCSV(const std::string& directory);
    void saveObjectsCSV();
    void flushMotionCSV();
    void saveMotionCSV(decimal time);
    void closeCSV();
    /// @}
};
