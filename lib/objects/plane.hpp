#pragma once
#include "aabb.hpp"
#include "object.hpp"
#include "precision.hpp"
#include "sphere.hpp"

struct Sphere;
struct AABB;

struct Plane : public Object
{
private:
    Vector3D normal = Vector3D(0_d, 1_d, 0_d); // Normal direction (unit vector)
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

    Plane(const Vector3D& position, const Vector3D& size)
        : Object(position, size)
    {
        halfWidth  = size[0] * 0.5_d;
        halfHeight = size[1] * 0.5_d;
        updateLocalAxes();
    }

    Plane(const Vector3D& position, const Vector3D& size, const Vector3D& _normal)
        : Object(position, size)
        , normal(_normal.getNormalised())
    {
        halfWidth  = size[0] * 0.5_d;
        halfHeight = size[1] * 0.5_d;
        updateLocalAxes();
    }

    Plane(const Vector3D& position, const Vector3D& size, decimal mass,
          const Vector3D& _normal = Vector3D(0_d, 1_d, 0_d))
        : Object(position, size, mass)
        , normal(_normal.getNormalised())
    {
        halfWidth  = size[0] * 0.5_d;
        halfHeight = size[1] * 0.5_d;
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
    void setNormal(const Vector3D& n)
    {
        normal = n.getNormalised();
        updateLocalAxes();
    }

    void setSize(const Vector3D& s)
    {
        halfWidth  = s[0] * 0.5_d;
        halfHeight = s[1] * 0.5_d;
    }
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// Update local tangent axes based on the normal.
    void updateLocalAxes();

    /// Project a world point onto the plane surface
    Vector3D projectPoint(const Vector3D& point) const;

    /// Check if a projected point lies within the finite rectangle
    bool containsPoint(const Vector3D& point) const;
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{
    bool planeCollision(const Plane& other);
    bool planeSphereCollision(const Sphere& sphere);
    bool planeAABBCollision(const AABB& aabb);
    bool checkCollision(const Object& other) override;
    /// @}
};
