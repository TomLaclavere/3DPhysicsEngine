#include "world/integrateRK4.hpp"

Derivative evaluate(const Object& obj, const Derivative& d, decimal dt)
{
    Object tmp = obj; // copy object state

    tmp.setPosition(obj.getPosition() + d.derivativeX * dt);
    tmp.setVelocity(obj.getVelocity() + d.derivativeV * dt);

    Derivative out;
    out.derivativeX = tmp.getVelocity();
    out.derivativeV = tmp.getAcceleration();
    return out;
}
