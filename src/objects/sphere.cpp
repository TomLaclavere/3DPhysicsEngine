/**
 * @file sphere.cpp
 * @brief Implementation of collision detection methods for Sphere.
 *
 * This file provides the concrete implementations of collision checks involving Spheres:
 * - Sphere vs. Sphere
 * - Sphere vs. Sphere
 * - Sphere vs. Plane
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see sphere.hpp
 */
#include "objects/sphere.hpp"

#include "collision/broad_collision.hpp"
#include "collision/narrow_collision.hpp"

// ============================================================================
//  Getters
// ============================================================================
ObjectType Sphere::getType() const { return ObjectType::Sphere; }
Vector3D   Sphere::getCenter() const { return getPosition(); }
decimal    Sphere::getDiameter() const { return getSize().getX(); }
decimal    Sphere::getRadius() const { return getDiameter() * 0.5_d; }

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks broad collision between two Sphere.s
 */
bool Sphere::checkSphereCollision(const Sphere& Sphere) { return BroadCollision::isColliding(*this, Sphere); }

/**
 * @brief Checks broad collision between an Sphere and an AABB.
 */
bool Sphere::checkSphereAABBCollision(const AABB& aabb) { return BroadCollision::isColliding(*this, aabb); }

/**
 * @brief Checks broad collision between an Sphere and a finite plane.
 */
bool Sphere::checkSpherePlaneCollision(const Plane& plane)
{
    return BroadCollision::isColliding(*this, plane);
}

/**
 * @brief Polymorphic broad collision detection against another Object.
 */
bool Sphere::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::Sphere:
        return Sphere::checkSphereCollision(static_cast<const Sphere&>(other));
    case ObjectType::AABB:
        return Sphere::checkSphereAABBCollision(static_cast<const AABB&>(other));
    case ObjectType::Plane:
        return Sphere::checkSpherePlaneCollision(static_cast<const Plane&>(other));
    default:
        return false;
    }
}

/**
 * @brief Checks narrow collision between two Sphere.
 */
bool Sphere::computeSphereCollision(const Sphere& Sphere, Contact& contact)
{
    return NarrowCollision::computeContact(*this, Sphere, contact);
}

/**
 * @brief Checks narrow collision between an Sphere and a sphere.
 */
bool Sphere::computeSphereAABBCollision(const AABB& aabb, Contact& contact)
{
    return NarrowCollision::computeContact(*this, aabb, contact);
}

/**
 * @brief Checks narrow collision between an Sphere and a finite plane.
 */
bool Sphere::computeSpherePlaneCollision(const Plane& plane, Contact& contact)
{
    return NarrowCollision::computeContact(*this, plane, contact);
}

/**
 * @brief Polymorphic narrow collision detection against another Object.
 */
bool Sphere::computeCollision(const Object& other, Contact& contact)
{
    switch (other.getType())
    {
    case ObjectType::Sphere:
        return Sphere::computeSphereCollision(static_cast<const Sphere&>(other), contact);
    case ObjectType::AABB:
        return Sphere::computeSphereAABBCollision(static_cast<const AABB&>(other), contact);
    case ObjectType::Plane:
        return Sphere::computeSpherePlaneCollision(static_cast<const Plane&>(other), contact);
    default:
        return false;
    }
}
