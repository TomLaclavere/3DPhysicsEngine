/**
 * @file plane.hpp
 * @brief Definition of the Plane object used in physics simulation.
 *
 * A plane is constructed with a position (the center of its two diagonals), its normal vector, and its size
 * along two tangent axes computed from the normal direction. It inherit from Object.
 */

#pragma once
#include "object.hpp"
#include "precision.hpp"

// Forward declaration
struct Sphere;
struct AABB;

/**
 * @class Plane
 * @brief Represents a planar object for physics simulation.
 *
 * Inherits from @ref Object and represents a finite plane.
 * Plane is defined by its position (center), its normal direction, and the size along
 * the two tangent axis computed from normal direction.
 *
 * The Plane supports collision detection with other Planes, Spheres, and AABBs.
 */
struct Plane : public Object
{
private:
    Vector3D normal = Vector3D(0_d, 0_d, 1_d); // Normal direction (unit vector)
    Vector3D u;                                // Tangent axis 1 (unit vector)
    Vector3D v;                                // Tangent axis 2 (unit vector)
    decimal  halfWidth  = 1_d;                 // Half extent along u
    decimal  halfHeight = 1_d;                 // Half extent along v

public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    Plane() { updateLocalAxes(); }

    explicit Plane(const Vector3D& position)
        : Object(position)
    {
        updateLocalAxes();
    }

    Plane(const Vector3D& position, const Vector3D& _normal)
        : Object(position)
        , normal(_normal.getNormalised())
    {

        updateLocalAxes();
    }

    Plane(const Vector3D& position, const Vector3D& size, const Vector3D& _normal)
        : Object(position, size)
        , normal(_normal.getNormalised())
        , halfWidth { size[0] * 0.5_d }
        , halfHeight { size[1] * 0.5_d }
    {
        updateLocalAxes();
    }

    Plane(const Vector3D& position, const Vector3D& size, decimal mass, const Vector3D& _normal)
        : Object(position, size, mass)
        , normal(_normal.getNormalised())
        , halfWidth { size[0] * 0.5_d }
        , halfHeight { size[1] * 0.5_d }
    {
        updateLocalAxes();
    }

    virtual ~Plane() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType      getType() const override { return ObjectType::Plane; }
    const Vector3D& getNormal() const { return normal; }
    const Vector3D& getU() const { return u; }
    const Vector3D& getV() const { return v; }
    decimal         getHalfWidth() const { return halfWidth; }
    decimal         getHalfHeight() const { return halfHeight; }
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{

    /// Set normal axis and recompute the two tangent axes.
    void setNormal(const Vector3D& n)
    {
        normal = n.getNormalised();
        updateLocalAxes();
    }
    void setHalfWidth(decimal halfwidth) { halfWidth = halfwidth; }
    void setHalfHeight(decimal halfheight) { halfHeight = halfheight; }
    /// Set half-width and half-height.
    void setSize(decimal halfwidth, decimal halfheight)
    {
        halfWidth  = halfwidth;
        halfHeight = halfheight;
    }
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// Compute local tangent axes based on the normal.
    void updateLocalAxes();
    /// Project a point onto the plane surface.
    Vector3D projectPoint(const Vector3D& point) const;
    /// Check if a projected point lies within the finite rectangle.
    bool containsPoint(const Vector3D& point) const;
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// Check collision between two Planes.
    bool planeCollision(const Plane& other);
    /// Check collision between a Plane and a Sphere.
    bool planeSphereCollision(const Sphere& sphere);
    /// Check collision between a Plane and an AABB.
    bool planeAABBCollision(const AABB& aabb);
    /// Check collision with another Object.
    bool checkCollision(const Object& other) override;
    /// @}
};
