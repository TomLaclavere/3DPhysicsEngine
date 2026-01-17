/**
 * @file plane.cpp
 * @brief Implementation of utilities and collision detection methods for Plane.
 *
 * This file provides the concrete implementations of collision checks involving Planes:
 * - Plane vs. Plane
 * - Plane vs. Sphere
 * - Plane vs. Plane
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see plane.hpp
 */
#include "objects/plane.hpp"

#include "collision/broad_collision.hpp"
#include "collision/narrow_collision.hpp"
#include "mathematics/common.hpp"

// ============================================================================
//  Getters
// ============================================================================
ObjectType      Plane::getType() const { return ObjectType::Plane; }
const Vector3D& Plane::getNormal() const { return normal; }
const Vector3D& Plane::getU() const { return u; }
const Vector3D& Plane::getV() const { return v; }
decimal         Plane::getHalfWidth() const { return halfWidth; }
decimal         Plane::getHalfHeight() const { return halfHeight; }

// ============================================================================
//  Utilities
// ============================================================================
void Plane::updateLocalAxes()
{
    Vector3D n = normal; // already normalized externally

    // 1. Pick a vector not parallel to n
    Vector3D tangent;
    if (std::abs(n[0]) < 0.9_d)
        tangent = Vector3D(1_d, 0_d, 0_d);
    else
        tangent = Vector3D(0_d, 1_d, 0_d);

    // 2. First tangent = orthogonalized against n
    u = tangent - n * tangent.dotProduct(n);
    u = u.getNormalised();

    // 3. Second tangent = orthogonal to both
    v = n.crossProduct(u);
    v = v.getNormalised();
}
Vector3D Plane::projectPoint(const Vector3D& point) const
{
    decimal dist = (point - getPosition()).dotProduct(normal);
    return point - dist * normal;
}

bool Plane::containsPoint(const Vector3D& point) const
{
    Vector3D local = point - getPosition();
    decimal  s     = local.dotProduct(u);
    decimal  t     = local.dotProduct(v);
    return (commonMaths::approxSmallerOrEqualThan(std::abs(s), halfWidth) &&
            commonMaths::approxSmallerOrEqualThan(std::abs(t), halfHeight));
}

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks broad collision between two Planes.
 */
bool Plane::checkPlaneCollision(const Plane& Plane) { return BroadCollision::isColliding(*this, Plane); }

/**
 * @brief Checks broad collision between an Plane and a sphere.
 */
bool Plane::checkPlaneSphereCollision(const Sphere& sphere)
{
    return BroadCollision::isColliding(*this, sphere);
}

/**
 * @brief Checks broad collision between an Plane and an AABB.
 */
bool Plane::checkPlaneAABBCollision(const AABB& aabb) { return BroadCollision::isColliding(*this, aabb); }

/**
 * @brief Polymorphic broad collision detection against another Object.
 */
bool Plane::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::Plane:
        return Plane::checkPlaneCollision(static_cast<const Plane&>(other));
    case ObjectType::Sphere:
        return Plane::checkPlaneSphereCollision(static_cast<const Sphere&>(other));
    case ObjectType::AABB:
        return Plane::checkPlaneAABBCollision(static_cast<const AABB&>(other));
    default:
        return false;
    }
}

/**
 * @brief Checks narrow collision between two Planes.
 */
bool Plane::computePlaneCollision(const Plane& Plane, Contact& contact)
{
    return NarrowCollision::computeContact(*this, Plane, contact);
}

/**
 * @brief Checks narrow collision between an Plane and a sphere.
 */
bool Plane::computePlaneSphereCollision(const Sphere& sphere, Contact& contact)
{
    return NarrowCollision::computeContact(*this, sphere, contact);
}

/**
 * @brief Checks narrow collision between a Plane and an AABB.
 */
bool Plane::computePlaneAABBCollision(const AABB& aabb, Contact& contact)
{
    return NarrowCollision::computeContact(*this, aabb, contact);
}

/**
 * @brief Polymorphic narrow collision detection against another Object.
 */
bool Plane::computeCollision(const Object& other, Contact& contact)
{
    switch (other.getType())
    {
    case ObjectType::Plane:
        return Plane::computePlaneCollision(static_cast<const Plane&>(other), contact);
    case ObjectType::Sphere:
        return Plane::computePlaneSphereCollision(static_cast<const Sphere&>(other), contact);
    case ObjectType::AABB:
        return Plane::computePlaneAABBCollision(static_cast<const AABB&>(other), contact);
    default:
        return false;
    }
}
