/**
 * @file integrateRK4.hpp
 *
 * @brief Utilities for Runge-Kutta 4 integration
 *
 */
#pragma once
#include "mathematics/vector.hpp"
#include "objects/object.hpp"

struct Derivative
{
    Vector3D derivativeX;
    Vector3D derivativeV;
};

Derivative evaluate(const Object& obj, const Derivative& d, decimal dt);
