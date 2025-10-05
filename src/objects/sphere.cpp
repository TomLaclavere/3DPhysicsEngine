/**
 * @file sphere.cpp
 * @brief Implementation of collision detection methods for Sphere.
 *
 * This file provides the concrete implementations of collision checks involving Spheres:
 * - Sphere vs. Sphere
 * - Sphere vs. AABB
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see AABB.hpp
 */
#include "objects/sphere.hpp"

#include "objects/collision.hpp"

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks collision between two spheres.
 *
 * Collision occurs if the distance between centers is less than or equal
 * to the sum of their radii.
 *
 * @param a First sphere.
 * @param b Second sphere.
 * @return true if the spheres intersect, false otherwise.
 */
bool Sphere::sphereCollision(const Sphere& sphere) { return ::sphereCollision(*this, sphere); }

/**
 * @brief Checks collision between a sphere and an axis-aligned bounding box (AABB).
 *
 * The closest point on the AABB to the sphere center is computed, and collision
 * occurs if this point lies within the sphere radius.
 *
 * @param sphere The sphere to test.
 * @param aabb The AABB to test against.
 * @return true if the sphere and AABB intersect, false otherwise.
 */
bool Sphere::sphereAABBCollision(const AABB& aabb) { return ::sphereAABBCollision(*this, aabb); }

/**
 * @brief Polymorphic collision detection against another Object.
 *
 * Dispatches to the appropriate collision method depending on the type of
 * the other object (Sphere or AABB).
 *
 * @param other The object to test against.
 * @return true if the objects intersect, false otherwise.
 */
bool Sphere::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::Sphere:
        return Sphere::sphereCollision(static_cast<const Sphere&>(other));
    case ObjectType::AABB:
        return Sphere::sphereAABBCollision(static_cast<const AABB&>(other));
    default:
        return false;
    }
}
