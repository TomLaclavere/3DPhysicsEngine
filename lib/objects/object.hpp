#pragma once
#include "mathematics/vector.hpp"
#include "precision.hpp"

enum class ObjectType
{
    Generic,
    Sphere,
    AABB,
    Plane
};

struct Object
{
private:
    Vector3D position     = Vector3D();
    Vector3D rotation     = Vector3D();
    Vector3D scale        = Vector3D(1_d, 1_d, 1_d);
    Vector3D velocity     = Vector3D();
    Vector3D acceleration = Vector3D();
    Vector3D force        = Vector3D();
    Vector3D torque       = Vector3D();
    decimal  mass         = 0.0_d;

public:
    // ============================================================================
    // ============================================================================
    //  Constructors / Destructors
    // ============================================================================
    // ============================================================================
    Object() = default;
    Object(decimal mass)
        : mass { mass }
    {}
    Object(const Vector3D& position)
        : position { position }
    {}
    Object(const Vector3D& position, const Vector3D& scale)
        : position { position }
        , scale { scale }
    {}
    Object(const Vector3D& position, const Vector3D& scale, decimal mass)
        : position { position }
        , scale { scale }
        , mass { mass }
    {}
    Object(const Vector3D& position, const Vector3D& rotation, const Vector3D& scale,
           const Vector3D& velocity, const Vector3D& acceleration, const Vector3D& force,
           const Vector3D& torque, decimal mass)
        : position { position }
        , rotation { rotation }
        , scale { scale }
        , velocity { velocity }
        , acceleration { acceleration }
        , force { force }
        , torque { torque }
        , mass { mass }
    {}
    virtual ~Object() = default;

    // ============================================================================
    // ============================================================================
    //  Getters
    // ============================================================================
    // ============================================================================
    Vector3D           get_position() const { return position; }
    Vector3D           get_rotation() const { return rotation; }
    Vector3D           get_scale() const { return scale; }
    Vector3D           get_velocity() const { return velocity; }
    Vector3D           get_acceleration() const { return acceleration; }
    Vector3D           get_force() const { return force; }
    Vector3D           get_torque() const { return torque; }
    decimal            get_mass() const { return mass; }
    virtual ObjectType get_type() const { return ObjectType::Generic; }

    // ============================================================================
    // ============================================================================
    //  Setters
    // ============================================================================
    // ============================================================================
    void set_position(const Vector3D& _position) { position = _position; }
    void set_rotation(const Vector3D& _rotation) { rotation = _rotation; }
    void set_scale(const Vector3D& _scale) { scale = _scale; }
    void set_velocity(const Vector3D& _velocity) { velocity = _velocity; }
    void set_acceleration(const Vector3D& _acceleration) { acceleration = _acceleration; }
    void set_force(const Vector3D& _force) { force = _force; }
    void set_torque(const Vector3D& _torque) { torque = _torque; }
    void set_mass(const decimal _mass) { mass = _mass; }

    // ============================================================================
    // ============================================================================
    //  Transformations
    // ============================================================================
    // ============================================================================
    void apply_translation(const Vector3D& v_translation) { position += v_translation; }
    void apply_rotation(const Vector3D& v_rotation) { rotation += v_rotation; }
    void apply_scaling(const Vector3D& v_scaling) { scale += v_scaling; }

    // ============================================================================
    // ============================================================================
    //  Physics
    // ============================================================================
    // ============================================================================
    void         apply_force(const Vector3D& _force) { force += _force; }
    void         apply_torque(const Vector3D& _torque) { torque += _torque; }
    virtual void integrate(decimal dt);

    // ============================================================================
    // ============================================================================
    //  Collision
    // ============================================================================
    // ============================================================================
    virtual bool check_collision(const Object& other) = 0; // Pure virtual
};
