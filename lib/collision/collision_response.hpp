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
#include "objects/object.hpp"

/**
 * @brief Corrects the positions of two objects to resolve interpenetration.
 *
 * This function applies a positional correction along the contact normal to prevent
 * objects from sinking into each other. It moves each object proportionally to its
 * inverse mass. Small penetrations below a given slop are ignored.
 *
 * @param A First object involved in the collision.
 * @param B Second object involved in the collision.
 * @param contact Contact information including normal and penetration depth.
 * @param percent The percentage of the penetration to correct (default 0.8).
 * @param slop Minimum penetration depth to consider for correction (default 0.01).
 */
void positionCorrection(Object& A, Object& B, Contact& contact, decimal percent = 0.8_d,
                        decimal slop = 0.01_d);

/**
 * @brief Resolves a collision between two objects by updating their velocities.
 *
 * This function implements a basic linear impulse-based collision response:
 *  1) Applies positional correction to resolve penetration.
 *  2) Computes relative velocity along the contact normal.
 *  3) Applies an impulse to each object according to the restitution coefficient
 *     and their inverse masses.
 *
 * @param A First object involved in the collision.
 * @param B Second object involved in the collision.
 * @param contact Contact information including normal and penetration depth.
 * @param restitution Coefficient of restitution [0, 1], where 0 = perfectly inelastic,
 *                    1 = perfectly elastic (default 0.5).
 */
void reboundCollision(Object& A, Object& B, Contact& contact, decimal restitution = 0.5_d);
