/**
 * @file object.cpp
 * @brief Implementation of Object class methods.
 *
 * Implements all member functions of the Object class declared in object.hpp,.
 *
 * This file includes:
 *  - Integration of motion equations.
 *
 * @see object.hpp
 */
#include "objects/object.hpp"

#include "mathematics/matrix.hpp"
#include "mathematics/vector.hpp"

#include <fstream>
#include <iomanip>

//  Constructors / Destructors
Object::Object(decimal mass)
    : mass { mass } {};
Object::Object(const Vector3D& position)
    : position { position }
{}
Object::Object(const Vector3D& position, const Vector3D& size)
    : position { position }
    , size { size }
{}
Object::Object(const Vector3D& position, const Vector3D& size, decimal mass)
    : position { position }
    , size { size }
    , mass { mass }
{
    checkFixed();
}
Object::Object(const Vector3D& position, const Vector3D& size, const Vector3D& velocity, decimal mass)
    : position { position }
    , size { size }
    , velocity { velocity }
    , mass { mass }
{
    checkFixed();
}
Object::Object(const Vector3D& position, const Vector3D& rotation, const Vector3D& size,
               const Vector3D& velocity, const Vector3D& acceleration, const Vector3D& force,
               const Vector3D& torque, decimal mass)
    : position { position }
    , rotation { rotation }
    , size { size }
    , velocity { velocity }
    , acceleration { acceleration }
    , force { force }
    , torque { torque }
    , mass { mass }
{
    checkFixed();
}

//  Getters
Vector3D   Object::getPosition() const { return position; }
Vector3D   Object::getRotation() const { return rotation; }
Vector3D   Object::getSize() const { return size; }
Vector3D   Object::getVelocity() const { return velocity; }
Vector3D   Object::getAcceleration() const { return acceleration; }
Vector3D   Object::getForce() const { return force; }
Vector3D   Object::getTorque() const { return torque; }
decimal    Object::getMass() const { return mass; }
decimal    Object::getStiffnessCst() const { return stiffnessCst; }
decimal    Object::getRestitutionCst() const { return restitutionCst; }
decimal    Object::getFrictionCst() const { return frictionCst; }
Material   Object::getMaterial() const { return material; }
ObjectType Object::getType() const { return ObjectType::Generic; }
bool       Object::getIsFixed() const { return fixed; }

//  Setters
void Object::setPosition(const Vector3D& _position) { position = _position; }
void Object::setRotation(const Vector3D& _rotation) { rotation = _rotation; }
void Object::setSize(const Vector3D& _size) { size = _size; }
void Object::setVelocity(const Vector3D& _velocity) { velocity = _velocity; }
void Object::setAcceleration(const Vector3D& _acceleration) { acceleration = _acceleration; }
void Object::setForce(const Vector3D& _force) { force = _force; }
void Object::setTorque(const Vector3D& _torque) { torque = _torque; }
void Object::setMass(const decimal _mass)
{
    mass = _mass;
    checkFixed();
}
void Object::setStiffnessCst(decimal k) { stiffnessCst = k; }
void Object::setRestitutionCst(decimal e) { restitutionCst = e; }
void Object::setFrictionCst(decimal mu) { frictionCst = mu; }
void Object::setMaterial(const Material& mat) { material = mat; }
void Object::setIsFixed(bool b)
{
    fixed = b;
    if (fixed)
        mass = 0_d;
}

//  Physics
void Object::checkFixed()
{
    if (mass <= 0_d)
        fixed = true;
    else
        fixed = false;
}
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

//  Utilities
void Object::initMotionCSV(std::ofstream& file)
{
    file << std::fixed << std::setprecision(6);
    file << "time,pos(x),pos(y),pos(z),vel(x),vel(y),vel(z),acc(x),acc(y),acc(z)\n";
}
bool Object::saveObjectCSV(std::ofstream& file)
{
    if (!file)
    {
        std::cerr << "Cannot open output file\n";
        return false;
    }
    Vector3D size = getSize();
    Vector3D pos  = getPosition();
    Vector3D rota = getRotation();
    file << getId() << "," << getName() << "," << getType() << "," << getMass() << "," << pos.getX() << ","
         << pos.getY() << "," << pos.getZ() << "," << size.getX() << "," << size.getY() << "," << size.getZ()
         << "," << rota.getX() << "," << rota.getY() << "," << rota.getZ() << "," << getIsFixed() << "\n";

    return file.good();
}
bool Object::saveMotionCSV(std::ofstream& file, decimal time)
{
    if (!file)
    {
        std::cerr << "Cannot open output file\n";
        return false;
    }

    const Vector3D& pos = getPosition();
    const Vector3D& vel = getVelocity();
    const Vector3D& acc = getAcceleration();

    file << time << "," << pos.getX() << "," << pos.getY() << "," << pos.getZ() << "," << vel.getX() << ","
         << vel.getY() << "," << vel.getZ() << "," << acc.getX() << "," << acc.getY() << "," << acc.getZ()
         << "\n";

    return file.good();
}
