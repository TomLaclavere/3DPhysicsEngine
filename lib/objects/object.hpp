/**
 * @file object.hpp
 * @brief Object class representing physical entities in the simulation.
 *
 * This class encapsulates the properties and behaviors of objects within the physics simulation,
 * including their position, rotation, size, and physical forces acting upon them.
 * It serves as a base class for specific object types like Sphere, AABB, and Plane.
 */
#pragma once
#include "mathematics/vector.hpp"
#include "ostream"
#include "precision.hpp"

enum class ObjectType
{
    Generic,
    Sphere,
    AABB,
    Plane
};

inline std::string toString(ObjectType type)
{
    switch (type)
    {
    case ObjectType::Generic:
        return "Generic";
    case ObjectType::Sphere:
        return "Sphere";
    case ObjectType::Plane:
        return "Plane";
    case ObjectType::AABB:
        return "AABB";
    default:
        return "Unknown";
    }
}

/**
 * @brief Object class representing a physical entity in the simulation.
 *
 * Holds physical intrinsics properties like mass (`decimal`) or size (`Vector3D`), as well as dynamic
 * properties: position, rotation, velocity, acceleration, forces, and torques (`Vector3D`).
 * Can be extended for specific object types (e.g., Sphere, AABB, Plane).
 *
 */
struct Object
{
private:
    Vector3D position       = Vector3D();
    Vector3D rotation       = Vector3D();
    Vector3D size           = Vector3D(1_d, 1_d, 1_d);
    Vector3D velocity       = Vector3D();
    Vector3D acceleration   = Vector3D();
    Vector3D force          = Vector3D();
    Vector3D torque         = Vector3D();
    decimal  mass           = 0_d; // static by default
    decimal  stiffnessCst   = 0_d;
    decimal  restitutionCst = 0_d;
    decimal  frictionCst    = 0_d;

    bool fixed = true;

public:
    /// @brief Constructions can be done with various levels of details.
    /// Default values are zero vectors for all `Vector3D` properties, except `size` which defaults to
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
    Object(const Vector3D& position, const Vector3D& size)
        : position { position }
        , size { size }
    {}
    Object(const Vector3D& position, const Vector3D& size, decimal mass)
        : position { position }
        , size { size }
        , mass { mass }
    {
        checkFixed();
    }
    Object(const Vector3D& position, const Vector3D& size, const Vector3D& velocity, decimal mass)
        : position { position }
        , size { size }
        , velocity { velocity }
        , mass { mass }
    {
        checkFixed();
    }
    Object(const Vector3D& position, const Vector3D& rotation, const Vector3D& size, const Vector3D& velocity,
           const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
        : position { position }
        , rotation { rotation }
        , size { size }
        , velocity { velocity }
        , acceleration { acceleration }
        , force { force }
        , torque { torque }
        , mass { mass }
    {
        checkFixed();
    }
    virtual ~Object() = default;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    Vector3D           getPosition() const;
    Vector3D           getRotation() const;
    Vector3D           getSize() const;
    Vector3D           getVelocity() const;
    Vector3D           getAcceleration() const;
    Vector3D           getForce() const;
    Vector3D           getTorque() const;
    decimal            getMass() const;
    decimal            getStiffnessCst() const;
    decimal            getRestitutionCst() const;
    decimal            getFrictionCst() const;
    virtual ObjectType getType() const;
    bool               getIsFixed() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setPosition(const Vector3D& _position);
    void setRotation(const Vector3D& _rotation);
    void setSize(const Vector3D& _size);
    void setVelocity(const Vector3D& _velocity);
    void setAcceleration(const Vector3D& _acceleration);
    void setForce(const Vector3D& _force);
    void setTorque(const Vector3D& _torque);
    void setMass(const decimal _mass);
    void setIsFixed(bool b) { fixed = b; };
    void setRestitutionCst(decimal e) { restitutionCst = e; }
    /// @}

    // ============================================================================
    /// @name Transformations
    // ============================================================================
    /// @{
    void addAcceleration(const Vector3D& acc) { acceleration += acc; }
    void applyTranslation(const Vector3D& v_translation) { position += v_translation; }
    void applyRotation(const Vector3D& v_rotation);
    /**
     * @brief Applies scaling to the object.
     *
     * For certain object types (e.g., spheres), non-uniform scaling is not permitted
     * as it may violate geometric assumptions. For such objects, only uniform scaling
     * (where all components of v_scaling are equal) is allowed.
     *
     * @param v_scaling The scaling vector to apply.
     */
    void applyScaling(const Vector3D& v_scaling) { size *= v_scaling; }
    /// @}

    // ============================================================================
    /// @name Physics
    // ============================================================================
    /// @{
    void checkFixed()
    {
        if (mass <= 0_d)
            fixed = true;
        else
            fixed = false;
    }
    bool isFixed() const { return fixed; }
    void resetForces()
    {
        force.setToNull();
        torque.setToNull();
    }
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

inline std::ostream& operator<<(std::ostream& os, ObjectType type) { return os << toString(type); }
