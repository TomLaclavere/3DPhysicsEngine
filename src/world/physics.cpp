#include "world/physics.hpp"

#include "collision/contact.hpp"
#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "precision.hpp"

#include <cmath>

// ========= Helpers =========

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
    // Fixed object → infinite mass → μ = other's mass
    if (commonMaths::approxSmallerOrEqualThan(m1, 0_d))
        return m2;
    if (commonMaths::approxSmallerOrEqualThan(m2, 0_d))
        return m1;
    return (m1 * m2) / (m1 + m2);
}

/**
 * @brief Compute the effective Stiffness of two springs in series.
 *
 * Formula:
 * \f[ k = \frac{k_1 k_2}{k_1 + k_2} \f]
 *
 * This harmonic mean ensures that the effective Stiffness is dominated
 * by the softer spring.
 *
 * @param k1 Stiffness constant of the first object.
 * @param k2 Stiffness constant of the second object.
 * @return Effective Stiffness constant.
 */
decimal Physics::effectiveStiffness(decimal k1, decimal k2)
{
    if (commonMaths::approxSmallerOrEqualThan(k1, 0_d))
        return k2;
    if (commonMaths::approxSmallerOrEqualThan(k2, 0_d))
        return k1;
    return (k1 * k2) / (k1 + k2);
}
decimal Physics::effectiveDamping(decimal d1, decimal d2)
{
    if (commonMaths::approxSmallerOrEqualThan(d1, 0_d))
        return d2;
    if (commonMaths::approxSmallerOrEqualThan(d2, 0_d))
        return d1;
    return (d1 * d2) / (d1 + d2);
}
decimal Physics::effectiveFriction(decimal mu1, decimal mu2) { return std::sqrt(mu1 * mu2); }

// ========= Forces =========

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
    return { 0_d, 0_d, -obj.getMass() * g };
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
Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2, Contact& contact)
{
    decimal r = contact.penetration;
    if (r <= 0_d)
        return Vector3D(0_d);

    decimal k = effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
    if (commonMaths::approxEqual(k, 0_d))
        return Vector3D(0_d);

    Vector3D n = contact.normal;
    // if ((obj1.getPosition() - obj2.getPosition()).dotProduct(n) < 0_d)
    //     n = -n;

    return k * r * n;
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
Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2, Contact& contact)
{
    if (contact.penetration <= 0_d)
        return Vector3D(0_d);

    decimal k_rel = effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
    decimal mu    = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k_rel, 0_d) || commonMaths::approxEqual(mu, 0_d))
        return Vector3D(0_d);

    decimal zeta = effectiveDamping(obj1.getDampingCst(), obj2.getDampingCst());
    decimal c    = 2_d * zeta * std::sqrt(k_rel * mu);

    Vector3D n = contact.normal;
    // if ((obj1.getPosition() - obj2.getPosition()).dotProduct(n) < 0_d)
    //     n = -n;
    Vector3D v_rel  = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn     = v_rel.dotProduct(n);
    decimal  F_damp = -c * std::min(vn, 0_d);
    return F_damp * n;
}

/**
 * @brief Compute total normal force (spring + damping).
 * @param obj1 First object.
 * @param obj2 Second object.
 * @return Normal contact force.
 */
Vector3D Physics::computeNormalForces(const Object& obj1, const Object& obj2, Contact& contact)
{
    decimal delta = contact.penetration;
    if (delta <= 0_d)
        return Vector3D(0_d);

    Vector3D n = contact.normal;
    if ((obj1.getPosition() - obj2.getPosition()).dotProduct(n) < 0_d)
        n = -n;
    decimal k    = effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
    decimal mu   = reducedMass(obj1.getMass(), obj2.getMass());
    decimal zeta = effectiveDamping(obj1.getDampingCst(), obj2.getDampingCst());
    decimal c    = 2_d * zeta * std::sqrt(k * mu);

    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    decimal F_spring = k * delta;
    decimal F_damp   = c * std::min(vn, 0_d);

    // Force is not allowed to be tractive (no adhesion).
    decimal F_normal = std::max(F_spring + F_damp, 0_d);

    return F_normal * n;
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
Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, Contact& contact,
                                       decimal F_normal_mag)
{
    if (commonMaths::approxEqual(F_normal_mag, 0_d))
        return Vector3D(0_d);

    decimal mu   = effectiveFriction(obj1.getFrictionCst(), obj2.getFrictionCst());
    decimal mu_k = mu;         // Dynamic friction
    decimal mu_s = 1.1_d * mu; // Static friction ~ 10% higher than dynamic one

    Vector3D n         = contact.normal;
    Vector3D v_rel     = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan     = v_rel - v_rel.dotProduct(n) * n;
    decimal  v_tan_mag = v_tan.getNorm();

    // Regularization threshold: below this threshold, static friction occurs via smoothing.
    constexpr decimal V_EPSILON = 1e-4_d;

    if (v_tan_mag < V_EPSILON)
    {
        // Static friction: we oppose the residual sliding proportionally
        return -mu_s * F_normal_mag * (v_tan / V_EPSILON);
    }

    // Dynamic friction (Coulomb)
    return -mu_k * F_normal_mag * v_tan.getNormalised();
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
Vector3D Physics::computeContactForce(const Object& obj1, const Object& obj2, Contact& contact)
{
    Vector3D normal  = computeNormalForces(obj1, obj2, contact);
    Vector3D tangent = computeFrictionForce(obj1, obj2, contact, normal.getNorm());
    return normal + tangent;
}
