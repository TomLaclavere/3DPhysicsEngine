/**
 * @file physicsWorld.hpp
 * @brief Definition of the Physical World used in physics simulation.
 *
 */

#include "config.hpp"
#include "mathematics/common.hpp"
#include "objects/object.hpp"

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
    Config&              config;
    std::vector<Object*> objects;

    decimal  timeStep        = config.getTimeStep();
    bool     isRunning       = false;
    decimal  gravityConstant = config.getGravity();
    Vector3D gravityAcc      = computeGravity();

public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    PhysicsWorld() = default;
    PhysicsWorld(Config& _config)
        : config { _config }
    {}
    ~PhysicsWorld() { clearObjects(); };
    /// @}

    // ============================================================================
    /// @name Core simulation methods
    // ============================================================================
    /// @{

    /// Initialise PhysicalWorld
    void initialize();
    void start() { isRunning = true; }
    void stop() { isRunning = false; }
    /// Re-initialise PhysicsWorld
    void reset() { initialize(); }
    /// @}

    // ============================================================================
    /// @name Time step methods
    // ============================================================================
    /// @{

    /// @brief Compute physical world at time t + dt.
    /// @param dt Timestep parameter in second.
    void update(decimal dt);
    /// @brief Integrate using Euler integration method.
    void integrateEuler();
    /// @}

    // ============================================================================
    /// @name Force computation
    // ============================================================================
    /// @{

    /// Compute gravitational acceleration from gravity constant.
    Vector3D computeGravity();
    void     computeForce();
    /// Apply gravitational force to all Object.
    void applyGravity();
    void applyForces() { applyGravity(); }
    /// @}

    // ============================================================================
    /// @name Object management
    // ============================================================================
    /// @{

    /// @brief  Add Object in the PhysicaWorld.
    /// @param obj Object instance pointer.
    void addObject(Object* obj)
    {
        if (obj)
            objects.push_back(obj);
    }
    /// Clear Object array
    void    clearObjects() { objects.clear(); }
    size_t  getObjectCount() const { return objects.size(); }
    Object* getObject(size_t index) { return (index < objects.size()) ? objects[index] : nullptr; }
    /// @}

    // ============================================================================
    /// @name Printing & Saving
    // ============================================================================
    /// @{
    void print();
    void save();
};
