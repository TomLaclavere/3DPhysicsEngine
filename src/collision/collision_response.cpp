#include "collision/collision_response.hpp"

#include <algorithm>

void positionCorrection(Object& A, Object& B, Contact& contact, decimal percent, decimal slop)
{
    if (contact.penetration <= slop)
        return;

    auto invMassA = A.getMass() > 0_d ? 1_d / A.getMass() : 0_d;
    auto invMassB = B.getMass() > 0_d ? 1_d / B.getMass() : 0_d;
    if (invMassA + invMassB <= 0_d)
        return;

    decimal  invMassSum = invMassA + invMassB;
    Vector3D correction =
        contact.normal * ((std::max(contact.penetration - slop, 0_d) / invMassSum) * percent);

    A.setPosition(A.getPosition() - correction * invMassA);
    B.setPosition(B.getPosition() + correction * invMassB);
}

void reboundCollision(Object& A, Object& B, Contact& contact, decimal restitution)
{
    decimal invMassA = A.getMass() > 0_d ? 1_d / A.getMass() : 0_d;
    decimal invMassB = B.getMass() > 0_d ? 1_d / B.getMass() : 0_d;

    decimal invMassSum = invMassA + invMassB;
    if (invMassSum <= 0_d)
        return;

    // Positional correction
    positionCorrection(A, B, contact);

    Vector3D va = A.getVelocity();
    Vector3D vb = B.getVelocity();

    // Relative velocity
    Vector3D relVel         = vb - va;
    decimal  velAlongNormal = relVel.dotProduct(contact.normal);

    if (velAlongNormal <= 0_d)
        return;

    decimal e = std::clamp(restitution, 0_d, 1_d);
    decimal j = (1_d + e) * velAlongNormal / invMassSum;

    Vector3D impulse = contact.normal * j;

    A.setVelocity(va - impulse * invMassA);
    B.setVelocity(vb + impulse * invMassB);
}
