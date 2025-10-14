#include "mathematics/vector.hpp"
#include "objects/object.hpp"

namespace Physics {

// ============================================================================
/// @name Helpers
// ============================================================================
/// @{

decimal reducedMass(decimal m1, decimal m2);
decimal effectiveStiffness(decimal k1, decimal k2);
decimal dampingRatioFromRestitution(decimal e);
/// @}

// ============================================================================
/// @name Forces
// ============================================================================
/// @{
/// Compute gravitational acceleration vector.
Vector3D computeGravityAcc(decimal g);
/// Compute gravitational force vector for an Object.
Vector3D computeGravityForce(decimal g, const Object& obj);

Vector3D computeSpringForce(const Object& obj1, const Object& obj2);
/// Compute Hooke spring force with effective stiffness = k.
Vector3D computeSpringForce(const Object& obj1, const Object& obj2, decimal k);

Vector3D computeDampingForce(const Object& obj1, const Object& obj2);
/// Compute damping force with effective restitution = e.
Vector3D computeDampingForce(const Object& obj1, const Object& obj2, decimal e);
/// Compute damping force with effective restitution = e and effective stiffness = k.
Vector3D computeDampingForce(const Object& obj1, const Object& obj2, decimal e, decimal k);
/// Compute normal forces (spring + damping).
Vector3D computeNormalForce(const Object& obj1, const Object& obj2);
/// Compute normal forces (spring + damping) with effective restitution = e and effective stiffness = k.
Vector3D computeNormalForce(const Object& obj1, const Object& obj2, decimal e, decimal k);
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2);
/// Compute friction force with friction coefficient = mu.
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu);
/// Compute friction force with friction coefficient = mu, effective restitution = e and effective stiffness =
/// k.
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu, decimal e, decimal k);
/// Compute all contact forces : spring, damping and friction.
Vector3D computeContactForce(const Object& obj1, const Object& obj2);
/// Compute all contact forces : spring, damping and friction with friction coefficient = mu, effective
/// restitution = e and effective stiffness = k.
Vector3D computeContactForce(const Object& obj1, const Object& obj2, decimal mu, decimal e, decimal k);
/// @}
} // namespace Physics
