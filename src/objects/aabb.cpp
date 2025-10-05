/**
 * @file aabb.cpp
 * @brief Implementation of collision detection methods for Axis-Aligned Bounding Boxes (AABB).
 *
 * This file provides the concrete implementations of collision checks involving AABBs:
 * - AABB vs. AABB
 * - AABB vs. Sphere
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
 * @brief Checks for intersection between two Axis-Aligned Bounding Boxes (AABB).
 *
 * This function verifies overlap along all three axes (x, y, z). Two AABBs are
 * considered colliding if they overlap on each axis simultaneously.
 *
 * @param aabb AABB to test against.
 * @return true if the AABBs overlap, false otherwise.
 */
bool AABB::aabbCollision(const AABB& aabb) { return ::aabbCollision(*this, aabb); }

/**
 * @brief Checks for intersection between an Axis-Aligned Bounding Box (AABB)
 *        and a Sphere.
 *
 * Delegates the collision computation to the static helper function in the
 * Sphere class to ensure consistency across collision checks.
 *
 * @param sphere Sphere to test against.
 * @return true if the AABB and the Sphere overlap, false otherwise.
 */
bool AABB::aabbSphereCollision(const Sphere& sphere) { return ::aabbSphereCollision(*this, sphere); }

/**
 * @brief Checks for intersection between this AABB and another Object.
 *
 * Dispatches the collision test depending on the runtime type of the other
 * object:
 * - AABB: Uses aabbCollision().
 * - Sphere: Uses aabbSphereCollision().
 * - Default: Returns false (no collision test implemented).
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
    default:
        return false;
    }
}
