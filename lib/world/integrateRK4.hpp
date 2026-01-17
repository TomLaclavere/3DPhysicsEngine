/**
 * @file integrateRK4.hpp
 *
 * @brief Utilities for Runge-Kutta 4 integration
 *
 */
#pragma once
#include "mathematics/vector.hpp"

struct Derivative
{
    Vector3D derivativeX; // d(position)/dit = velicity
    Vector3D derivativeV; // d(vitesse)/dt = acceleration
};
