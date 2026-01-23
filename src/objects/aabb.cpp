/**
 * @file aabb.cpp
 * @brief Implementation of collision detection methods for Axis-Aligned Bounding Boxes (AABB).
 *
 * This file provides the concrete implementations of collision checks involving AABBs:
 * - AABB vs. AABB
 * - AABB vs. Sphere
 * - AABB vs. Plane
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see aabb.hpp
 */
#include "objects/aabb.hpp"

#include "collision/broad_collision.hpp"
#include "collision/narrow_collision.hpp"

// ============================================================================
//  Constructors / Destructors
// ============================================================================
AABB::AABB(const Vector3D& position, const Vector3D& size)
    : Object(position, size)
{}
AABB::AABB(const Vector3D& position, const Vector3D& size, decimal mass)
    : Object(position, size, mass)
{
    checkFixed();
}
AABB::AABB(const Vector3D& position, const Vector3D& size, const Vector3D& velocity, decimal mass)
    : Object(position, size, velocity, mass)
{
    checkFixed();
}
AABB::AABB(const Vector3D& position, const Vector3D& rotation, const Vector3D& size, const Vector3D& velocity,
           const Vector3D& acceleration, const Vector3D& force, const Vector3D& torque, decimal mass)
    : Object(position, rotation, size, velocity, acceleration, force, torque, mass)
{
    checkFixed();
}

// ============================================================================
//  Getters
// ============================================================================
ObjectType AABB::getType() const { return ObjectType::AABB; }
Vector3D   AABB::getHalfExtents() const { return getSize() * 0.5_d; }
Vector3D   AABB::getMin() const { return getPosition() - getHalfExtents(); }
Vector3D   AABB::getMax() const { return getPosition() + getHalfExtents(); }

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks broad collision between two aabb.
 */
bool AABB::checkAABBCollision(const AABB& aabb) { return BroadCollision::isColliding(*this, aabb); }

/**
 * @brief Checks broad collision between an aabb and a sphere.
 */
bool AABB::checkAABBSphereCollision(const Sphere& sphere)
{
    return BroadCollision::isColliding(*this, sphere);
}

/**
 * @brief Checks broad collision between an aabb and a finite plane.
 */
bool AABB::checkAABBPlaneCollision(const Plane& plane) { return BroadCollision::isColliding(*this, plane); }

/**
 * @brief Polymorphic broad collision detection against another Object.
 */
bool AABB::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::AABB:
        return AABB::checkAABBCollision(static_cast<const AABB&>(other));
    case ObjectType::Sphere:
        return AABB::checkAABBSphereCollision(static_cast<const Sphere&>(other));
    case ObjectType::Plane:
        return AABB::checkAABBPlaneCollision(static_cast<const Plane&>(other));
    default:
        return false;
    }
}

/**
 * @brief Checks narrow collision between two aabb.
 */
bool AABB::computeAABBCollision(const AABB& aabb, Contact& contact)
{
    return NarrowCollision::computeContact(*this, aabb, contact);
}

/**
 * @brief Checks narrow collision between an aabb and a sphere.
 */
bool AABB::computeAABBSphereCollision(const Sphere& sphere, Contact& contact)
{
    return NarrowCollision::computeContact(*this, sphere, contact);
}

/**
 * @brief Checks narrow collision between an aabb and a finite plane.
 */
bool AABB::computeAABBPlaneCollision(const Plane& plane, Contact& contact)
{
    return NarrowCollision::computeContact(*this, plane, contact);
}

/**
 * @brief Polymorphic narrow collision detection against another Object.
 */
bool AABB::computeCollision(const Object& other, Contact& contact)
{
    switch (other.getType())
    {
    case ObjectType::AABB:
        return AABB::computeAABBCollision(static_cast<const AABB&>(other), contact);
    case ObjectType::Sphere:
        return AABB::computeAABBSphereCollision(static_cast<const Sphere&>(other), contact);
    case ObjectType::Plane:
        return AABB::computeAABBPlaneCollision(static_cast<const Plane&>(other), contact);
    default:
        return false;
    }
}
