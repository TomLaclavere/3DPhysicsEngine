#pragma once
#include "object.hpp"
#include "precision.hpp"
#include "sphere.hpp"

struct AABB : public Object
{
private:
public:
    // ============================================================================
    // ============================================================================
    //  Constructors / Destructors
    // ============================================================================
    // ============================================================================
    AABB() = default;
    AABB(const Vector3D& position)
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

    // ============================================================================
    // ============================================================================
    //  Getters
    // ============================================================================
    // ============================================================================
    ObjectType get_type() const override { return ObjectType::AABB; }
    Vector3D   get_min() const { return get_position() - get_scale() * 0.5; }
    Vector3D   get_max() const { return get_position() + get_scale() * 0.5; }

    // ============================================================================
    // ============================================================================
    //  Collision
    // ============================================================================
    // ============================================================================
    bool         aabb_collision(const AABB& a, const AABB& b);
    bool         AABB_sphere_collision(const AABB& AABB, const Sphere& sphere);
    virtual bool check_collision(const Object& other) override;
};
