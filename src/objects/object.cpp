/**
 * @file object.cpp
 * @brief Implementation of Object class methods.
 *
 * Implements all member functions of the Object class declared in object.hpp,.
 *
 * This file includes:
 *  - Integration of motion equations.
 *
 * @see object.hpp
 */
#include "objects/object.hpp"

// ============================================================================
//  Apply forces / torques
// ============================================================================
/**
 * Default implementation uses simple Euler integration.
 * Can be overridden by derived classes for more complex behaviors.
 * @param dt Time step for integration (`decimal`).
 */
void Object::integrate(decimal dt)
{
    velocity += acceleration * dt;
    position += velocity * dt;
}
