#include "mathematics/vector.hpp"
#include "objects/object.hpp"

namespace Physics {
Vector3D computeGravityAcc(decimal g);
Vector3D computeGravityForce(decimal g, const Object& obj);
decimal  computeStiffnessRel(const Object& obj1, const Object& obj2);
Vector3D computeSpringForce(const Object& obj1, const Object& obj2);
Vector3D computeSpringForce(const Object& obj1, const Object& obj2, decimal k);
Vector3D computeDampingForce(const Object& obj1, const Object& obj2);
Vector3D computeDampingForce(const Object& obj1, const Object& obj2, decimal e);
Vector3D computeDampingForce(const Object& obj1, const Object& obj2, decimal e, decimal k);
Vector3D computeNormalForce(const Object& obj1, const Object& obj2);
Vector3D computeNormalForce(const Object& obj1, const Object& obj2, decimal e, decimal k);
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2);
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu);
Vector3D computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu, decimal e, decimal k);
Vector3D computeContactForce(const Object& obj1, const Object& obj2);
Vector3D computeContactForce(const Object& obj1, const Object& obj2, decimal mu, decimal e, decimal k);

} // namespace Physics
