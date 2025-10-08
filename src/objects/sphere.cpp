/**
 * @file sphere.cpp
 * @brief Implementation of collision detection methods for Sphere.
 *
 * This file provides the concrete implementations of collision checks involving Spheres:
 * - Sphere vs. Sphere
 * - Sphere vs. AABB
 * - Sphere vs. Plane
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see sphere.hpp
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
 * @param sphere Sphere to test against.
 * @return true if the spheres intersect, false otherwise.
 */
bool Sphere::sphereCollision(const Sphere& sphere) { return Collision::collide(*this, sphere); }

/**
 * @brief Checks collision between a sphere and an axis-aligned bounding box (AABB).
 *
 * Collision occurs if the closest point on the AABB to the sphere center is smalled than the sphere radius.
 *
 * @param aabb AABB to test against.
 * @return true if the sphere and AABB intersect, false otherwise.
 */
bool Sphere::sphereAABBCollision(const AABB& aabb) { return Collision::collide(*this, aabb); }

/**
 * @brief Checks collision between a sphere and a finite plane.
 *
 * Collision occurs if the sphere's center-to-plane distance is smaller than the sphere radius, and if the
 * projection of the sphere center onto the plane lies within its rectangle bounds.
 *
 * @param plane Plane to test against.
 * @return true if the sphere and plane intersect, false otherwise.
 */
bool Sphere::spherePlaneCollision(const Plane& plane) { return Collision::collide(*this, plane); }

/**
 * @brief Polymorphic collision detection against another Object.
 *
 * Dispatches to the appropriate collision method depending on the type of
 * the other object.
 * Only collision between Sphere, AABB, and Plane are implemented.
 *
 * @param other Object to test against.
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
    case ObjectType::Plane:
        return Sphere::spherePlaneCollision(static_cast<const Plane&>(other));
    default:
        return false;
    }
}
