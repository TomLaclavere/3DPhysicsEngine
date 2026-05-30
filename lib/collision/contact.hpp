/**
 * @file contact.hpp
 * @brief Contact data structure used in narrow collision phase.
 */
#pragma once

#include "mathematics/vector.hpp"

// Forward declaration
class Object;

/**
 * @brief Holds the result of a narrow-phase collision query.
 *
 * Filled by `NarrowCollision::computeContact` and consumed by collision
 * response and force-application routines.
 */
struct Contact
{
    Vector3D position;    /// World-space contact point.
    Vector3D normal;      /// Collision normal, pointing from B toward A.
    decimal  penetration; /// Depth of interpenetration (>0 when overlapping).

    const Object* A; /// First object involved in the collision.
    const Object* B; /// Second object involved in the collision.
};
