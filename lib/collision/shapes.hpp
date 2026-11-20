#pragma once

#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "precision.hpp"

struct SphereShape
{
    Vector3D center;
    decimal  radius;
};

struct AABBShape
{
    Vector3D center;
    Vector3D halfExtents;

    Vector3D getMin() const { return center - halfExtents; }
    Vector3D getMax() const { return center + halfExtents; }
};

struct PlaneShape
{
    Vector3D center, normal, u, v;
    decimal  halfWidth, halfHeight;
    decimal  boundingHalfSize; // (0.5 * size).getMax(), used by the broad phase
};

// Object wrappers
inline SphereShape shapeOf(const Sphere& s) { return { .center = s.getCenter(), .radius = s.getRadius() }; }
inline AABBShape   shapeOf(const AABB& a)
{
    return { .center = a.getPosition(), .halfExtents = a.getHalfExtents() };
}
inline PlaneShape shapeOf(const Plane& p)
{
    return { .center           = p.getPosition(),
             .normal           = p.getNormal(),
             .u                = p.getU(),
             .v                = p.getV(),
             .halfWidth        = p.getHalfWidth(),
             .halfHeight       = p.getHalfHeight(),
             .boundingHalfSize = (0.5 * p.getSize()).getMax() };
}