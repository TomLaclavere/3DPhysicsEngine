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

#include "objects/AABB.hpp"
#include "precision.hpp"

#include <algorithm>

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
bool Sphere::sphereCollision(const Sphere& a, const Sphere& b)
{
    Vector3D centerDiff      = b.getCenter() - a.getCenter();
    decimal  squaredDistance = centerDiff.getNormSquare();
    decimal  squaredSumRadii = (a.getRadius() + b.getRadius()) * (a.getRadius() + b.getRadius());
    return squaredDistance <= squaredSumRadii;
}

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
bool Sphere::aabbSphereCollision(const Sphere& sphere, const AABB& aabb)
{
    const Vector3D c    = sphere.getCenter();
    const Vector3D amin = aabb.getMin();
    const Vector3D amax = aabb.getMax();

    // Clamp each coordinate of the sphere center to the AABB bounds
    Vector3D closest;
    closest[0] = std::clamp(c[0], amin[0], amax[0]);
    closest[1] = std::clamp(c[1], amin[1], amax[1]);
    closest[2] = std::clamp(c[2], amin[2], amax[2]);

    // Compute squared distance from sphere center to closest point on AABB
    decimal dist = (closest - c).getNormSquare();
    decimal r2   = sphere.getRadius() * sphere.getRadius();

    // Add small epsilon (PRECISION_MACHINE) to account for floating-point errors
    return dist <= r2 + PRECISION_MACHINE;
}

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
        return sphereCollision(*this, static_cast<const Sphere&>(other));
    case ObjectType::AABB:
        return aabbSphereCollision(*this, static_cast<const AABB&>(other));
    default:
        return false;
    }
}
