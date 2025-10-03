/**
 * @file AABB.cpp
 * @brief Implementation of collision detection methods for Axis-Aligned Bounding Boxes (AABB).
 *
 * This file provides the concrete implementations of collision checks involving AABBs:
 * - AABB vs. AABB
 * - AABB vs. Sphere
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see AABB.hpp
 */
#include "objects/AABB.hpp"

#include "objects/sphere.hpp"

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks for intersection between two Axis-Aligned Bounding Boxes (AABB).
 *
 * This function verifies overlap along all three axes (x, y, z). Two AABBs are
 * considered colliding if they overlap on each axis simultaneously.
 *
 * @param a First AABB to test.
 * @param b Second AABB to test.
 * @return true if the AABBs overlap, false otherwise.
 */
bool AABB::aabbCollision(const AABB& a, const AABB& b)
{
    // Check for overlap along each axis
    return (a.getMin()[0] <= b.getMax()[0] && a.getMax()[0] >= b.getMin()[0]) &&
           (a.getMin()[1] <= b.getMax()[1] && a.getMax()[1] >= b.getMin()[1]) &&
           (a.getMin()[2] <= b.getMax()[2] && a.getMax()[2] >= b.getMin()[2]);
}

/**
 * @brief Checks for intersection between an Axis-Aligned Bounding Box (AABB)
 *        and a Sphere.
 *
 * Delegates the collision computation to the static helper function in the
 * Sphere class to ensure consistency across collision checks.
 *
 * @param aabb The AABB to test.
 * @param sphere The Sphere to test.
 * @return true if the AABB and the Sphere overlap, false otherwise.
 */
bool AABB::aabbSphereCollision(const AABB& aabb, const Sphere& sphere)
{
    // Use the static function from Sphere for consistency
    return Sphere::aabbSphereCollision(sphere, aabb);
}

/**
 * @brief Checks for intersection between this AABB and another Object.
 *
 * Dispatches the collision test depending on the runtime type of the other
 * object:
 * - AABB: Uses aabb_collision().
 * - Sphere: Uses aabbSphereCollision().
 * - Default: Returns false (no collision test implemented).
 *
 * @param other The Object to test against.
 * @return true if a collision is detected, false otherwise.
 */
bool AABB::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::AABB:
        return aabbCollision(*this, static_cast<const AABB&>(other));
    case ObjectType::Sphere:
        return aabbSphereCollision(*this, static_cast<const Sphere&>(other));
    default:
        return false;
    }
}
