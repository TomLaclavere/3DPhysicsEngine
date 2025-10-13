#include "world/physics.hpp"

#include <algorithm>
#include <cmath>

namespace {
using std::abs;
using std::sqrt;

// Reduced mass mu = (m1*m2)/(m1+m2). Return 0 if non-positive masses.
decimal reducedMass(decimal m1, decimal m2)
{
    if (commonMaths::approxSmallerOrEqualThan(m1, decimal(0)) ||
        commonMaths::approxSmallerOrEqualThan(m2, decimal(0)))
        return 0_d;
    return (m1 * m2) / (m1 + m2);
}

// Effective stiffness for two springs in series (harmonic mean).
decimal effectiveStiffness(decimal k1, decimal k2)
{
    if (commonMaths::approxSmallerOrEqualThan(k1, decimal(0)))
        return k2;
    if (commonMaths::approxSmallerOrEqualThan(k2, decimal(0)))
        return k1;
    return (k1 * k2) / (k1 + k2);
}

// Convert restitution coefficient e -> damping ratio zeta.
// For e <= 0 we return 1 (high damping), for e >= 1 we return 0 (no damping).
decimal dampingRatioFromRestitution(decimal e)
{
    if (e <= 0_d)
        return 1.0_d;
    if (e >= 1.0_d)
        return 0.0_d;
    decimal ln_e = std::log(e);
    return -ln_e / std::sqrt((decimal)M_PI * (decimal)M_PI + ln_e * ln_e);
}
} // namespace

// Gravity (vector along Z)
Vector3D Physics::computeGravityAcc(decimal g) { return Vector3D(0_d, 0_d, g); }

Vector3D Physics::computeGravityForce(decimal g, const Object& obj)
{
    // F = m * g
    return Vector3D(0_d, 0_d, obj.getMass() * g);
}

// Stiffness combined
decimal Physics::computeStiffnessRel(const Object& obj1, const Object& obj2)
{
    return effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
}

// Hooke spring force. Sign convention: return force applied ON obj1 from obj2
// r = pos2 - pos1, force_on_1 = -k * r  (restores toward equilibrium)
Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    decimal  k = computeStiffnessRel(obj1, obj2);
    if (commonMaths::approxEqual(k, decimal(0)) || r.isNull())
        return Vector3D(0_d, 0_d, 0_d);
    return -k * r;
}
Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2, decimal k)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (commonMaths::approxEqual(k, decimal(0)) || r.isNull())
        return Vector3D(0_d, 0_d, 0_d);
    return -k * r;
}

// Damping force along normal. Returns force applied ON obj1 from obj2.
// Uses effective stiffness k_rel and reduced mass mu to compute viscous coeff c.
Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2)
{
    decimal k_rel = computeStiffnessRel(obj1, obj2);
    decimal mu    = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k_rel, decimal(0)) || commonMaths::approxEqual(mu, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    decimal e    = std::sqrt(obj1.getRestitutionCst() * obj2.getRestitutionCst());
    decimal zeta = dampingRatioFromRestitution(e);
    decimal c    = 2_d * zeta * std::sqrt(k_rel * mu);

    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    return -c * vn * n;
}

Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2, decimal e)
{
    decimal k_rel = computeStiffnessRel(obj1, obj2);
    decimal mu    = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k_rel, decimal(0)) || commonMaths::approxEqual(mu, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    decimal zeta = dampingRatioFromRestitution(e);
    decimal c    = 2_d * zeta * std::sqrt(k_rel * mu);

    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    return -c * vn * n;
}

Vector3D Physics::computeDampingForce(const Object& obj1, const Object& obj2, decimal e, decimal k)
{
    decimal mu = reducedMass(obj1.getMass(), obj2.getMass());
    if (commonMaths::approxEqual(k, decimal(0)) || commonMaths::approxEqual(mu, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    decimal zeta = dampingRatioFromRestitution(e);
    decimal c    = 2_d * zeta * std::sqrt(k * mu);

    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    decimal  vn    = v_rel.dotProduct(n);

    return -c * vn * n;
}

// Normal = spring + damping
Vector3D Physics::computeNormalForce(const Object& obj1, const Object& obj2)
{
    return computeSpringForce(obj1, obj2) + computeDampingForce(obj1, obj2);
}
Vector3D Physics::computeNormalForce(const Object& obj1, const Object& obj2, decimal e, decimal k)
{
    return computeSpringForce(obj1, obj2, k) + computeDampingForce(obj1, obj2, e, k);
}

// Friction: Coulomb-like viscous approximation: F_t = -mu * |N| * t_hat
Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2)
{
    decimal mu = std::sqrt(obj1.getFrictionCst() * obj2.getFrictionCst());
    return computeFrictionForce(obj1, obj2, mu);
}

Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull() || commonMaths::approxEqual(mu, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan = v_rel - (v_rel.dotProduct(n) * n);

    if (v_tan.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D normalForce = computeNormalForce(obj1, obj2);
    decimal  normalMag   = normalForce.getNorm();

    if (commonMaths::approxEqual(normalMag, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    return -mu * normalMag * v_tan.getNormalised();
}

Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu, decimal e,
                                       decimal k)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull() || commonMaths::approxEqual(mu, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan = v_rel - (v_rel.dotProduct(n) * n);

    if (v_tan.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D normalForce = computeNormalForce(obj1, obj2, e, k);
    decimal  normalMag   = normalForce.getNorm();

    if (commonMaths::approxEqual(normalMag, decimal(0)))
        return Vector3D(0_d, 0_d, 0_d);

    return -mu * normalMag * v_tan.getNormalised();
}

// Contact (normal + friction). Positive direction: force on obj1 due to obj2.
Vector3D Physics::computeContactForce(const Object& obj1, const Object& obj2)
{
    Vector3D normal   = computeNormalForce(obj1, obj2);
    Vector3D friction = computeFrictionForce(obj1, obj2);
    return normal + friction;
}

Vector3D Physics::computeContactForce(const Object& obj1, const Object& obj2, decimal mu, decimal e,
                                      decimal k)
{
    Vector3D normal   = computeNormalForce(obj1, obj2, e, k);
    Vector3D friction = computeFrictionForce(obj1, obj2, mu, e, k);
    return normal + friction;
}
