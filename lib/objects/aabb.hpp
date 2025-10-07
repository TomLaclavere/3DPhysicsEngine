/**
 * @file aabb.hpp
 * @brief Axis-Aligned Bounding Box (AABB) object used in physics simulation.
 *
 * An AABB is constructed with a position (its center) and a vector giving its size along the three Cartesian
 * axis. It inherit from Object.
 */
#pragma once
#include "aabb.hpp"
#include "object.hpp"
#include "precision.hpp"

// Forward declaration
struct Sphere;
struct Plane;

/**
 * @class AABB
 * @brief Represent an Axis-Aligned Bounding Box (AABB) for physics simulation.
 *
 * Inherits from @ref Object and represents a box aligned with the X, Y, Z axes.
 * An AABB is defined by its center position and size (width, height, depth).
 *
 * The AABB supports collision detection with other AABBs, Spheres, and Planes.
 */
struct AABB : public Object
{
private:
public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    AABB() = default;
    explicit AABB(const Vector3D& position)
        : Object(position)
    {}
    AABB(const Vector3D& position, const Vector3D& size)
        : Object(position, size)
    {}
    AABB(const Vector3D& position, const Vector3D& size, decimal mass)
        : Object(position, size, mass)
    {}
    AABB(const Vector3D& position, const Vector3D& rotation, const Vector3D& size, const Vector3D& velocity,
         const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : Object(position, rotation, size, velocity, acceleration, force, torque, mass)
    {}
    virtual ~AABB() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType getType() const override { return ObjectType::AABB; }
    Vector3D   getHalfExtents() const { return getSize() * 0.5_d; }
    /// Return the minimum corner of the AABB.
    Vector3D getMin() const { return getPosition() - getHalfExtents(); }
    /// Return the maximum corner of the AABB.
    Vector3D getMax() const { return getPosition() + getHalfExtents(); }
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// Check collision between two AABBs.
    bool aabbCollision(const AABB& aabb);
    /// Check collision between an AABB and a Sphere.
    bool aabbSphereCollision(const Sphere& sphere);
    /// Check collisoin between an AABB and a Plane
    bool aabbPlaneCollision(const Plane& plane);
    /// Check collision with another Object.
    virtual bool checkCollision(const Object& other) override;
    /// @}
};
