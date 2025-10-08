/**
 * @file collision.hpp
 * @brief Declaration of collision detection functions between physics objects.
 *
 * Provides type-specific and generic interfaces to test collisions between
 * objects used in the physics simulation (Sphere, AABB, Plane).
 *
 * The `collide<T1, T2>` function is the generic entry point.
 * Symmetric overloads are automatically generated to avoid code duplication.
 */

#pragma once

#include "aabb.hpp"
#include "plane.hpp"
#include "sphere.hpp"

namespace Collision {

// ============================================================================
// Generic dispatch entry point
// ============================================================================

/**
 * @brief Generic collision dispatch function.
 *
 * Specialized for supported pairs of object types.
 *
 * @tparam T1 First object type
 * @tparam T2 Second object type
 * @param a First object
 * @param b Second object
 * @return true if objects intersect, false otherwise.
 */
template <typename T1, typename T2>
bool collide(const T1& a, const T2& b);

// ============================================================================
// Explicit specializations
// ============================================================================

// --- Sphere vs Sphere ---
template <>
bool collide<Sphere, Sphere>(const Sphere&, const Sphere&);

// --- Sphere vs Plane ---
template <>
bool collide<Sphere, Plane>(const Sphere&, const Plane&);

// --- Sphere vs AABB ---
template <>
bool collide<Sphere, AABB>(const Sphere&, const AABB&);

// --- AABB vs AABB ---
template <>
bool collide<AABB, AABB>(const AABB&, const AABB&);

// --- AABB vs Plane ---
template <>
bool collide<AABB, Plane>(const AABB&, const Plane&);

// --- Plane vs Plane ---
template <>
bool collide<Plane, Plane>(const Plane&, const Plane&);

// Symmetric versions (defined in terms of the asymmetric ones)
template <>
inline bool collide<Plane, Sphere>(const Plane& a, const Sphere& b)
{
    return collide(b, a);
}

template <>
inline bool collide<AABB, Sphere>(const AABB& a, const Sphere& b)
{
    return collide(b, a);
}

template <>
inline bool collide<Plane, AABB>(const Plane& a, const AABB& b)
{
    return collide(b, a);
}

} // namespace Collision
