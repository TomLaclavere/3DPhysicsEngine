#include "objects/collision.hpp"

#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <algorithm>

// ============================================================================
//  Sphere
// ============================================================================
bool sphereCollision(const Sphere& sphere1, const Sphere& sphere2)
{
    Vector3D centerDiff      = sphere2.getCenter() - sphere1.getCenter();
    decimal  squaredDistance = centerDiff.getNormSquare();
    decimal  squaredSumRadii =
        (sphere2.getRadius() + sphere1.getRadius()) * (sphere2.getRadius() + sphere1.getRadius());

    return squaredDistance <= squaredSumRadii + PRECISION_MACHINE;
}

bool sphereAABBCollision(const Sphere& sphere, const AABB& aabb)
{
    const Vector3D center  = sphere.getCenter();
    const Vector3D aabbMin = aabb.getMin();
    const Vector3D aabbMax = aabb.getMax();

    // Clamp each coordinate of the sphere center to the AABB bounds
    Vector3D closest;
    closest[0] = std::clamp(center[0], aabbMin[0], aabbMax[0]);
    closest[1] = std::clamp(center[1], aabbMin[1], aabbMax[1]);
    closest[2] = std::clamp(center[2], aabbMin[2], aabbMax[2]);

    // Compute squared distance from sphere center to closest point on AABB
    decimal dist = (closest - center).getNormSquare();
    decimal r2   = sphere.getRadius() * sphere.getRadius();

    return dist <= r2 + PRECISION_MACHINE;
}

bool spherePlaneCollision(const Sphere& sphere, const Plane& plane)
{
    // 1- Signed distance from the sphere center to plane
    decimal dist = (sphere.getCenter() - plane.getPosition()).dotProduct(plane.getNormal());

    if (std::abs(dist) > sphere.getRadius())
        return false; // sphere too far away from plane

    // 2- Project sphere center onto the plane
    Vector3D proj = sphere.getCenter() - dist * plane.getNormal();

    // 3- Check if projection lies within plane rectangle
    Vector3D local = proj - plane.getPosition();
    decimal  s     = local.dotProduct(plane.getU());
    decimal  t     = local.dotProduct(plane.getV());

    return (std::abs(s) <= plane.getHalfWidth()) && (std::abs(t) <= plane.getHalfHeight());
}

// ============================================================================
//  AABB
// ============================================================================
bool aabbCollision(const AABB& aabb1, const AABB& aabb2)
{
    // Check for overlap along each axis
    return (aabb1.getMin() <= aabb2.getMax() && aabb1.getMax() >= aabb2.getMin());
}

bool aabbSphereCollision(const AABB& aabb, const Sphere& sphere) { return sphereAABBCollision(sphere, aabb); }

bool aabbPlaneCollision(const AABB& aabb, const Plane& plane)
{
    // 1- Signed distance from AABB center to plane
    decimal dist = (aabb.getPosition() - plane.getPosition()).dotProduct(plane.getNormal());

    // 2- Project "radius" of the AABB along the plane normal
    decimal r = std::abs(aabb.getHalfExtents()[0] * plane.getNormal()[0]) +
                std::abs(aabb.getHalfExtents()[1] * plane.getNormal()[1]) +
                std::abs(aabb.getHalfExtents()[2] * plane.getNormal()[2]);

    if (std::abs(dist) > r)
        return false; // no overlap with infinite plane

    // 3- Check corners against plane bounds
    for (int x = -1; x <= 1; x += 2)
        for (int y = -1; y <= 1; y += 2)
            for (int z = -1; z <= 1; z += 2)
            {
                Vector3D corner =
                    aabb.getPosition() + Vector3D(x * aabb.getHalfExtents()[0], y * aabb.getHalfExtents()[1],
                                                  z * aabb.getHalfExtents()[2]);
                decimal  cornerDist = (corner - plane.getPosition()).dotProduct(plane.getNormal());
                Vector3D proj       = corner - cornerDist * plane.getNormal();

                Vector3D local = proj - plane.getPosition();
                decimal  s     = local.dotProduct(plane.getU());
                decimal  t     = local.dotProduct(plane.getV());

                if (std::abs(s) <= plane.getHalfWidth() && std::abs(t) <= plane.getHalfHeight())
                    return true; // at least one corner intersects the plane patch
            }

    return false;
}

// ============================================================================
//  Plane
// ============================================================================
bool planeCollision(const Plane& plane1, const Plane& plane2)
{
    // 1- Check if parallel
    const decimal dot = std::abs(plane1.getNormal().dotProduct(plane2.getNormal()));
    if (dot > 1 - PRECISION_MACHINE)
        return false; // Parallel planes never intersect

    // 2- Compute intersection line (assuming infinite planes)
    Vector3D dir = plane1.getNormal().crossProduct(plane2.getNormal()).getNormalised();

    // Solve for a point on the line:
    // n1.x = n1.P1, n2.x = n2.P2
    decimal d1 = plane1.getNormal().dotProduct(plane1.getPosition());
    decimal d2 = plane2.getNormal().dotProduct(plane2.getPosition());

    // Find intersection point
    Vector3D intersectionPoint = ((d1 * plane2.getNormal() - (d2 * plane1.getNormal())).crossProduct(dir)) /
                                 (plane1.getNormal().crossProduct(plane2.getNormal())).getNormSquare();

    // 3- Project that point into each plane’s local frame to see if it lies within their rectangles
    auto inBounds = [](const Plane& P, const Vector3D& X)
    {
        Vector3D local = X - P.getPosition();
        decimal  s     = local.dotProduct(P.getU());
        decimal  t     = local.dotProduct(P.getV());
        return (std::abs(s) <= P.getHalfWidth()) && (std::abs(t) <= P.getHalfHeight());
    };

    return inBounds(plane1, intersectionPoint) && inBounds(plane2, intersectionPoint);
};

bool planeSphereCollision(const Plane& plane, const Sphere& sphere)
{
    return spherePlaneCollision(sphere, plane);
}

bool planeAABBCollision(const Plane& plane, const AABB& aabb) { return aabbPlaneCollision(aabb, plane); }
