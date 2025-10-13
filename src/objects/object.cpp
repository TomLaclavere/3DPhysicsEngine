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

// ============================================================================
//  Getters
// ============================================================================
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
ObjectType Object::getType() const { return ObjectType::Generic; }
bool       Object::getIsFixed() const { return fixed; }

// ============================================================================
//  Setters
// ============================================================================
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

// ============================================================================
//  Apply forces / torques
// ============================================================================
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
