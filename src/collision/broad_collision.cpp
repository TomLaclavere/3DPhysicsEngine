#include "collision/broad_collision.hpp"

#include "mathematics/common.hpp"

#include <algorithm>

// ============================================================================
//  Sphere vs Sphere
// ============================================================================
bool BroadCollision::isColliding(const Sphere& sphere1, const Sphere& sphere2)
{
    Vector3D d = sphere2.getCenter() - sphere1.getCenter();
    decimal  r = sphere1.getRadius() + sphere2.getRadius();
    return commonMaths::approxSmallerOrEqualThan(d.getNormSquare(), r * r);
}

// ============================================================================
//  Sphere vs AABB
// ============================================================================
bool BroadCollision::isColliding(const Sphere& sphere, const AABB& aabb)
{
    decimal aabbRadius = aabb.getHalfExtents().getNorm(); // demi-diagonale
    decimal dist       = (sphere.getCenter() - aabb.getPosition()).getNorm();
    return commonMaths::approxSmallerOrEqualThan(dist, aabbRadius + sphere.getRadius());
}

// ============================================================================
//  Sphere vs Plane
// ============================================================================
bool BroadCollision::isColliding(const Sphere& sphere, const Plane& plane)
{
    Vector3D n    = plane.getNormal().getNormalised();
    decimal  dist = (sphere.getCenter() - plane.getPosition()).dotProduct(n);
    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist),
                                                 sphere.getRadius() + (0.5 * plane.getSize()).getMax());
}

// ============================================================================
//  AABB vs AABB
// ============================================================================
bool BroadCollision::isColliding(const AABB& aabb1, const AABB& aabb2)
{
    return !(aabb1.getMax().getX() < aabb2.getMin().getX() || aabb1.getMin().getX() > aabb2.getMax().getX() ||
             aabb1.getMax().getY() < aabb2.getMin().getY() || aabb1.getMin().getY() > aabb2.getMax().getY() ||
             aabb1.getMax().getZ() < aabb2.getMin().getZ() || aabb1.getMin().getZ() > aabb2.getMax().getZ());
}

// ============================================================================
//  AABB vs Plane
// ============================================================================
bool BroadCollision::isColliding(const AABB& aabb, const Plane& plane)
{
    decimal  aabbRadius = aabb.getHalfExtents().getNorm();
    Vector3D n          = plane.getNormal().getNormalised();
    decimal  dist       = (aabb.getPosition() - plane.getPosition()).dotProduct(n);

    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist),
                                                 aabbRadius + (0.5 * plane.getSize()).getMax());
}

// ============================================================================
//  Plane vs Plane
// ============================================================================
bool BroadCollision::isColliding(const Plane& plane1, const Plane& plane2)
{
    decimal dist = (plane1.getPosition() - plane2.getPosition()).getNorm();
    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist),
                                                 (0.5 * plane1.getSize()).getMax()) +
           (0.5 * plane2.getSize()).getMax();
}
