/**
 * @file plane.hpp
 * @brief Definition of the Plane object used in physics simulation.
 *
 * A plane is constructed with a position (the center of its two diagonals), its normal vector, and its size
 * along two tangent axes computed from the normal direction. It inherit from Object.
 */

#pragma once
#include "object.hpp"

// Forward declaration
class Sphere;
class AABB;

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
class Plane : public Object
{
private:
    Vector3D u;           // Tangent axis 1 (unit vector)
    Vector3D v;           // Tangent axis 2 (unit vector)
    decimal  halfHeight = 1_d;
    decimal  halfWidth  = 1_d;

public:
    /// @name Constructors / Destructors
    /// @{

    Plane() { setNormal(Vector3D(0_d, 0_d, 1_d)); }

    explicit Plane(const Vector3D& position)
        : Object(position)
    {
        setNormal(Vector3D(0_d, 0_d, 1_d));
    }
    Plane(const Vector3D& position, const Vector3D& _normal)
        : Object(position)
    {
        setNormal(_normal);
    }
    Plane(const Vector3D& position, const Vector3D& size, const Vector3D& _normal)
        : Object(position, size)
        , halfHeight { size[0] * 0.5_d }
        , halfWidth { size[1] * 0.5_d }
    {
        setNormal(_normal);
    }
    Plane(const Vector3D& position, const Vector3D& size, decimal mass, const Vector3D& _normal)
        : Object(position, size, mass)
        , halfHeight { size[0] * 0.5_d }
        , halfWidth { size[1] * 0.5_d }
    {
        setNormal(_normal);
        checkFixed();
    }
    Plane(const Vector3D& position, const Vector3D& size, const Vector3D& velocity, decimal mass,
          const Vector3D& _normal)
        : Object(position, size, velocity, mass)
        , halfHeight { size[0] * 0.5_d }
        , halfWidth { size[1] * 0.5_d }
    {
        setNormal(_normal);
        checkFixed();
    }
    Plane(const Vector3D& position, const Vector3D& obj_normal, const Vector3D& size, const Vector3D& velocity,
          const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass,
          const Vector3D& _normal)
        : Object(position, obj_normal, size, velocity, acceleration, force, torque, mass)
        , halfHeight { size[0] * 0.5_d }
        , halfWidth { size[1] * 0.5_d }
    {
        setNormal(_normal);
        checkFixed();
    }

    virtual ~Plane() = default;
    /// @}

    /// @name Getters
    /// @{

    decimal         getVolume() const override;
    ObjectType      getType() const override;
    const Vector3D& getU() const;
    const Vector3D& getV() const;
    decimal         getHalfWidth() const;
    decimal         getHalfHeight() const;
    /// @}

    /// @name Setters
    /// @{

    void setMaterial(const Material& mat) override;
    /// Set normal axis and recompute the two tangent axes.
    void setNormal(const Vector3D& n) override
    {
        Object::setNormal(n.getNormalised());
        updateLocalAxes();
    }
    void setSize(const Vector3D& s) override
    {
        Object::setSize(s);
        halfHeight = s[0] * 0.5_d;
        halfWidth  = s[1] * 0.5_d;
    }
    void setHalfWidth(decimal halfwidth)
    {
        halfWidth = halfwidth;
        Vector3D s = getSize();
        Object::setSize(Vector3D(s[0], halfwidth * 2_d, s[2]));
    }
    void setHalfHeight(decimal halfheight)
    {
        halfHeight = halfheight;
        Vector3D s = getSize();
        Object::setSize(Vector3D(halfheight * 2_d, s[1], s[2]));
    }
    /// Set half-width and half-height.
    void setSize(decimal halfwidth, decimal halfheight)
    {
        halfWidth  = halfwidth;
        halfHeight = halfheight;
        Vector3D s = getSize();
        Object::setSize(Vector3D(halfheight * 2_d, halfwidth * 2_d, s[2]));
    }
    /// @}

    /// @name Utilities
    /// @{

    /// Compute local tangent axes based on the normal.
    void updateLocalAxes();
    /// Project a point onto the plane surface.
    Vector3D projectPoint(const Vector3D& point) const;
    /// Check if a projected point lies within the finite rectangle.
    bool containsPoint(const Vector3D& point) const;
    /// @}

    /// @name Collision
    /// @{

    /// Check broad collision between two Planes.
    bool checkPlaneCollision(const Plane& Plane);
    /// Check broad collision between a Plane and an AABB.
    bool checkPlaneAABBCollision(const AABB& aabb);
    /// Check broad collision between a Plane and a Plane
    bool checkPlaneSphereCollision(const Sphere& sphere);
    /// Check broad collision with another Object.
    bool checkCollision(const Object& other) override;

    /// Check narrow collision between two Planes.
    bool computePlaneCollision(const Plane& Plane, Contact& contact);
    /// Check narrow collision between a Plane and an AABB.
    bool computePlaneAABBCollision(const AABB& aabb, Contact& contact);
    /// Check narrow collision between a Plane and a Plane.
    bool computePlaneSphereCollision(const Sphere& sphere, Contact& contact);
    /// Check narrow collision with another Object.
    bool computeCollision(const Object& other, Contact& contact) override;
    /// @}
};
