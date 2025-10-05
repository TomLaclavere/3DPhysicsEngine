/**
 * @file sphere.hpp
 * @brief Definition of the Sphere object used in physics simulation.
 *
 * A Sphere is represented as a position (its center) and a uniform size corresponding to its diameter. It
 * inherits from Object and integrates into the polymorphic collision system.
 */

#pragma once
#include "aabb.hpp"
#include "object.hpp"
#include "plane.hpp"
#include "precision.hpp"

struct AABB; ///< Forward declaration for AABB-Sphere collision functions.

/**
 * @class Sphere
 * @brief Represents a spherical object for physics simulation.
 *
 * A Sphere is defined by its center (position) and its radius, which is derived
 * from the size attribute of Object. By convention:
 * - `size.x == size.y == size.z`, enforced via constructors.
 * - The radius is computed as `size.x * 0.5`.
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
    explicit Sphere(const Vector3D& position)
        : Object(position)
    {}
    Sphere(const Vector3D& position, const decimal& size)
        : Object(position, Vector3D(size))
    {}
    Sphere(const Vector3D& position, const decimal& size, decimal mass)
        : Object(position, Vector3D(size), mass)
    {}
    Sphere(const Vector3D& position, const Vector3D& rotation, const decimal& size, const Vector3D& velocity,
           const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : Object(position, rotation, Vector3D(size), velocity, acceleration, force, torque, mass)
    {}
    virtual ~Sphere() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType getType() const override { return ObjectType::Sphere; }
    Vector3D   getCenter() const { return getPosition(); }
    decimal    getDiameter() const { return getSize().getX(); }
    decimal    getRadius() const { return getDiameter() * 0.5_d; }
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// Check collision between two Spheres.
    bool sphereCollision(const Sphere& sphere);
    /// Check collision between a Sphere and an AABB.
    bool sphereAABBCollision(const AABB& aabb);
    /// Check collision between a Sphere and a Plane
    bool spherePlaneCollision(const Plane& plane);
    /// Check collision with another Object.
    virtual bool checkCollision(const Object& other) override;
    /// @}
};
