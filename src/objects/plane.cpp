/**
 * @file plane.cpp
 * @brief Implementation of utilities and collision detection methods for Plane.
 *
 * This file provides the concrete implementations of collision checks involving Planes:
 * - Plane vs. Plane
 * - Plane vs. Sphere
 * - Plane vs. AABB
 *
 * It also implements the polymorphic collision check used by the Object hierarchy.
 *
 * @see plane.hpp
 */
#include "objects/plane.hpp"

#include "objects/collision.hpp"

// ============================================================================
//  Getters
// ============================================================================
ObjectType      Plane::getType() const { return ObjectType::Plane; }
const Vector3D& Plane::getNormal() const { return normal; }
const Vector3D& Plane::getU() const { return u; }
const Vector3D& Plane::getV() const { return v; }
decimal         Plane::getHalfWidth() const { return halfWidth; }
decimal         Plane::getHalfHeight() const { return halfHeight; }

// ============================================================================
//  Utilities
// ============================================================================
void Plane::updateLocalAxes()
{
    // Choose an arbitrary vector not parallel to the normal
    Vector3D ref = (std::abs(normal[0]) < 0.9_d) ? Vector3D(1_d, 0_d, 0_d) : Vector3D(0_d, 0_d, 1_d);
    u            = (normal.crossProduct(ref)).getNormalised();
    v            = (u.crossProduct(normal)).getNormalised();
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
    return (commonMaths::approxSmallerOrEqualThan(std::abs(s), halfWidth) &&
            commonMaths::approxSmallerOrEqualThan(std::abs(t), halfHeight));
}

// ============================================================================
//  Collision
// ============================================================================
/**
 * @brief Checks collision between two finite planes.
 *
 * Collision occurs if the associated infinite planes intersects (they are not parallel), and if the
 * intersection line passes through both rectangles' bounds.
 *
 * @param plane Plane to test against.
 * @return true if the planes intersect, false otherwise.
 */
bool Plane::planeCollision(const Plane& plane) { return Collision::collide(*this, plane); }

/**
 * @brief Checks collision between a finite plane and a sphere.
 *
 * Collision occurs if the sphere's center-to-plane distance is smaller than the sphere radius, and if the
 * projection of the sphere center onto the plane lies within its rectangle bounds.
 *
 * @param sphere Sphere to test against.
 * @return true if the Plane and the Sphere intersect, false otherwise.
 */
bool Plane::planeSphereCollision(const Sphere& sphere) { return Collision::collide(*this, sphere); }

/**
 * @brief Checks collision between a finite plane and an aabb.
 *
 * Collision occurs if the signed distance from the AABB to the plane is less than or equal to the projection
 * radius of the AABB onto the plane normal.
 *
 * @param aabb AABB to test against.
 * @return true if the Plane and the AABB intersect, false otherwise.
 */
bool Plane::planeAABBCollision(const AABB& aabb) { return Collision::collide(*this, aabb); }

/**
 * @brief Polymorphic collision detection against another Object.
 *
 * Dispatches to the appropriate collision method depending on the type of
 * the other object.
 * Only collision between Plane, Sphere, and AABB are implemented.
 *
 * @param other Object to test against.
 * @return true if the objects intersect, false otherwise.
 */
bool Plane::checkCollision(const Object& other)
{
    switch (other.getType())
    {
    case ObjectType::Plane:
        return Plane::planeCollision(static_cast<const Plane&>(other));
    case ObjectType::Sphere:
        return Plane::planeSphereCollision(static_cast<const Sphere&>(other));
    case ObjectType::AABB:
        return Plane::planeAABBCollision(static_cast<const AABB&>(other));
    default:
        return false;
    }
}
