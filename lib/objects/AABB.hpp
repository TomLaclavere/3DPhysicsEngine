#pragma once
#include "object.hpp"
#include "precision.hpp"
#include "sphere.hpp"

struct AABB : public Object
{
private:
    Vector3D position;
    Vector3D rotation;
    Vector3D scale;
    Vector3D velocity;
    Vector3D acceleration;
    Vector3D force;
    Vector3D torque;
    decimal  mass;

public:
    // ============================================================================
    // ============================================================================
    //  Constructors / Destructors
    // ============================================================================
    // ============================================================================
    AABB(const Vector3D& position = Vector3D(), const Vector3D& rotation = Vector3D(),
         const Vector3D& scale = Vector3D(1, 1, 1), const Vector3D& velocity = Vector3D(),
         const Vector3D& acceleration = Vector3D(), const Vector3D& force = Vector3D(),
         const Vector3D& torque = Vector3D(), decimal mass = 0.0)
        : Object(position, rotation, scale, velocity, acceleration, force, torque, mass)
    {}

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
