/**
 * @file sphere.hpp
 * @brief Definition of the Sphere object used in physics simulation.
 *
 * A Sphere is represented as a position (its center) and a uniform scale
 * (used internally to compute its radius). It inherits from Object and
 * integrates into the polymorphic collision system.
 */

#pragma once
#include "object.hpp"
#include "precision.hpp"

struct AABB; ///< Forward declaration for AABB-Sphere collision functions.

/**
 * @class Sphere
 * @brief Represents a spherical object for physics simulation.
 *
 * A Sphere is defined by its center (position) and its radius, which is derived
 * from the scale attribute of Object. By convention:
 * - `scale.x == scale.y == scale.z`, enforced via constructors.
 * - The radius is computed as `scale.x * 0.5`.
 *
 * The Sphere supports collision detection with other Spheres and AABBs.
 */
struct Sphere : public Object
{
private:
public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    Sphere() = default;
    Sphere(const Vector3D& position)
        : Object(position)
    {}
    Sphere(const Vector3D& position, const decimal& scale)
        : Object(position, Vector3D(scale))
    {}
    Sphere(const Vector3D& position, const decimal& scale, decimal mass)
        : Object(position, Vector3D(scale), mass)
    {}
    Sphere(const Vector3D& position, const Vector3D& rotation, const decimal& scale, const Vector3D& velocity,
           const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : Object(position, rotation, Vector3D(scale), velocity, acceleration, force, torque, mass)
    {}
    virtual ~Sphere() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType get_type() const override { return ObjectType::Sphere; }
    Vector3D   get_center() const { return get_position(); }
    decimal    get_radius() const { return get_scale()[0] * 0.5_d; }
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// Check collision between two Spheres.
    bool sphere_collision(const Sphere& a, const Sphere& b);
    /// Check collision between a Sphere and an AABB.
    static bool AABB_sphere_collision(const Sphere& sphere, const AABB& aabb);
    /// Check collision with another Object.
    virtual bool check_collision(const Object& other) override;
    /// @}
};
