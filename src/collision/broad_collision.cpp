#include "collision/broad_collision.hpp"

#include "mathematics/common.hpp"

#include <algorithm>

// ============================================================================
//  Sphere vs Sphere
// ============================================================================

/**
 * @brief Check broad collision between two Spheres.
 *
 * Collision occurs if the distance between centers is less than or equal
 * to the sum of their radii.
 *
 * @param sphere1
 * @param sphere2
 * @return true if the spheres intersect, false otherwise.
 */
bool BroadCollision::isColliding(const Sphere& sphere1, const Sphere& sphere2)
{
    Vector3D d = sphere2.getCenter() - sphere1.getCenter();
    decimal  r = sphere1.getRadius() + sphere2.getRadius();
    return commonMaths::approxSmallerOrEqualThan(d.getNormSquare(), r * r);
}

// ============================================================================
//  Sphere vs AABB
// ============================================================================

/**
 * @brief Check broad collisoin between Sphere and AABB.
 *
 * Collision occurs if the radius of the sphere including the AABB (defined as its half-diagonal) plus the
 * Sphere radius are greater than the distance between centers.
 *
 * @param sphere
 * @param aabb
 * @return true if the sphere and AABB intersect, false otherwise.
 */
bool BroadCollision::isColliding(const Sphere& sphere, const AABB& aabb)
{
    decimal aabbRadius = aabb.getHalfExtents().getNorm(); // half-diagonal
    decimal dist       = (sphere.getCenter() - aabb.getPosition()).getNorm();
    return commonMaths::approxSmallerOrEqualThan(dist, aabbRadius + sphere.getRadius());
}

// ============================================================================
//  Sphere vs Plane
// ============================================================================

/**
 * @brief Check broad collision between Sphere and Plane.
 *
 * Collision occurs if the absolute signed distance between the sphere center
 * and the plane is less than or equal to the sphere radius plus an
 * approximation of the plane extent.
 *
 * The plane is treated as a finite surface, approximated by a bounding
 * sphere whose radius is defined as half of its maximal size.
 *
 * @param sphere
 * @param plane
 * @return true if the sphere and plane potentially intersect, false otherwise.
 */
bool BroadCollision::isColliding(const Sphere& sphere, const Plane& plane)
{
    Vector3D n    = plane.getNormal().getNormalised();
    decimal  dist = (sphere.getCenter() - plane.getPosition()).dotProduct(n);
    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist),
                                                 sphere.getRadius() + (0.5 * plane.getSize()).getMax());
}

// ============================================================================
//  AABB vs AABB
// ============================================================================

/**
 * @brief Check broad collision between two AABBs.
 *
 * Collision occurs if the projections of the two axis-aligned bounding boxes
 * overlap on all three coordinate axes (X, Y, and Z).
 *
 * This test is equivalent to checking for a separating axis aligned with
 * the world coordinate system.
 *
 * @param aabb1
 * @param aabb2
 * @return true if the AABBs intersect, false otherwise.
 */
bool BroadCollision::isColliding(const AABB& aabb1, const AABB& aabb2)
{
    return !(aabb1.getMax().getX() < aabb2.getMin().getX() || aabb1.getMin().getX() > aabb2.getMax().getX() ||
             aabb1.getMax().getY() < aabb2.getMin().getY() || aabb1.getMin().getY() > aabb2.getMax().getY() ||
             aabb1.getMax().getZ() < aabb2.getMin().getZ() || aabb1.getMin().getZ() > aabb2.getMax().getZ());
}

// ============================================================================
//  AABB vs Plane
// ============================================================================

/**
 * @brief Broad-phase check: AABB vs Plane.
 *
 * Collision is reported if the AABB's center is within the AABB's half-diagonal
 * distance from the plane along the plane normal.
 * This is a fast conservative test: the plane is treated as infinite.
 *
 * @param aabb The AABB to test.
 * @param plane The Plane to test.
 * @return true if the AABB and plane may intersect, false otherwise.
 */
bool BroadCollision::isColliding(const AABB& aabb, const Plane& plane)
{
    Vector3D n    = plane.getNormal().getNormalised();
    Vector3D half = aabb.getHalfExtents();

    // Project AABB half-extents along the plane normal
    decimal aabbProj = half[0] * commonMaths::absVal(n[0]) + half[1] * commonMaths::absVal(n[1]) +
                       half[2] * commonMaths::absVal(n[2]);

    // Distance from AABB center to plane along the normal
    decimal dist = (aabb.getPosition() - plane.getPosition()).dotProduct(n);

    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist), aabbProj);
}

// ============================================================================
//  Plane vs Plane
// ============================================================================

/**
 * @brief Check broad collision between two Planes.
 *
 * Collision occurs if the distance between the plane reference points
 * is less than or equal to the sum of their approximate extents.
 *
 * Each plane is approximated by a bounding sphere whose radius is defined
 * as half of its maximal size.
 *
 * @param plane1
 * @param plane2
 * @return true if the planes potentially intersect, false otherwise.
 */
bool BroadCollision::isColliding(const Plane& plane1, const Plane& plane2)
{
    decimal dist = (plane1.getPosition() - plane2.getPosition()).getNorm();
    return commonMaths::approxSmallerOrEqualThan(
        commonMaths::absVal(dist), (0.5 * plane1.getSize()).getMax() + (0.5 * plane2.getSize()).getMax());
}
