/**
 * @file vector.cpp
 * @brief Implementation of Vector3D class methods and related free functions.
 *
 * Implements all member functions of the Vector3D class declared in vector.hpp,
 * as well as free arithmetic operators and utility functions.
 *
 * This file includes:
 *  - Vector3D utilities (absolute value, normalization, norms)
 *  - Setters and property checks
 *  - Vector operations (dot/cross product)
 *  - Comparison and arithmetic operators
 *  - Free arithmetic operators for Vector3D and decimal
 *  - Stream output operator
 *
 * Exception safety:
 *  - Out-of-range access for `operator()` throws `std::out_of_range`.
 *  - Division by zero in in-place or free arithmetic operators throws `std::invalid_argument`.
 *
 * @see vector.hpp
 */

#include "mathematics/vector.hpp"

#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>

// ============================================================================
//  Utilities
// ============================================================================
void Vector3D::normalise()
{
    decimal n = getNorm();
    if (n < PRECISION_MACHINE)
        *this = Vector3D(0, 0, 0);
    else
        *this /= n;
}
decimal  Vector3D::getNorm() const { return std::sqrt(getNormSquare()); }
Vector3D Vector3D::getNormalised() const
{
    Vector3D normalisedV = Vector3D((*this));
    normalisedV.normalise();
    return normalisedV;
}

// ============================================================================
//  Vector Operations
// ============================================================================
decimal  dotProduct(const Vector3D& lhs, const Vector3D& rhs) { return lhs.dotProduct(rhs); }
Vector3D crossProduct(const Vector3D& lhs, const Vector3D& rhs) { return lhs.crossProduct(rhs); }

// ============================================================================
//  Element Access Operators
// ============================================================================
/// @param i Index of the component (0, 1, or 2).
/// @return Reference to the component at index `i`.
decimal& Vector3D::at(std::size_t i)
{
    if (i >= 3)
        throw std::out_of_range("Vector3D index out of range");
    return v[i];
}
/// @param i Index of the component (0, 1, or 2).
/// @return Value of the component at index `i`.
decimal Vector3D::at(std::size_t i) const
{
    if (i >= 3)
        throw std::out_of_range("Vector3D index out of range");
    return v[i];
}

// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
Vector3D& Vector3D::operator/=(const Vector3D& other)
{
    if (commonMaths::approxEqual(other[0], decimal(0)) || commonMaths::approxEqual(other[1], decimal(0)) ||
        commonMaths::approxEqual(other[2], decimal(0)))
        throw std::invalid_argument("Division by zero");
    v[0] /= other[0];
    v[1] /= other[1];
    v[2] /= other[2];
    return *this;
}
Vector3D& Vector3D::operator/=(decimal s)
{
    if (commonMaths::approxEqual(s, decimal(0)))
        throw std::invalid_argument("Division by zero");
    return *this *= decimal(1) / s;
}

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// Vector3D op Vector3D
Vector3D operator/(const Vector3D& lhs, const Vector3D& rhs)
{
    if (commonMaths::approxEqual(rhs[0], decimal(0)) || commonMaths::approxEqual(rhs[1], decimal(0)) ||
        commonMaths::approxEqual(rhs[2], decimal(0)))
        throw std::invalid_argument("Division by zero");
    return applyVector(lhs, rhs, std::divides<decimal>());
}

// Vector3D op decimal
Vector3D operator/(const Vector3D& lhs, decimal rhs)
{
    if (commonMaths::approxEqual(rhs, decimal(0)))
        throw std::invalid_argument("Division by zero");
    return applyVector(lhs, rhs, std::divides<decimal>());
}

// Decimal op Vector3D
// Vector3D op decimal
Vector3D operator/(decimal lhs, const Vector3D& rhs)
{
    if (commonMaths::approxEqual(rhs[0], decimal(0)) || commonMaths::approxEqual(rhs[1], decimal(0)) ||
        commonMaths::approxEqual(rhs[2], decimal(0)))
        throw std::invalid_argument("Division by zero");
    return applyVector(lhs, rhs, std::divides<decimal>());
}

// ============================================================================
// Printing
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Vector3D& v)
{
    std::ios oldState(nullptr);
    oldState.copyfmt(os);

    os << std::scientific << std::setprecision(3) << "(" << v[0] << " , " << v[1] << " , " << v[2] << ")";

    os.copyfmt(oldState);
    return os;
}
