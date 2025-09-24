#include "objects/object.hpp"

// ============================================================================
// ============================================================================
//  Apply forces / torques
// ============================================================================
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
