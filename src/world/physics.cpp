#include "world/physics.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
using std::abs;
using std::sqrt;

// reduced mass mu = (m1 * m2) / (m1 + m2); returns 0 if either mass is <= 0
decimal reducedMass(decimal m1, decimal m2)
{
    if (commonMaths::approxSmallerOrEqualThan(m1, decimal(0)) ||
        commonMaths::approxSmallerOrEqualThan(m2, decimal(0)))
        return 0_d;
    return (m1 * m2) / (m1 + m2);
}

// effective stiffness for two springs in series (harmonic mean).
// if both zero -> 0, if one zero -> the other.
decimal effectiveStiffness(decimal k1, decimal k2)
{
    if (commonMaths::approxSmallerOrEqualThan(k1, decimal(0)))
        return k2; // if k2 also 0 -> returns 0
    if (commonMaths::approxSmallerOrEqualThan(k2, decimal(0)))
        return k1;
    // harmonic: 1/k = 1/k1 + 1/k2  => k = k1*k2/(k1+k2)
    return (k1 * k2) / (k1 + k2);
}

// compute damping ratio (zeta) from restitution coefficient e (0<e<=1).
// If e <= 0 -> large damping (zeta ~ 1). If e >= 1 -> 0 damping.
decimal dampingRatioFromRestitution(decimal e)
{
    if (e <= 0_d)
        return 1.0_d; // heavily damped / inelastic
    if (e >= 1.0_d)
        return 0.0_d;
    decimal ln_e = std::log(e);
    // zeta = -ln(e) / sqrt(pi^2 + ln(e)^2)
    return -ln_e / std::sqrt((decimal)M_PI * (decimal)M_PI + ln_e * ln_e);
}
} // namespace

// Gravity
Vector3D Physics::computeGravityAcc(decimal g) { return Vector3D(0_d, 0_d, g); }

Vector3D Physics::computeGravityForce(decimal g, const Object& obj)
{
    // F = m * g (vector)
    return Vector3D(0_d, 0_d, obj.getMass() * g);
}

// Stiffness
decimal Physics::computeStiffnessRel(const Object& obj1, const Object& obj2)
{
    return effectiveStiffness(obj1.getStiffnessCst(), obj2.getStiffnessCst());
}

// Spring (Hooke) force: F = -k * x (x = displacement). Keep sign convention explicit.
// Here r = pos2 - pos1 (displacement of 2 w.r.t 1). If you want force on obj1 due to obj2:
// F_on_1 = -k * r. We return vector force magnitude following typical Hooke convention: -k*r.
Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    decimal  k = computeStiffnessRel(obj1, obj2);
    if (commonMaths::approxEqual(k, decimal(0)) || r.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    return -k * r; // negative sign: restores towards equilibrium
}

Vector3D Physics::computeSpringForce(const Object& obj1, const Object& obj2, decimal k)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (commonMaths::approxEqual(k, decimal(0)) || r.isNull())
        return Vector3D(0_d, 0_d, 0_d);
    return -k * r;
}

// Damping force (viscous) along normal direction.
// We compute c = 2 * zeta * sqrt(k * mu), then F_damp = - c * (v_rel · n) * n
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

// Normal force = spring + damping (both along the contact normal)
Vector3D Physics::computeNormalForce(const Object& obj1, const Object& obj2)
{
    return computeSpringForce(obj1, obj2) + computeDampingForce(obj1, obj2);
}
Vector3D Physics::computeNormalForce(const Object& obj1, const Object& obj2, decimal e, decimal k)
{
    return computeSpringForce(obj1, obj2, k) + computeDampingForce(obj1, obj2, e, k);
}

// Friction: F_t = -mu * |N| * t_hat , where t_hat is unit tangent direction (opposite v_tan)
Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2)
{
    decimal mu = std::sqrt(obj1.getFrictionCst() * obj2.getFrictionCst());
    return computeFrictionForce(obj1, obj2, mu);
}

Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan = v_rel - (v_rel.dotProduct(n) * n);

    if (v_tan.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D normalForce = computeNormalForce(obj1, obj2);
    decimal  normalMag   = normalForce.getNorm();

    return -mu * normalMag * v_tan.getNormalised(); // direction opposite tangential velocity
}

Vector3D Physics::computeFrictionForce(const Object& obj1, const Object& obj2, decimal mu, decimal e,
                                       decimal k)
{
    Vector3D r = obj2.getPosition() - obj1.getPosition();
    if (r.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D n     = r.getNormalised();
    Vector3D v_rel = obj2.getVelocity() - obj1.getVelocity();
    Vector3D v_tan = v_rel - (v_rel.dotProduct(n) * n);
    if (v_tan.isNull())
        return Vector3D(0_d, 0_d, 0_d);

    Vector3D normalForce = computeNormalForce(obj1, obj2, e, k);
    decimal  normalMag   = normalForce.getNorm();

    return -mu * normalMag * v_tan.getNormalised();
}

// Contact force: normal + friction
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
