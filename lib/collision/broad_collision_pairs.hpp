/**
 * @file broad_collision.hpp
 * @brief Declaration of broad collision phase functions between physics objects.
 *
 * Provides quick computations to test objects if objects can be colliding.
 *
 */

#pragma once

#include "shapes.hpp"

namespace BoundingSphereTest {
// Test if objects are overlapping if we condider them as Sphere build fromt their largest size. O(N²), but
// more accurate.

// Sphere vs Sphere
bool isColliding(const SphereShape&, const SphereShape&);

// Sphere vs Plane
bool isColliding(const SphereShape&, const PlaneShape&);

// Sphere vs AABB
bool isColliding(const SphereShape&, const AABBShape&);

// AABB vs AABB
bool isColliding(const AABBShape&, const AABBShape&);

// AABB vs Plane
bool isColliding(const AABBShape&, const PlaneShape&);

// Plane vs Plane
bool isColliding(const PlaneShape&, const PlaneShape&);

// Symmetric versions (defined in terms of the asymmetric ones)
inline bool isColliding(const PlaneShape& a, const SphereShape& b) { return isColliding(b, a); }

inline bool isColliding(const AABBShape& a, const SphereShape& b) { return isColliding(b, a); }

inline bool isColliding(const PlaneShape& a, const AABBShape& b) { return isColliding(b, a); }

template <typename A, typename B>
    requires requires(const A& a, const B& b) {
        shapeOf(a);
        shapeOf(b);
    }
inline bool isColliding(const A& a, const B& b)
{
    return isColliding(shapeOf(a), shapeOf(b));
}

} // namespace BoundingSphereTest
