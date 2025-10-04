#pragma once

#include "config.hpp"
#include "mathematics/common.hpp"
#include "objects/object.hpp"

#include <vector>

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
    void initialize();
    void start() { isRunning = true; }
    void stop() { isRunning = false; }
    void reset() { initialize(); }
    /// @}

    // ============================================================================
    /// @name Time step methods
    // ============================================================================
    /// @{
    void update(decimal dt);
    void fixedUpdate();
    /// @}

    // ============================================================================
    /// @name Force computation
    // ============================================================================
    /// @{
    Vector3D computeGravity();
    void     computeForce();
    void     applyGravity();
    void     applyForces() { applyGravity(); }
    /// @}

    // ============================================================================
    /// @name Object management
    // ============================================================================
    /// @{
    void addObject(Object* obj)
    {
        if (obj)
            objects.push_back(obj);
    }
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
