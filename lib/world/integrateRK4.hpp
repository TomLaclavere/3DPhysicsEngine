/**
 * @file integrateRK4.hpp
 *
 * @brief Utilities for Runge-Kutta 4 integration
 *
 */
#pragma once
#include "mathematics/vector.hpp"

/// One RK4 stage derivative: dx/dt = velocity, dv/dt = acceleration.
struct Derivative
{
    Vector3D derivativeX; /// d(position)/dt = velocity.
    Vector3D derivativeV; /// d(velocity)/dt = acceleration.
};
