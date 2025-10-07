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

#include "objects/collision.hpp"

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks collision between two aabb.
 *
 * Collision occurs if they overlap on each axis simultaneously.
 *
 * @param aabb AABB to test against.
 * @return true if the AABBs intersect, false otherwise.
 */
bool AABB::aabbCollision(const AABB& aabb) { return ::aabbCollision(*this, aabb); }

/**
 * @brief Checks collision between an aabb and a sphere.
 *
 * Collision occurs if the sphere's center-to-plane distance is smaller than the sphere radius, and if the
 * projection of the sphere center onto the plane lies within its rectangle bounds.
 *
 * @param sphere Sphere to test against.
 * @return true if the AABB and the Sphere intersect, false otherwise.
 */
bool AABB::aabbSphereCollision(const Sphere& sphere) { return ::aabbSphereCollision(*this, sphere); }

/**
 * @brief Checks collision between an aabb and a finite plane.
 *
 * Collision occurs if the signed distance from the AABB to the plane is less than or equal to the projection
 * radius of the AABB onto the plane normal.
 *
 * @param plane Plane to test against.
 * @return true if the AABB and the Plane intersect, false otherwise.
 */
bool AABB::aabbPlaneCollision(const Plane& plane) { return ::aabbPlaneCollision(*this, plane); }

/**
 * @brief Polymorphic collision detection against another Object.
 *
 * Dispatches to the appropriate collision method depending on the type of
 * the other object.
 * Only collision between Sphere, AABB, and Plane are implemented.
 *
 * @param other Object to test against.
 * @return true if a collision is detected, false otherwise.
 */
bool AABB::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::AABB:
        return AABB::aabbCollision(static_cast<const AABB&>(other));
    case ObjectType::Sphere:
        return AABB::aabbSphereCollision(static_cast<const Sphere&>(other));
    case ObjectType::Plane:
        return AABB::aabbPlaneCollision(static_cast<const Plane&>(other));
    default:
        return false;
    }
}
