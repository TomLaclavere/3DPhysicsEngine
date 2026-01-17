/**
 * @file narrow_collision.hpp
 * @brief Declaration of narrow collision phase functions between physics objects.
 *
 * Provides accurate and expensive computations to test collision between objects, and if so, to compute
 * collision properties : contact point position, penetration lenght and normal vector.
 *
 */

#pragma once

#include "aabb.hpp"
#include "collision/contact.hpp"
#include "plane.hpp"
#include "sphere.hpp"

namespace NarrowCollision {

// Sphere vs Sphere
bool computeContact(const Sphere&, const Sphere&, Contact& contact);

// Sphere vs Plane
bool computeContact(const Sphere&, const Plane&, Contact& contact);

// Sphere vs AABB
bool computeContact(const Sphere&, const AABB&, Contact& contact);

// AABB vs AABB
bool computeContact(const AABB&, const AABB&, Contact& contact);

// AABB vs Plane
bool computeContact(const AABB&, const Plane&, Contact& contact);

// Plane vs Plane
bool computeContact(const Plane&, const Plane&, Contact& contact);

// Symmetric versions (defined in terms of the asymmetric ones)
inline bool computeContact(const Plane& a, const Sphere& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

inline bool computeContact(const AABB& a, const Sphere& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

inline bool computeContact(const Plane& a, const AABB& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

} // namespace NarrowCollision
