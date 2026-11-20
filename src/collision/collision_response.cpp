/**
 * @file collision_response.cpp
 * @brief Implementation of impulse-based collision response: position correction and velocity rebound.
 */
#include "collision/collision_response.hpp"

#include "mathematics/vector.hpp"
#include "world/contact_body.hpp"

#include <algorithm>

std::pair<Vector3D, Vector3D> positionCorrection(const ContactBody& A, const ContactBody& B,
                                                 const Contact& contact, decimal percent, decimal slop)
{
    if (contact.penetration <= slop)
        return { A.position, B.position };

    auto    invMassA   = A.invMass;
    auto    invMassB   = B.invMass;
    decimal invMassSum = invMassA + invMassB;
    if (invMassSum <= 0_d)
        return { A.position, B.position };

    Vector3D n = contact.normal;
    // if ((A.position - B.position).dotProduct(n) < 0_d)
    //     n = -n;

    decimal  correctionMag = (std::max(contact.penetration - slop, 0_d) / invMassSum) * percent;
    Vector3D correction    = n * correctionMag;

    return { A.position + correction * invMassA, B.position - correction * invMassB };
}

ContactResponse reboundCollision(const ContactBody& A, const ContactBody& B, const Contact& contact)
{
    decimal invMassSum = A.invMass + B.invMass;

    if (invMassSum <= 0_d)
        return {
            .positionA = A.position, .positionB = B.position, .velocityA = A.velocity, .velocityB = B.velocity
        };

    Vector3D n = contact.normal;
    if ((A.position - B.position).dotProduct(n) < 0_d)
        n = -n;

    const auto [xa, xb] = positionCorrection(A, B, contact);

    Vector3D va = A.velocity;
    Vector3D vb = B.velocity;

    Vector3D relVel         = va - vb;
    decimal  velAlongNormal = relVel.dotProduct(n);
    if (velAlongNormal >= 0_d)
        return { .positionA = xa, .positionB = xb, .velocityA = va, .velocityB = vb };

    decimal e = std::clamp(std::min(A.restitution, B.restitution), 0_d, 1_d);
    decimal j = -(1_d + e) * velAlongNormal / invMassSum;

    Vector3D impulse = n * j;

    va += impulse * A.invMass;
    vb -= impulse * B.invMass;

    return { .positionA = xa, .positionB = xb, .velocityA = va, .velocityB = vb };
}
