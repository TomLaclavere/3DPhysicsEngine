/**
 * @file narrow_collision.hpp
 * @brief Declaration of narrow collision phase functions between physics objects.
 *
 * Provides accurate and expensive computations to test collision between objects, and if so, to compute
 * collision properties : contact point position, penetration lenght and normal vector.
 *
 */

#pragma once

#include "contact.hpp"
#include "shapes.hpp"

namespace NarrowCollision {

// Sphere vs Sphere
bool computeContact(const SphereShape&, const SphereShape&, Contact& contact);
// Sphere vs Plane
bool computeContact(const SphereShape&, const PlaneShape&, Contact& contact);
// Sphere vs AABB
bool computeContact(const SphereShape&, const AABBShape&, Contact& contact);
// AABB vs AABB
bool computeContact(const AABBShape&, const AABBShape&, Contact& contact);
// AABB vs Plane
bool computeContact(const AABBShape&, const PlaneShape&, Contact& contact);
// Plane vs Plane
bool computeContact(const PlaneShape&, const PlaneShape&, Contact& contact);

// Symmetric versions (defined in terms of the asymmetric ones)
inline bool computeContact(const PlaneShape& a, const SphereShape& b, Contact& contact)
{
    return computeContact(b, a, contact);
}
inline bool computeContact(const AABBShape& a, const SphereShape& b, Contact& contact)
{
    return computeContact(b, a, contact);
}
inline bool computeContact(const PlaneShape& a, const AABBShape& b, Contact& contact)
{
    return computeContact(b, a, contact);
}

template <typename A, typename B>
    requires requires(const A& a, const B& b) {
        shapeOf(a);
        shapeOf(b);
    }
inline bool computeContact(const A& a, const B& b, Contact& contact)
{
    return computeContact(shapeOf(a), shapeOf(b), contact);
}
} // namespace NarrowCollision
