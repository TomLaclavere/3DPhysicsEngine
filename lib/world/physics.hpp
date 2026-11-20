/**
 * @file physics.hpp
 * @brief Physics force computations used by @ref PhysicsWorld.
 *
 * Provides helpers for reduced mass / effective material constants, and
 * functions to compute gravity, spring/damping, friction, and combined
 * contact forces between pairs of objects.
 */
#pragma once

#include "collision/contact.hpp"
#include "contact_body.hpp"
#include "mathematics/vector.hpp"

namespace Physics {

// ============================================================================
/// @name Helpers
// ============================================================================
/// @{

decimal reducedMass(decimal m1, decimal m2);
decimal effectiveStiffness(decimal k1, decimal k2);
decimal effectiveDamping(decimal d1, decimal d2);
decimal effectiveFriction(decimal mu1, decimal mu2);
/// @}

// ============================================================================
/// @name Forces
// ============================================================================
/// @{
/// Compute gravitational acceleration vector.
Vector3D computeGravityAcc(decimal g);
/// Compute gravitational force vector for a body.
/// Every force below also has an `Object` overload, which reads the body with `contactBodyOf`.
Vector3D        computeGravityForce(decimal g, const ContactBody& obj);
inline Vector3D computeGravityForce(decimal g, const Object& obj)
{
    return computeGravityForce(g, contactBodyOf(obj));
}
/// Internal - Compute spring (Hooke) force between two objects in collision.
Vector3D        computeSpringForce(const ContactBody& obj1, const ContactBody& obj2, const Contact& contact);
inline Vector3D computeSpringForce(const Object& obj1, const Object& obj2, const Contact& contact)
{
    return computeSpringForce(contactBodyOf(obj1), contactBodyOf(obj2), contact);
}
/// Internal - Compute damping (dissipation) force between two objects in collision.
Vector3D        computeDampingForce(const ContactBody& obj1, const ContactBody& obj2, const Contact& contact);
inline Vector3D computeDampingForce(const Object& obj1, const Object& obj2, const Contact& contact)
{
    return computeDampingForce(contactBodyOf(obj1), contactBodyOf(obj2), contact);
}
/// Compute normal forces : spring + damping.
Vector3D        computeNormalForces(const ContactBody& obj1, const ContactBody& obj2, const Contact& contact);
inline Vector3D computeNormalForces(const Object& obj1, const Object& obj2, const Contact& contact)
{
    return computeNormalForces(contactBodyOf(obj1), contactBodyOf(obj2), contact);
}
/// Compute friction (Coulomb) force between two objects in collision.
Vector3D        computeFrictionForce(const ContactBody& obj1, const ContactBody& obj2, const Contact& contact,
                                     decimal F_normal_mag);
inline Vector3D computeFrictionForce(const Object& obj1, const Object& obj2, const Contact& contact,
                                     decimal F_normal_mag)
{
    return computeFrictionForce(contactBodyOf(obj1), contactBodyOf(obj2), contact, F_normal_mag);
}
/// Compute contacts forces : spring + damping + friction.
Vector3D        computeContactForce(const ContactBody& obj1, const ContactBody& obj2, const Contact& contact);
inline Vector3D computeContactForce(const Object& obj1, const Object& obj2, const Contact& contact)
{
    return computeContactForce(contactBodyOf(obj1), contactBodyOf(obj2), contact);
}
/// @}
} // namespace Physics
