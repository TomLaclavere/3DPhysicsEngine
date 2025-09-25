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
bool AABB::aabb_collision(const AABB& a, const AABB& b)
{
    // Check for overlap along each axis
    return (a.get_min()[0] <= b.get_max()[0] && a.get_max()[0] >= b.get_min()[0]) &&
           (a.get_min()[1] <= b.get_max()[1] && a.get_max()[1] >= b.get_min()[1]) &&
           (a.get_min()[2] <= b.get_max()[2] && a.get_max()[2] >= b.get_min()[2]);
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
bool AABB::AABB_sphere_collision(const AABB& aabb, const Sphere& sphere)
{
    // Use the static function from Sphere for consistency
    return Sphere::AABB_sphere_collision(sphere, aabb);
}

/**
 * @brief Checks for intersection between this AABB and another Object.
 *
 * Dispatches the collision test depending on the runtime type of the other
 * object:
 * - AABB: Uses aabb_collision().
 * - Sphere: Uses AABB_sphere_collision().
 * - Default: Returns false (no collision test implemented).
 *
 * @param other The Object to test against.
 * @return true if a collision is detected, false otherwise.
 */
bool AABB::check_collision(const Object& other)
{
    switch (other.get_type())
    {
    case ObjectType::AABB:
        return aabb_collision(*this, static_cast<const AABB&>(other));
    case ObjectType::Sphere:
        return AABB_sphere_collision(*this, static_cast<const Sphere&>(other));
    default:
        return false;
    }
}
