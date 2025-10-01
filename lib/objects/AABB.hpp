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

/**
 * @brief Axis-Aligned Bounding Box (AABB).
 *
 * Inherits from @ref Object and represents a box aligned with the X, Y, Z axes.
 * Defined by its center position and scale (width, height, depth).
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
    AABB(const Vector3D& position, const Vector3D& scale)
        : Object(position, scale)
    {}
    AABB(const Vector3D& position, const Vector3D& scale, decimal mass)
        : Object(position, scale, mass)
    {}
    AABB(const Vector3D& position, const Vector3D& rotation, const Vector3D& scale, const Vector3D& velocity,
         const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : Object(position, rotation, scale, velocity, acceleration, force, torque, mass)
    {}
    virtual ~AABB() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType get_type() const override { return ObjectType::AABB; }
    /// Return the minimum corner of the AABB.
    Vector3D get_min() const { return get_position() - get_scale() * 0.5; }
    /// Return the maximum corner of the AABB.
    Vector3D get_max() const { return get_position() + get_scale() * 0.5; }
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// Check collision between two AABBs.
    bool aabb_collision(const AABB& a, const AABB& b);
    /// Check collision between an AABB and a Sphere.
    bool AABB_sphere_collision(const AABB& AABB, const Sphere& sphere);
    /// Check collision with another Object.
    virtual bool check_collision(const Object& other) override;
    /// @}
};
