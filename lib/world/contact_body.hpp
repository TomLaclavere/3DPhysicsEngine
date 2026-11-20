#pragma once
#include "mathematics/vector.hpp"
#include "objects/object.hpp"

struct ContactBody
{
    Vector3D position;
    Vector3D velocity;
    decimal  mass;    // 0 for fixed bodies
    decimal  invMass; // mass > 0 ? 1 / mass : 0
    decimal  stiffness;
    decimal  damping;
    decimal  friction;
    decimal  restitution;
};

struct ContactResponse
{
    Vector3D positionA, positionB;
    Vector3D velocityA, velocityB;
};

inline ContactBody contactBodyOf(const Object& obj)
{
    const decimal m = obj.getMass();
    return { .position    = obj.getPosition(),
             .velocity    = obj.getVelocity(),
             .mass        = m,
             .invMass     = m > 0_d ? 1_d / m : 0_d,
             .stiffness   = obj.getStiffnessCst(),
             .damping     = obj.getDampingCst(),
             .friction    = obj.getFrictionCst(),
             .restitution = obj.getRestitutionCst() };
}