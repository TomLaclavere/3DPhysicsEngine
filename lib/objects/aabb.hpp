/**
 * @file aabb.hpp
 * @brief Axis-Aligned Bounding Box (AABB) object for collision detection.
 *
 * Provides an implementation of an Object that represents a 3D box aligned
 * with the world axes. Used for fast collision detection against other AABBs
 * and spheres.
 */
#pragma once
#include "object.hpp"
#include "precision.hpp"
#include "sphere.hpp"

// Forward declaration
struct Sphere;
struct AABB;
struct Plane;

/**
 * @brief Axis-Aligned Bounding Box (AABB).
 *
 * Inherits from @ref Object and represents a box aligned with the X, Y, Z axes.
 * Defined by its center position and size (width, height, depth).
 *
 * Example usage:
 * @code
 * AABB box(Vector3D(0,0,0), Vector3D(2,2,2)); // Cube centered at origin, size 2
 * Sphere sphere(Vector3D(1,0,0), 1.0_d);
 * bool collision = box.check_collision(sphere);
 * @endcode
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
    bool aabbPlaneCollision(const Sphere& sphere);
    /// Check collision with another Object.
    virtual bool checkCollision(const Object& other) override;
    /// @}
};
