/**
 * @file object.hpp
 * @brief Object class representing physical entities in the simulation.
 *
 * This class encapsulates the properties and behaviors of objects within the physics simulation,
 * including their position, rotation, scale, and physical forces acting upon them.
 * It serves as a base class for specific object types like Sphere, AABB, and Plane.
 */
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

/**
 * @brief Object class representing a physical entity in the simulation.
 *
 * Holds physical intrinsics properties like mass (`decimal`) or scale (`Vector3D`), as well as dynamic
 * properties: position, rotation, velocity, acceleration, forces, and torques (`Vector3D`).
 * Can be extended for specific object types (e.g., Sphere, AABB, Plane).
 *
 */
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
    /// @brief Constructions can be done with various levels of details.
    /// Default values are zero vectors for all `Vector3D` properties, except `scale` which defaults to
    /// (1,1,1). Mass defaults to 0.0.
    // ============================================================================
    /// @name Constructors / Destructors
    // ============================================================================
    /// @{

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
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    Vector3D           getPosition() const { return position; }
    Vector3D           getRotation() const { return rotation; }
    Vector3D           getScale() const { return scale; }
    Vector3D           getVelocity() const { return velocity; }
    Vector3D           getAcceleration() const { return acceleration; }
    Vector3D           getForce() const { return force; }
    Vector3D           getTorque() const { return torque; }
    decimal            getMass() const { return mass; }
    virtual ObjectType getType() const { return ObjectType::Generic; }
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setPosition(const Vector3D& _position) { position = _position; }
    void setRotation(const Vector3D& _rotation) { rotation = _rotation; }
    void setScale(const Vector3D& _scale) { scale = _scale; }
    void setVelocity(const Vector3D& _velocity) { velocity = _velocity; }
    void setAcceleration(const Vector3D& _acceleration) { acceleration = _acceleration; }
    void setForce(const Vector3D& _force) { force = _force; }
    void setTorque(const Vector3D& _torque) { torque = _torque; }
    void setMass(const decimal _mass) { mass = _mass; }
    /// @}

    // ============================================================================
    /// @name Transformations
    // ============================================================================
    /// @{
    void applyTranslation(const Vector3D& v_translation) { position += v_translation; }
    void applyRotation(const Vector3D& v_rotation) { rotation += v_rotation; }
    void applyScaling(const Vector3D& v_scaling) { scale += v_scaling; }
    /// @}

    // ============================================================================
    /// @name Physics
    // ============================================================================
    /// @{
    void applyForce(const Vector3D& _force) { force += _force; }
    void applyTorque(const Vector3D& _torque) { torque += _torque; }
    /// Integrate motion equations over a time step `dt` to update physical properties.
    virtual void integrate(decimal dt);
    /// @}

    // ============================================================================
    /// @name Collision
    // ============================================================================
    /// @{

    /// @brief Check for collision with another object.
    /// This is a pure virtual functions that must be implemented by derived classes.
    virtual bool checkCollision(const Object& other) = 0; // Pure virtual
    /// @}
};
