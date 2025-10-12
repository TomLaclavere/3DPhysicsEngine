/**
 * @file collision.cpp
 * @brief Definition of collision conditions between Object types used in physics simulation.
 *
 * Implements collision detection between Sphere, AABB, and Plane objects.
 *
 * @see collision.hpp
 */

#include "objects/collision.hpp"

#include "mathematics/common.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <algorithm>
#include <cmath>

namespace Collision {

// ============================================================================
//  Sphere vs Sphere
// ============================================================================
/**
 * @brief Checks collision between two spheres.
 *
 * Collision occurs if the distance between centers is less than or equal
 * to the sum of their radii.
 *
 * @param s1 Sphere instance.
 * @param s2 Sphere instance.
 * @return true if the spheres intersect, false otherwise.
 */
template <>
bool collide<Sphere, Sphere>(const Sphere& s1, const Sphere& s2)
{
    const Vector3D diff  = s2.getCenter() - s1.getCenter();
    const decimal  dist2 = diff.getNormSquare();
    const decimal  rSum  = s1.getRadius() + s2.getRadius();

    return commonMaths::approxSmallerOrEqualThan(dist2, rSum * rSum);
}

// ============================================================================
//  Sphere vs AABB
// ============================================================================
/**
 * @brief Checks collision between a sphere and an axis-aligned bounding box (AABB).
 *
 * Collision occurs if the closest point on the AABB to the sphere center is smalled than the sphere radius.
 *
 * @param sphere Sphere instance.
 * @param aabb AABB instance.
 * @return true if the sphere and AABB intersect, false otherwise.
 */
template <>
bool collide<Sphere, AABB>(const Sphere& sphere, const AABB& aabb)
{
    const Vector3D center = sphere.getCenter();
    const Vector3D min    = aabb.getMin();
    const Vector3D max    = aabb.getMax();

    // Find the closest point on AABB to sphere center
    Vector3D closestPoint;
    closestPoint[0] = std::max(min[0], std::min(center[0], max[0]));
    closestPoint[1] = std::max(min[1], std::min(center[1], max[1]));
    closestPoint[2] = std::max(min[2], std::min(center[2], max[2]));

    // Check if closest point is inside sphere
    const Vector3D diff   = center - closestPoint;
    const decimal  dist2  = diff.getNormSquare();
    const decimal  radius = sphere.getRadius();

    return commonMaths::approxSmallerOrEqualThan(dist2, radius * radius);
}

// ============================================================================
//  Sphere vs Plane
// ============================================================================
/**
 * @brief Checks collision between a sphere and a finite plane.
 *
 * Collision occurs if the sphere's center-to-plane distance is smaller than the sphere radius, and if the
 * projection of the sphere center onto the plane lies within its rectangle bounds.
 *
 * @param sphere Sphere instance.
 * @param plane Plane instance.
 * @return true if the sphere and plane intersect, false otherwise.
 */
template <>
bool collide<Sphere, Plane>(const Sphere& sphere, const Plane& plane)
{
    const Vector3D& sphereCenter = sphere.getCenter();
    const decimal   sphereRadius = sphere.getRadius();

    // 1. Check distance using plane equation
    const Vector3D planeToSphere = sphereCenter - plane.getPosition();
    const decimal  signedDist    = planeToSphere.dotProduct(plane.getNormal());

    // Early exit: sphere completely behind or too far in front
    if (signedDist < -sphereRadius || signedDist > sphereRadius)
    {
        return false;
    }

    // 2. Project onto plane and check bounds with radius padding
    const Vector3D proj  = sphereCenter - signedDist * plane.getNormal();
    const Vector3D local = proj - plane.getPosition();

    const decimal s = local.dotProduct(plane.getU());
    const decimal t = local.dotProduct(plane.getV());

    // Check bounds with radius consideration
    const decimal effectiveHalfWidth  = plane.getHalfWidth() + sphereRadius;
    const decimal effectiveHalfHeight = plane.getHalfHeight() + sphereRadius;

    if (std::abs(s) > effectiveHalfWidth || std::abs(t) > effectiveHalfHeight)
    {
        return false;
    }

    // 3. Exact distance to clamped point
    const decimal clampedS = std::clamp(s, -plane.getHalfWidth(), plane.getHalfWidth());
    const decimal clampedT = std::clamp(t, -plane.getHalfHeight(), plane.getHalfHeight());

    const Vector3D closestPoint = plane.getPosition() + clampedS * plane.getU() + clampedT * plane.getV();
    const Vector3D delta        = closestPoint - sphereCenter;
    const decimal  dist2        = delta.dotProduct(delta);

    // Collision if within radius
    return commonMaths::approxSmallerOrEqualThan(dist2, sphere.getRadius() * sphere.getRadius());
}

// ============================================================================
//  AABB vs AABB
// ============================================================================
/**
 * @brief Checks collision between two aabb.
 *
 * Collision occurs if they overlap on each axis simultaneously.
 *
 * @param a1 AABB instance.
 * @param a2 AABB instance.
 * @return true if the AABBs intersect, false otherwise.
 */
template <>
bool collide<AABB, AABB>(const AABB& a1, const AABB& a2)
{
    const Vector3D a1Min = a1.getMin();
    const Vector3D a1Max = a1.getMax();
    const Vector3D a2Min = a2.getMin();
    const Vector3D a2Max = a2.getMax();

    // Check for separation on any axis
    if (a1Max[0] < a2Min[0] || a1Min[0] > a2Max[0])
        return false;
    if (a1Max[1] < a2Min[1] || a1Min[1] > a2Max[1])
        return false;
    if (a1Max[2] < a2Min[2] || a1Min[2] > a2Max[2])
        return false;

    return true;
}

// ============================================================================
//  AABB vs Plane
// ============================================================================
/**
 * @brief Checks collision between an aabb and a finite plane.
 *
 * Collision occurs if the signed distance from the AABB to the plane is less than or equal to the projection
 * radius of the AABB onto the plane normal.
 *
 * @param aabb AABB instance.
 * @param plane Plane instance.
 * @return true if the AABB and the Plane intersect, false otherwise.
 */
template <>
bool collide<AABB, Plane>(const AABB& aabb, const Plane& plane)
{
    // Calculate AABB center and half-extents
    const Vector3D center      = aabb.getPosition();
    const Vector3D halfExtents = aabb.getHalfExtents();

    // Project AABB onto plane normal to get the effective radius
    const decimal r = halfExtents[0] * std::abs(plane.getNormal()[0]) +
                      halfExtents[1] * std::abs(plane.getNormal()[1]) +
                      halfExtents[2] * std::abs(plane.getNormal()[2]);

    // Calculate distance from AABB center to plane
    const Vector3D centerToPlane = center - plane.getPosition();
    const decimal  distance      = centerToPlane.dotProduct(plane.getNormal());

    // If AABB is completely on one side of the plane, no collision
    if (std::abs(distance) > r)
    {
        return false;
    }

    // At this point, we know the AABB intersects the infinite plane
    // Now check if any corner of AABB projects within the finite plane bounds

    const Vector3D planeU     = plane.getU();
    const Vector3D planeV     = plane.getV();
    const decimal  halfWidth  = plane.getHalfWidth();
    const decimal  halfHeight = plane.getHalfHeight();

    // Check all 8 corners of the AABB
    for (int i = -1; i <= 1; i += 2)
    {
        for (int j = -1; j <= 1; j += 2)
        {
            for (int k = -1; k <= 1; k += 2)
            {
                const Vector3D corner =
                    center + Vector3D(i * halfExtents[0], j * halfExtents[1], k * halfExtents[2]);

                // Project corner onto plane
                const Vector3D cornerToPlane = corner - plane.getPosition();
                const decimal  cornerDist    = cornerToPlane.dotProduct(plane.getNormal());
                const Vector3D projection    = corner - cornerDist * plane.getNormal();

                // Check if projection is within plane bounds
                const Vector3D local = projection - plane.getPosition();
                const decimal  s     = local.dotProduct(planeU);
                const decimal  t     = local.dotProduct(planeV);

                if (std::abs(s) <= halfWidth && std::abs(t) <= halfHeight)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

// ============================================================================
//  Plane vs Plane
// ============================================================================
/**
 * @brief Checks collision between two finite planes.
 *
 * Collision occurs if the associated infinite planes intersects (they are not parallel), and if the
 * intersection line passes through both rectangles' bounds.
 *
 * @param p1 Plane instance.
 * @param p2 Plane instance.
 * @return true if the planes intersect, false otherwise.
 */
template <>
bool collide<Plane, Plane>(const Plane& p1, const Plane& p2)
{
    const Vector3D n1 = p1.getNormal();
    const Vector3D n2 = p2.getNormal();

    // 1 - Check if planes are parallel
    const decimal dot = std::abs(n1.dotProduct(n2));
    if (commonMaths::approxEqual(dot, 1.0))
    {
        // Planes are parallel, check if they're coincident
        const decimal d1 = n1.dotProduct(p1.getPosition());
        const decimal d2 = n2.dotProduct(p2.getPosition());
        return commonMaths::approxEqual(d1, d2);
    }

    // 2 - Compute intersection line
    Vector3D dir = p1.getNormal().crossProduct(p2.getNormal()).getNormalised();

    // Solve for a point on the line:
    // n1.x = n1.P1, n2.x = n2.P2
    decimal d1 = p1.getNormal().dotProduct(p1.getPosition());
    decimal d2 = p2.getNormal().dotProduct(p2.getPosition());

    // Find intersection point
    Vector3D intersectionPoint = ((d1 * p2.getNormal() - (d2 * p1.getNormal())).crossProduct(dir)) /
                                 (p1.getNormal().crossProduct(p2.getNormal())).getNormSquare();

    // 3- Project that point into each plane’s local frame to see if it lies within their rectangles
    auto inBounds = [](const Plane& P, const Vector3D& X)
    {
        Vector3D local = X - P.getPosition();
        decimal  s     = local.dotProduct(P.getU());
        decimal  t     = local.dotProduct(P.getV());
        return (std::abs(s) <= P.getHalfWidth()) && (std::abs(t) <= P.getHalfHeight());
    };

    return inBounds(p1, intersectionPoint) && inBounds(p2, intersectionPoint);
}

} // namespace Collision
