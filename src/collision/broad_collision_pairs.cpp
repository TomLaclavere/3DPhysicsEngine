/**
 * @file broad_collision.cpp
 * @brief Implementation of broad-phase collision detection functions.
 */
#include "collision/broad_collision_pairs.hpp"

#include "mathematics/common.hpp"

//  Sphere vs Sphere

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
bool BoundingSphereTest::isColliding(const SphereShape& sphere1, const SphereShape& sphere2)
{
    Vector3D d = sphere2.center - sphere1.center;
    decimal  r = sphere1.radius + sphere2.radius;
    return commonMaths::approxSmallerOrEqualThan(d.getNormSquare(), r * r);
}

//  Sphere vs AABB

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
bool BoundingSphereTest::isColliding(const SphereShape& sphere, const AABBShape& aabb)
{
    decimal aabbRadius = aabb.halfExtents.getNorm(); // half-diagonal
    decimal dist       = (sphere.center - aabb.center).getNorm();
    return commonMaths::approxSmallerOrEqualThan(dist, aabbRadius + sphere.radius);
}

//  Sphere vs Plane

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
bool BoundingSphereTest::isColliding(const SphereShape& sphere, const PlaneShape& plane)
{
    Vector3D n    = plane.normal;
    decimal  dist = (sphere.center - plane.center).dotProduct(n);
    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist),
                                                 sphere.radius + (plane.boundingHalfSize));
}

//  AABB vs AABB

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
bool BoundingSphereTest::isColliding(const AABBShape& aabb1, const AABBShape& aabb2)
{
    return !(aabb1.getMax().getX() < aabb2.getMin().getX() || aabb1.getMin().getX() > aabb2.getMax().getX() ||
             aabb1.getMax().getY() < aabb2.getMin().getY() || aabb1.getMin().getY() > aabb2.getMax().getY() ||
             aabb1.getMax().getZ() < aabb2.getMin().getZ() || aabb1.getMin().getZ() > aabb2.getMax().getZ());
}

//  AABB vs Plane

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
bool BoundingSphereTest::isColliding(const AABBShape& aabb, const PlaneShape& plane)
{
    Vector3D n    = plane.normal;
    Vector3D half = aabb.halfExtents;

    // Project AABB half-extents along the plane normal
    decimal aabbProj = half[0] * commonMaths::absVal(n[0]) + half[1] * commonMaths::absVal(n[1]) +
                       half[2] * commonMaths::absVal(n[2]);

    // Distance from AABB center to plane along the normal
    decimal dist = (aabb.center - plane.center).dotProduct(n);

    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist), aabbProj);
}

//  Plane vs Plane

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
bool BoundingSphereTest::isColliding(const PlaneShape& plane1, const PlaneShape& plane2)
{
    decimal dist = (plane1.center - plane2.center).getNorm();
    return commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(dist),
                                                 plane1.boundingHalfSize + plane2.boundingHalfSize);
}
