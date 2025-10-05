#pragma once
#include "object.hpp"
#include "precision.hpp"
#include "sphere.hpp"

// Forward declaration
struct Sphere;
struct AABB;
struct Plane;

struct Plane : public Object
{
private:
public:
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{
    Plane() = default;
    explicit Plane(const Vector3D& position)
        : Object(position)
    {}
    Plane(const Vector3D& position, const Vector3D& size)
        : Object(position, size)
    {}
    Plane(const Vector3D& position, const Vector3D& size, decimal mass)
        : Object(position, size, mass)
    {}
    Plane(const Vector3D& position, const Vector3D& rotation, const Vector3D& size, const Vector3D& velocity,
          const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : Object(position, rotation, size, velocity, acceleration, force, torque, mass)
    {}
    virtual ~Plane() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    ObjectType getType() const override { return ObjectType::Plane; }
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// Check collision between two Planes.
    bool planeCollision(const Plane& b);
    /// Check collision between a Plane and a Sphere.
    bool planeSphereCollision(const Sphere& sphere);
    /// Check collision between a Plane and an AABB
    bool planeAABBCollision(const Sphere& sphere);
    /// Check collision with another Object.
    virtual bool checkCollision(const Object& other) override;
    /// @}
};
