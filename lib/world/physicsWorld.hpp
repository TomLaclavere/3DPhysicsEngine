/**
 * @file physicsWorld.hpp
 * @brief Definition of the Physical World used in physics simulation.
 *
 */
#pragma once
#include "mathematics/common.hpp"
#include "objects/object.hpp"
#include "world/config.hpp"
#include "world/integrateRK4.hpp"
#include "world/physics.hpp"

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
    Config&              config = Config::get();
    std::vector<Object*> objects;

    bool        isRunning  = false;
    std::string solver     = config.getSolver();
    decimal     timeStep   = config.getTimeStep();
    decimal     gravityCst = config.getGravity();
    Vector3D    gravityAcc = Physics::computeGravityAcc(gravityCst);

public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    PhysicsWorld() = default;
    explicit PhysicsWorld(Config& _config)
        : config { _config }
    {}
    ~PhysicsWorld() { clearObjects(); };
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    Config&  getConfig() const;
    bool     getIsRunning() const;
    decimal  getTimeStep() const;
    decimal  getGravityCst() const;
    Vector3D getGravityAcc() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setSolver(std::string solver);
    void setTimeStep(decimal step);
    void setGravityCst(decimal g);
    void setGravityAcc(const Vector3D& acc);
    /// @}

    // ============================================================================
    /// @name Core simulation methods
    // ============================================================================
    /// @{

    /// Initialize the physics world: reset objects, time step, and gravity.
    void initialize();
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
    /// @name Time step methods
    // ============================================================================
    /// @{

    /// Semi-implicit Euler integrator for one object.
    void integrateEuler(Object& obj, decimal dt);
    /// Verlet integrator for one object.
    void integrateVerlet(Object& obj, decimal dt);
    /// Runge-Kutta 4 integrator for one object.
    void integrateRK4(Object& obj, decimal dt);
    /// @brief Integrate all objects over one time step.
    /// Resets accelerations, applies forces, and moves objects using semi-implicit Euler.
    void integrate();
    /// Run simulation over all iterations.
    void run();
    /// @}

    // ============================================================================
    /// @name Force computation
    // ============================================================================
    /// @{

    /// Apply gravitational force to one object.
    void applyGravityForce(Object& obj);
    /// Apply gravitational force to all movable objects.
    void applyGravityForces();
    /// Apply spring forces on a single object due to another.
    void applySpringForces(Object& obj, Object& other);
    /// Apply dampling forces on a single object due to another.
    void applyDamplingForces(Object& obj, Object& other);
    /// Apply friction forces on a single object due to another.
    void applyFrictionForces(Object& obj, Object& other);
    /// Apply contact forces (spring + damping + friction) between two objects.
    void applyContactForces(Object& obj, Object& other);
    /// Stop overlapping objects by zeroing their velocity and acceleration.
    void avoidOverlap(Object& obj, Object& other);
    /// Compute and apply all forces for the curent physics step on one Object.
    void computeAcceleration(Object& obj);
    /// Compute and apply all forces for the current physics step.
    void applyForces();
    /// @}

    // ============================================================================
    /// @name Object management
    // ============================================================================
    /// @{

    /// Add Object in the PhysicaWorld.
    void addObject(Object* obj)
    {
        if (obj)
            objects.push_back(obj);
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
    void save();
    /// @}
};
