#include "world/physics.hpp"

#include "mathematics/common.hpp"

#include <cmath>

// ============================================================================
//  Helpers
// ============================================================================

/**
 * @brief Compute the reduced mass of two interacting bodies.
 *
 * The reduced mass is defined as:
 * \f[ \mu = \frac{m_1 m_2}{m_1 + m_2} \f]
 *
 * It is used in many two-body formulations (e.g., springs, damping).
 * Returns 0 if either mass is non-positive.
 *
 * @param m1 Mass of the first object.
 * @param m2 Mass of the second object.
 * @return The reduced mass (0 if invalid masses).
 */
decimal Physics::reducedMass(decimal m1, decimal m2)
{
    if (commonMaths::approxSmallerOrEqualThan(m1, 0_d) || commonMaths::approxSmallerOrEqualThan(m2, 0_d))
        return 0_d;
    return (m1 * m2) / (m1 + m2);
}

/**
 * @brief Compute the effective stiffness of two springs in series.
 *
 * Formula:
 * \f[ k = \frac{k_1 k_2}{k_1 + k_2} \f]
 *
 * This harmonic mean ensures that the effective stiffness is dominated
 * by the softer spring.
 *
 * @param k1 Stiffness constant of the first object.
 * @param k2 Stiffness constant of the second object.
 * @return Effective stiffness constant.
 */
decimal Physics::effectiveStiffness(decimal k1, decimal k2)
{
    if (commonMaths::approxSmallerOrEqualThan(k1, 0_d))
        return k2;
    if (commonMaths::approxSmallerOrEqualThan(k2, 0_d))
        return k1;
    return (k1 * k2) / (k1 + k2);
}

/**
 * @brief Compute damping ratio (ζ) from restitution coefficient (e).
 *
 * The damping ratio describes the level of damping in an impact system.
 * Using:
 * \f[ \zeta = -\frac{\ln(e)}{\sqrt{\pi^2 + (\ln(e))^2}} \f]
 *
 * - If e ≤ 0 → ζ = 1 (overdamped, inelastic)
 * - If e ≥ 1 → ζ = 0 (no damping, perfectly elastic)
 *
 * @param e Coefficient of restitution (0 ≤ e ≤ 1).
 * @return Damping ratio (ζ).
 */
decimal Physics::dampingRatioFromRestitution(decimal e)
{
    if (e <= 0_d)
        return 1.0_d;
    if (e >= 1.0_d)
        return 0.0_d;
    decimal ln_e = std::log(e);
    return -ln_e / std::sqrt(std::numbers::pi_v<decimal> * std::numbers::pi_v<decimal> + ln_e * ln_e);
}

// ============================================================================
//  Forces
// ============================================================================

/**
 * @brief Compute gravitational acceleration vector.
 * @param g Gravitational constant (usually -9.81).
 * @return Gravity acceleration as a 3D vector (along Z-axis).
 */
Vector3D Physics::computeGravityAcc(decimal g) { return Vector3D(0_d, 0_d, -g); }

/**
 * @brief Compute gravitational force on a given object.
 * @param g Gravitational constant (usually -9.81).
 * @param obj Target object.
 * @return Gravitational force vector.
 */
Vector3D Physics::computeGravityForce(decimal g, const Object& obj)
{
    return Vector3D(0_d, 0_d, -obj.getMass() * g);
}

/**
 * @brief Compute spring (Hooke’s law) force between two objects.
 *
 * Force applied on obj1 due to obj2:
 * \f[ \mathbf{F}_{1} = -k (\mathbf{x_2} - \mathbf{x_1}) \f]
 *
 * The direction is opposite to the displacement vector, restoring equilibrium.
 *
 * @param obj1 First object.
 * @param obj2 Second object.
 * @return Spring force vector acting on obj1.
 */
Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    decimal  k = effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
    if (commonMaths::approxEqual(k, 0_d) || r.isNull())
        return Vector3D(0_d);
    return -k * r;
}

/**
 * @brief Compute spring force using a fixed stiffness constant.
 *
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param k Stiffness constant.
 * @return Spring force vector acting on obj1.
 */
Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2, decimal k)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (commonMaths::approxEqual(k, 0_d) || r.isNull())
        return Vector3D(0_d);
    return -k * r;
}

/**
 * @brief Compute damping (viscous) force along the normal between two objects.
 *
 * Formula:
 * \f[ F_d = -c (v_{rel} \cdot n) n \f]
 * with:
 * \f[ c = 2 \zeta \sqrt{k \mu} \f]
 *
 * @param obj1 First object.
 * @param obj2 Second object.
 * @return Damping force acting on obj1.
 */
Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2)
{
    decimal k_rel = effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
    decimal mu    = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k_rel, 0_d) || commonMaths::approxEqual(mu, 0_d))
        return Vector3D(0_d);

    decimal e    = std::sqrt(obj1.getRestitutionCst() * obj2.getRestitutionCst());
    decimal zeta = dampingRatioFromRestitution(e);
    decimal c    = 2_d * zeta * std::sqrt(k_rel * mu);

    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    return -c * vn * n;
}

/**
 * @brief Compute damping force using a fixed restitution coefficient.
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param e Coefficient of restitution.
 * @return Damping force acting on obj1.
 */
Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2, decimal e)
{
    decimal k_rel = effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
    decimal mu    = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k_rel, 0_d) || commonMaths::approxEqual(mu, 0_d))
        return Vector3D(0_d);

    decimal zeta = dampingRatioFromRestitution(e);
    decimal c    = 2_d * zeta * std::sqrt(k_rel * mu);

    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    return -c * vn * n;
}

/**
 * @brief Compute damping force using explicit restitution and stiffness.
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param e Coefficient of restitution.
 * @param k Stiffness constant.
 * @return Damping force acting on obj1.
 */
Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2, decimal e, decimal k)
{
    decimal mu = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k, 0_d) || commonMaths::approxEqual(mu, 0_d))
        return Vector3D(0_d);

    decimal zeta = dampingRatioFromRestitution(e);
    decimal c    = 2_d * zeta * std::sqrt(k * mu);

    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    return -c * vn * n;
}

/**
 * @brief Compute total normal force (spring + damping).
 * @param obj1 First object.
 * @param obj2 Second object.
 * @return Normal contact force.
 */
Vector3D Physics::computeNormalForce(const Object& obj1, const Object& obj2)
{
    return computeSpringForce(obj1, obj2) + computeDampingForce(obj1, obj2);
}

/**
 * @brief Compute total normal force using explicit restitution and stiffness.
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param e Coefficient of restitution.
 * @param k Stiffness constant.
 * @return Normal contact force.
 */
Vector3D Physics::computeNormalForce(const Object& obj1, const Object& obj2, decimal e, decimal k)
{
    return computeSpringForce(obj1, obj2, k) + computeDampingForce(obj1, obj2, e, k);
}

/**
 * @brief Compute tangential friction force between two objects.
 *
 * Approximated using a Coulomb-like model:
 * \f[ F_t = -\mu |N| \hat{t} \f]
 * where \f$ \hat{t} \f$ is the unit tangent of relative motion.
 *
 * @param obj1 First object.
 * @param obj2 Second object.
 * @return Frictional force acting on obj1.
 */
Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2)
{
    decimal mu = std::sqrt(obj1.getFrictionCst() * obj2.getFrictionCst());
    return computeFrictionForce(obj1, obj2, mu);
}

/**
 * @brief Compute tangential friction force using explicit friction coefficient.
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param mu Friction coefficient.
 * @return Friction force acting on obj1.
 */
Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull() || commonMaths::approxEqual(mu, 0_d))
        return Vector3D(0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan = v_rel - (v_rel.dotProduct(n) * n);
    if (v_tan.isNull())
        return Vector3D(0_d);

    Vector3D normalForce = computeNormalForce(obj1, obj2);
    decimal  normalMag   = normalForce.getNorm();
    if (commonMaths::approxEqual(normalMag, 0_d))
        return Vector3D(0_d);

    return -mu * normalMag * v_tan.getNormalised();
}

/**
 * @brief Compute friction force using explicit friction, restitution, and stiffness constants.
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param mu Friction coefficient.
 * @param e Coefficient of restitution.
 * @param k Stiffness constant.
 * @return Frictional force acting on obj1.
 */
Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu, decimal e,
                                       decimal k)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull() || commonMaths::approxEqual(mu, 0_d))
        return Vector3D(0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan = v_rel - (v_rel.dotProduct(n) * n);
    if (v_tan.isNull())
        return Vector3D(0_d);

    Vector3D normalForce = computeNormalForce(obj1, obj2, e, k);
    decimal  normalMag   = normalForce.getNorm();
    if (commonMaths::approxEqual(normalMag, 0_d))
        return Vector3D(0_d);

    return -mu * normalMag * v_tan.getNormalised();
}

/**
 * @brief Compute full contact force (normal + friction).
 *
 * This represents the total force applied on obj1 due to obj2.
 *
 * @param obj1 First object.
 * @param obj2 Second object.
 * @return Contact force vector.
 */
Vector3D Physics::computeContactForce(const Object& obj1, const Object& obj2)
{
    Vector3D normal   = computeNormalForce(obj1, obj2);
    Vector3D friction = computeFrictionForce(obj1, obj2);
    return normal + friction;
}

/**
 * @brief Compute full contact force (normal + friction) with explicit constants.
 * @param obj1 First object.
 * @param obj2 Second object.
 * @param mu Friction coefficient.
 * @param e Coefficient of restitution.
 * @param k Stiffness constant.
 * @return Contact force vector.
 */
Vector3D Physics::computeContactForce(const Object& obj1, const Object& obj2, decimal mu, decimal e,
                                      decimal k)
{
    Vector3D normal   = computeNormalForce(obj1, obj2, e, k);
    Vector3D friction = computeFrictionForce(obj1, obj2, mu, e, k);
    return normal + friction;
}
