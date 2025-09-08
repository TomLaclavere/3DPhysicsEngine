#pragma once
#include "object.hpp"
#include "precision.hpp"

struct AABB; // Forward declaration

struct Sphere : public Object
{
private:
public:
    // ============================================================================
    // ============================================================================
    //  Constructors / Destructors
    // ============================================================================
    // ============================================================================
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

    // ============================================================================
    // ============================================================================
    //  Getters
    // ============================================================================
    // ============================================================================
    ObjectType get_type() const override { return ObjectType::Sphere; }
    Vector3D   get_center() const { return get_position(); }
    decimal    get_radius() const { return get_scale()[0] * 0.5_d; }

    // ============================================================================
    // ============================================================================
    //  Collision
    // ============================================================================
    // ============================================================================
    bool         sphere_collision(const Sphere& a, const Sphere& b);
    static bool  AABB_sphere_collision(const Sphere& sphere, const AABB& aabb);
    virtual bool check_collision(const Object& other) override;
};
