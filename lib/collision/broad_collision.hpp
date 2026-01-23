/**
 * @file broad_collision.hpp
 * @brief Declaration of broad collision phase functions between physics objects.
 *
 * Provides quick computations to test objects if objects can be colliding.
 *
 */

#pragma once

#include "contact.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

namespace BroadCollision {

// Sphere vs Sphere
bool isColliding(const Sphere&, const Sphere&);

// Sphere vs Plane
bool isColliding(const Sphere&, const Plane&);

// Sphere vs AABB
bool isColliding(const Sphere&, const AABB&);

// AABB vs AABB
bool isColliding(const AABB&, const AABB&);

// AABB vs Plane
bool isColliding(const AABB&, const Plane&);

// Plane vs Plane
bool isColliding(const Plane&, const Plane&);

// Symmetric versions (defined in terms of the asymmetric ones)
inline bool isColliding(const Plane& a, const Sphere& b) { return isColliding(b, a); }

inline bool isColliding(const AABB& a, const Sphere& b) { return isColliding(b, a); }

inline bool isColliding(const Plane& a, const AABB& b) { return isColliding(b, a); }

} // namespace BroadCollision
