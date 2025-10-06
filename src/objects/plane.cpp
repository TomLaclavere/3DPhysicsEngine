#include "objects/plane.hpp"

#include "objects/collision.hpp"

// ============================================================================
/// Utilities
// ============================================================================
void Plane::updateLocalAxes()
{
    // Choose an arbitrary vector not parallel to the normal
    Vector3D ref = (std::abs(normal[0]) < 0.9_d) ? Vector3D(1_d, 0_d, 0_d) : Vector3D(0_d, 0_d, 1_d);
    u            = (normal.crossProduct(ref)).getNormalised();
    v            = (normal.crossProduct(u)).getNormalised();
}
Vector3D Plane::projectPoint(const Vector3D& point) const
{
    decimal dist = (point - getPosition()).dotProduct(normal);
    return point - dist * normal;
}

bool Plane::containsPoint(const Vector3D& point) const
{
    Vector3D local = point - getPosition();
    decimal  s     = local.dotProduct(u);
    decimal  t     = local.dotProduct(v);
    return (std::abs(s) <= halfWidth && std::abs(t) <= halfHeight);
}

// ============================================================================
//  Collision
// ============================================================================
bool Plane::planeCollision(const Plane& plane) { return ::planeCollision(*this, plane); }

bool Plane::planeSphereCollision(const Sphere& sphere) { return ::planeSphereCollision(*this, sphere); }

bool Plane::planeAABBCollision(const AABB& aabb) { return ::planeAABBCollision(*this, aabb); }
