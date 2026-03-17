#include "collision/contact.hpp"
#include "mathematics/vector.hpp"
#include "objects/object.hpp"

namespace Physics {

// ============================================================================
/// @name Helpers
// ============================================================================
/// @{

decimal reducedMass(decimal m1, decimal m2);
decimal effectiveYoung(decimal k1, decimal k2);
decimal effectiveDamping(decimal d1, decimal d2);
decimal effectiveFriction(decimal mu1, decimal mu2);
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
/// Compute spring (Hooke) force between two objects in collision.
Vector3D computeSpringForce(const Object& obj1, const Object& obj2, Contact& contact);
/// Compute damping (dissipation) force between two objects in collision.
Vector3D computeDampingForce(const Object& obj1, const Object& obj2, Contact& contact);
/// Compute normal forces : spring + damping.
Vector3D computeNormalForces(const Object& obj1, const Object& obj2, Contact& contact);
/// Compute friction (Coulomb) force between two objects in collision.
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2, Contact& contact);
/// Compute contacts forces : spring + damping + friction.
Vector3D computeContactForce(const Object& obj1, const Object& obj2, Contact& contact);
/// @}
} // namespace Physics
