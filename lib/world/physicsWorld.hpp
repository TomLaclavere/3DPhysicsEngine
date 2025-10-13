/**
 * @file physicsWorld.hpp
 * @brief Definition of the Physical World used in physics simulation.
 *
 */

#include "mathematics/common.hpp"
#include "objects/object.hpp"
#include "world/config.hpp"
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

    bool     isRunning  = false;
    decimal  timeStep   = config.getTimeStep();
    decimal  gravityCst = config.getGravity();
    Vector3D gravityAcc = Physics::computeGravityAcc(gravityCst);

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
    void setTimeStep(decimal step);
    void setGravityCst(decimal g);
    void setGravityAcc(const Vector3D& acc);
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

    /// @brief Integrate using Euler integration method.
    void integrateEuler(Object& obj, decimal dt);
    /// @brief Compute physical world at time t + dt.
    /// @param dt Timestep parameter in second.
    void integrate(decimal dt);
    /// @}

    // ============================================================================
    /// @name Force computation
    // ============================================================================
    /// @{

    /// Apply gravitational force to all Object.
    void applyGravityForces();
    void applySpringForces(Object& obj, Object& other);
    void applyDamplingForces(Object& obj, Object& other);
    void applyFrictionForces(Object& obj, Object& other);
    void applyContactForces(Object& obj, Object& other);
    /// Avoid overlap by nullifying the acceleration and speed if collision occurs
    void avoidOverlap(Object& obj, Object& other);
    void applyForces();
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
    void removeObject(Object* obj)
    {
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
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
    void printState() const;
    void save();
};
