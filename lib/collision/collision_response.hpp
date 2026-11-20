/**
 * @file collision_response.hpp
 * @brief Declaration of collision response functions between physics objects.
 *
 * Provides functions to compute the back force applied on object in collision.
 *
 * @note Be careful, this file does not include angular correction for now.
 */

#pragma once

#include "contact.hpp"
#include "mathematics/vector.hpp"
#include "world/contact_body.hpp"

/**
 * @brief Computes the positions of two bodies after resolving interpenetration.
 *
 * This function applies a positional correction along the contact normal to prevent
 * bodies from sinking into each other. It moves each body proportionally to its
 * inverse mass. Small penetrations below a given slop are ignored.
 *
 * @param A First body involved in the collision.
 * @param B Second body involved in the collision.
 * @param contact Contact information including normal and penetration depth.
 * @param percent The percentage of the penetration to correct (default 0.8).
 * @param slop Minimum penetration depth to consider for correction (default 0.01).
 * @return The corrected positions of A and B (unchanged if no correction applies).
 *
 * The `Object&` overload writes the corrected positions back into the objects.
 */
std::pair<Vector3D, Vector3D> positionCorrection(const ContactBody& A, const ContactBody& B,
                                                 const Contact& contact, decimal percent = 0.8_d,
                                                 decimal slop = 0.01_d);
inline void positionCorrection(Object& A, Object& B, const Contact& contact, decimal percent = 0.8_d,
                               decimal slop = 0.01_d)
{
    const auto [xA, xB] = positionCorrection(contactBodyOf(A), contactBodyOf(B), contact, percent, slop);
    A.setPosition(xA);
    B.setPosition(xB);
}

/**
 * @brief Resolves a collision between two bodies: computes their new positions and velocities.
 *
 * This function implements a basic linear impulse-based collision response:
 *  1) Applies positional correction to resolve penetration.
 *  2) Computes relative velocity along the contact normal.
 *  3) Applies an impulse to each body according to the restitution coefficient
 *     (the smaller of the two, clamped to [0, 1]) and their inverse masses.
 *
 * @param A First body involved in the collision.
 * @param B Second body involved in the collision.
 * @param contact Contact information including normal and penetration depth.
 * @return The new positions and velocities of A and B. Velocities are unchanged if the bodies
 *         are separating; nothing changes if both have infinite mass.
 *
 * The `Object&` overload writes the result back into the objects.
 */
ContactResponse reboundCollision(const ContactBody& A, const ContactBody& B, const Contact& contact);

inline void reboundCollision(Object& A, Object& B, Contact& contact)
{
    const ContactResponse r = reboundCollision(contactBodyOf(A), contactBodyOf(B), contact);
    A.setPosition(r.positionA);
    B.setPosition(r.positionB);
    A.setVelocity(r.velocityA);
    B.setVelocity(r.velocityB);
}
