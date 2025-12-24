/**
 * @file sphere.hpp
 * @brief Definition of the Sphere object used in physics simulation.
 *
 * A Sphere is constructed with a position (its center) and a uniform size corresponding to its diameter.
 * It inherits from Object.
 */

#pragma once
#include "object.hpp"

// Forward declaration
struct AABB;
struct Plane;

/**
 * @class Sphere
 * @brief Represents a spherical object for physics simulation.
 *
 * Inherits from @ref Object and represents a sphere.
 * Sphere is defined by its position (center) and its diameter, which is derived
 * from the size attribute of Object. By convention:
 * - `size.x == size.y == size.z`, enforced via constructors.
 * - The radius is computed as `size.x * 0.5`.
 *
 * The Sphere supports collision detection with other Spheres, AABBs, and Planes.
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
    {
        checkFixed();
    }
    Sphere(const Vector3D& position, const decimal& size, const Vector3D& velocity, decimal mass)
        : Object(position, Vector3D(size), velocity, mass)
    {
        checkFixed();
    }
    Sphere(const Vector3D& position, const Vector3D& rotation, const decimal& size, const Vector3D& velocity,
           const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : Object(position, rotation, Vector3D(size), velocity, acceleration, force, torque, mass)
    {
        checkFixed();
    }
    virtual ~Sphere() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType getType() const override;
    Vector3D   getCenter() const;
    decimal    getDiameter() const;
    decimal    getRadius() const;
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
