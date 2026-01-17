/**
 * @file collision.hpp
 * @brief Declaration of collision detection functions between physics objects.
 *
 * Provides type-specific and generic interfaces to test collisions between
 * objects used in the physics simulation (Sphere, AABB, Plane).
 *
 * The `computeContact<T1, T2>` function is the generic entry point.
 * Symmetric overloads are automatically generated to avoid code duplication.
 */

#pragma once

#include "aabb.hpp"
#include "contact.hpp"
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
bool computeContact(const T1& a, const T2& b, Contact& contact);

// ============================================================================
// Explicit specializations
// ============================================================================

// --- Sphere vs Sphere ---
template <>
bool computeContact<Sphere, Sphere>(const Sphere&, const Sphere&, Contact& contact);

// --- Sphere vs Plane ---
template <>
bool computeContact<Sphere, Plane>(const Sphere&, const Plane&, Contact& contact);

// --- Sphere vs AABB ---
template <>
bool computeContact<Sphere, AABB>(const Sphere&, const AABB&, Contact& contact);

// --- AABB vs AABB ---
template <>
bool computeContact<AABB, AABB>(const AABB&, const AABB&, Contact& contact);

// --- AABB vs Plane ---
template <>
bool computeContact<AABB, Plane>(const AABB&, const Plane&, Contact& contact);

// --- Plane vs Plane ---
template <>
bool computeContact<Plane, Plane>(const Plane&, const Plane&, Contact& contact);

// Symmetric versions (defined in terms of the asymmetric ones)
template <>
inline bool computeContact<Plane, Sphere>(const Plane& a, const Sphere& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

template <>
inline bool computeContact<AABB, Sphere>(const AABB& a, const Sphere& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

template <>
inline bool computeContact<Plane, AABB>(const Plane& a, const AABB& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

} // namespace Collision
