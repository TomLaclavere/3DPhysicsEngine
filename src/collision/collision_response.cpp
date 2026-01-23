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

    decimal invMassA   = 1_d / A.getMass();
    decimal invMassB   = 1_d / B.getMass();
    decimal invMassSum = invMassA + invMassB;
    if (invMassSum <= 0_d)
        return;

    Vector3D correction =
        contact.normal * ((std::max(contact.penetration - slop, 0_d) / invMassSum) * percent);
    // move A back along -normal, B forward along +normal
    A.setPosition(A.getPosition() - correction * invMassA);
    B.setPosition(B.getPosition() + correction * invMassB);
}

void reboundCollision(Object& A, Object& B, Contact& contact, decimal restitution)
{
    decimal invMassA = 1_d / A.getMass();
    decimal invMassB = 1_d / B.getMass();

    // 1) Positional correction
    positionCorrection(A, B, contact);

    // 2) Relative velocity at contact (linear only)
    Vector3D va     = A.getVelocity();
    Vector3D vb     = B.getVelocity();
    Vector3D relVel = vb - va;

    decimal velAlongNormal = relVel.dotProduct(contact.normal);
    // Objects separating? do nothing
    if (velAlongNormal > 0._d)
        return;

    // 3) Compute impulse scalar j
    decimal e          = std::clamp(restitution, 0._d, 1._d);
    decimal invMassSum = invMassA + invMassB;
    if (invMassSum <= 0._d)
        return; // both immovable

    decimal j = -(1._d + e) * velAlongNormal / invMassSum;

    // 4) Apply impulse
    Vector3D impulse = contact.normal * j;
    A.setVelocity(va - impulse * invMassA);
    B.setVelocity(vb + impulse * invMassB);
}
