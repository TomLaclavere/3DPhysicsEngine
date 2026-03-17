#include "collision/collision_response.hpp"

#include <algorithm>

void positionCorrection(Object& A, Object& B, Contact& contact, decimal percent, decimal slop)
{
    if (contact.penetration <= slop)
        return;

    auto    invMassA   = A.getMass() > 0_d ? 1_d / A.getMass() : 0_d;
    auto    invMassB   = B.getMass() > 0_d ? 1_d / B.getMass() : 0_d;
    decimal invMassSum = invMassA + invMassB;
    if (invMassSum <= 0_d)
        return;

    Vector3D n = contact.normal;
    // if ((A.getPosition() - B.getPosition()).dotProduct(n) < 0_d)
    //     n = -n;

    decimal  correctionMag = (std::max(contact.penetration - slop, 0_d) / invMassSum) * percent;
    Vector3D correction    = n * correctionMag;

    A.setPosition(A.getPosition() + correction * invMassA);
    B.setPosition(B.getPosition() - correction * invMassB);
}

void reboundCollision(Object& A, Object& B, Contact& contact)
{
    decimal invMassA   = A.getMass() > 0_d ? 1_d / A.getMass() : 0_d;
    decimal invMassB   = B.getMass() > 0_d ? 1_d / B.getMass() : 0_d;
    decimal invMassSum = invMassA + invMassB;

    if (invMassSum <= 0_d)
        return;

    Vector3D n = contact.normal;
    // if ((A.getPosition() - B.getPosition()).dotProduct(n) < 0_d)
    //     n = -n;

    positionCorrection(A, B, contact);

    Vector3D va = A.getVelocity();
    Vector3D vb = B.getVelocity();

    Vector3D relVel         = va - vb;
    decimal  velAlongNormal = relVel.dotProduct(n);
    if (velAlongNormal >= 0_d)
        return;

    decimal e =
        std::clamp(std::min(A.getMaterial().getRestitution(), B.getMaterial().getRestitution()), 0_d, 1_d);
    decimal j = -(1_d + e) * velAlongNormal / invMassSum;

    Vector3D impulse = n * j;

    A.setVelocity(va + impulse * invMassA);
    B.setVelocity(vb - impulse * invMassB);
}
