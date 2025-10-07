/**
 * @file collision.hpp
 * @brief Definition of collision conditions between Object used in physics simulation.
 *
 * Defining methods to check collision between Object.
 * Currently, collision between Sphere, AABB, and Plane are implemented.
 *
 */
#pragma once

#include "aabb.hpp"
#include "plane.hpp"
#include "sphere.hpp"

// ============================================================================
/// @name Collision with Sphere
// ============================================================================
/// @{

/// Check collision between two Sphere.
bool sphereCollision(const Sphere& sphere1, const Sphere& sphere2);
/// Check collision between a Sphere and an AABB.
bool sphereAABBCollision(const Sphere& sphere, const AABB& aabb);
/// Check collision between a Sphere and a Plane.
bool spherePlaneCollision(const Sphere& sphere, const Plane& plane);
/// @}

// ============================================================================
/// @name Collision with AABB
// ============================================================================
/// @{

/// Check collision between two AABB.
bool aabbCollision(const AABB& aabb1, const AABB& aabb2);
/// Check collision between an AABB and a Sphere.
bool aabbSphereCollision(const AABB& aabb, const Sphere& sphere);
/// Check collision between an AABB and a Plane.
bool aabbPlaneCollision(const AABB& aabb, const Plane& plane);
/// @}

// ============================================================================
/// @name Collision with Plane
// ============================================================================
/// @{

/// Check collision between two Plane.
bool planeCollision(const Plane& plane1, const Plane& plane2);
/// Check collision between a Plane and a Sphere.
bool planeSphereCollision(const Plane& plane, const Sphere& sphere);
/// Check collision between a Plane and an AABB.
bool planeAABBCollision(const Plane& plane, const AABB& aabb);
/// @}
