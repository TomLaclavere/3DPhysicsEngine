#include "objects/collision.hpp"

#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <algorithm>

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

bool spherePlaneCollision(const Sphere& sphere, const Plane& plane) { return true; }

bool aabbCollision(const AABB& aabb1, const AABB& aabb2)
{
    // Check for overlap along each axis
    return (aabb1.getMin() <= aabb2.getMax() && aabb1.getMax() >= aabb2.getMin());
}

bool aabbSphereCollision(const AABB& aabb, const Sphere& sphere) { return sphereAABBCollision(sphere, aabb); }

bool aabbPlaneCollision(const AABB& aabb, const Plane& plane) { return true; }

bool planeCollision(const Plane& plane1, const Plane& plane2) { return true; }

bool planeSphereCollision(const Plane& plane, const Sphere& sphere)
{
    return spherePlaneCollision(sphere, plane);
}

bool planeAABBCollision(const Plane& plane, const AABB& aabb) { return aabbPlaneCollision(aabb, plane); }
