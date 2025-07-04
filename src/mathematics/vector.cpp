#include "mathematics/vector.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>

// ============================================================================
// ============================================================================
//  Utilities
// ============================================================================
// ============================================================================
void Vector3D::absolute()
{
    v[0] = std::fabs(v[0]);
    v[1] = std::fabs(v[1]);
    v[2] = std::fabs(v[2]);
}
void Vector3D::normalize()
{
    decimal n = getNorm();
    if (n < PRECISION_MACHINE)
        *this = Vector3D(0, 0, 0);
    else
        *this /= n;
}
Vector3D Vector3D::getAbsoluteVector() const
{
    Vector3D absV = Vector3D((*this));
    absV.absolute();
    return absV;
}
Vector3D Vector3D::getNormalized() const
{
    Vector3D normalizedV = Vector3D((*this));
    normalizedV.normalize();
    return normalizedV;
}
Vector3D min(const Vector3D& a, const Vector3D& b)
{
    return Vector3D { std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]) };
}
Vector3D max(const Vector3D& a, const Vector3D& b)
{
    return Vector3D { std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]) };
}

// ============================================================================
// ============================================================================
//  Setters
// ============================================================================
// ============================================================================
void Vector3D::setX(decimal x_) { v[0] = x_; }
void Vector3D::setY(decimal y_) { v[1] = y_; }
void Vector3D::setZ(decimal z_) { v[2] = z_; }
void Vector3D::setToZero() { v = { 0, 0, 0 }; }
void Vector3D::setAllValues(decimal s) { v = { s, s, s }; }
void Vector3D::setAllValues(decimal x_, decimal y_, decimal z_) { v = { x_, y_, z_ }; }

// ============================================================================
// ============================================================================
//  Property Checks
// ============================================================================
// ============================================================================
bool Vector3D::isZero() const { return commonMaths::approxEqual(getNormSquare(), decimal(0)); }
bool Vector3D::isLengthEqual(decimal val) const { return commonMaths::approxEqual(getNormSquare(), val); }
bool Vector3D::isFinite() const { return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]); }
bool Vector3D::isNormalized() const { return commonMaths::approxEqual(getNorm(), decimal(1)); }

// ============================================================================
// ============================================================================
//  Vector Operations
// ============================================================================
// ============================================================================
decimal Vector3D::dotProduct(const Vector3D& other) const
{
    return v[0] * other[0] + v[1] * other[1] + v[2] * other[2];
}
decimal  dotProduct(const Vector3D& lhs, const Vector3D& rhs) { return lhs.dotProduct(rhs); }
Vector3D Vector3D::crossProduct(const Vector3D& other) const
{
    return Vector3D { v[1] * other[2] - v[2] * other[1], v[2] * other[0] - v[0] * other[2],
                      v[0] * other[1] - v[1] * other[0] };
}
Vector3D crossProduct(const Vector3D& lhs, const Vector3D& rhs) { return lhs.crossProduct(rhs); }

// ============================================================================
// ============================================================================
//  Comparison Operators
// ============================================================================
// ============================================================================
bool Vector3D::operator==(const Vector3D& other) const
{
    return commonMaths::approxEqual(v[0], other[0]) && commonMaths::approxEqual(v[1], other[1]) &&
           commonMaths::approxEqual(v[2], other[2]);
}
bool Vector3D::operator!=(const Vector3D& other) const { return !(*this == other); }
bool Vector3D::operator<(const Vector3D& other) const
{
    if (v[0] != other[0])
        return v[0] < other[0];
    if (v[1] != other[1])
        return v[1] < other[1];
    return v[2] < other[2];
}
bool Vector3D::operator<=(const Vector3D& other) const
{
    if (v[0] != other[0])
        return v[0] < other[0];
    if (v[1] != other[1])
        return v[1] < other[1];
    return v[2] <= other[2];
}
bool Vector3D::operator>(const Vector3D& other) const
{
    if (v[0] != other[0])
        return v[0] > other[0];
    if (v[1] != other[1])
        return v[1] > other[1];
    return v[2] > other[2];
}
bool Vector3D::operator>=(const Vector3D& other) const
{
    if (v[0] != other[0])
        return v[0] > other[0];
    if (v[1] != other[1])
        return v[1] > other[1];
    return v[2] >= other[2];
}
bool Vector3D::approxEqual(const Vector3D& other, decimal p) const
{
    return commonMaths::approxEqual(v[0], other[0], p) && commonMaths::approxEqual(v[1], other[1], p) &&
           commonMaths::approxEqual(v[2], other[2], p);
}

// ============================================================================
// ============================================================================
//  Element Acess Operators
// ============================================================================
// ============================================================================
decimal& Vector3D::operator()(int i)
{
    if (i < 0 || i >= 3)
        throw std::out_of_range("Vector3D index out of range");
    return v[i];
}
decimal Vector3D::operator()(int i) const
{
    if (i < 0 || i >= 3)
        throw std::out_of_range("Vector3D index out of range");
    return v[i];
}
decimal& Vector3D::operator[](int i) { return v[i]; }
decimal Vector3D::operator[](int i) const { return v[i]; }

// ============================================================================
// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
// ============================================================================
Vector3D& Vector3D::operator-()
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
    return *this;
}
Vector3D& Vector3D::operator+=(const Vector3D& other)
{
    v[0] += other[0];
    v[1] += other[1];
    v[2] += other[2];
    return *this;
}
Vector3D& Vector3D::operator-=(const Vector3D& other)
{
    v[0] -= other[0];
    v[1] -= other[1];
    v[2] -= other[2];
    return *this;
}
Vector3D& Vector3D::operator*=(const Vector3D& other)
{
    v[0] *= other[0];
    v[1] *= other[1];
    v[2] *= other[2];
    return *this;
}
Vector3D& Vector3D::operator/=(const Vector3D& other)
{
    if (other[0] == 0 || other[1] == 0 || other[2] == 0)
        throw std::invalid_argument("Division by zero");
    v[0] /= other[0];
    v[1] /= other[1];
    v[2] /= other[2];
    return *this;
}
Vector3D& Vector3D::operator+=(decimal s)
{
    v[0] += s;
    v[1] += s;
    v[2] += s;
    return *this;
}
Vector3D& Vector3D::operator-=(decimal s)
{
    v[0] -= s;
    v[1] -= s;
    v[2] -= s;
    return *this;
}
Vector3D& Vector3D::operator*=(decimal s)
{
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
    return *this;
}
Vector3D& Vector3D::operator/=(decimal s)
{
    if (s == 0)
        throw std::invalid_argument("Division by zero");
    return *this *= decimal(1) / s;
}

// ============================================================================
// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ============================================================================
// Vector3D op Vector3D
Vector3D operator+(const Vector3D& lhs, const Vector3D& rhs)
{
    return applyVector(lhs, rhs, std::plus<decimal>());
}
Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs)
{
    return applyVector(lhs, rhs, std::minus<decimal>());
}
Vector3D operator*(const Vector3D& lhs, const Vector3D& rhs)
{
    return applyVector(lhs, rhs, std::multiplies<decimal>());
}
Vector3D operator/(const Vector3D& lhs, const Vector3D& rhs)
{
    if (rhs[0] == 0 || rhs[1] == 0 || rhs[2] == 0)
        throw std::invalid_argument("Division by zero");
    return applyVector(lhs, rhs, std::divides<decimal>());
}
// Vector3D op decimal
Vector3D operator+(const Vector3D& lhs, decimal rhs) { return applyVector(lhs, rhs, std::plus<decimal>()); }
Vector3D operator-(const Vector3D& lhs, decimal rhs) { return applyVector(lhs, rhs, std::minus<decimal>()); }
Vector3D operator*(const Vector3D& lhs, decimal rhs)
{
    return applyVector(lhs, rhs, std::multiplies<decimal>());
}
Vector3D operator/(const Vector3D& lhs, decimal rhs)
{
    if (rhs == 0)
        throw std::invalid_argument("Division by zero");
    return applyVector(lhs, rhs, std::divides<decimal>());
}
// Decimal op Vector3D
Vector3D operator+(decimal lhs, const Vector3D& rhs) { return rhs + lhs; }
Vector3D operator-(decimal lhs, const Vector3D& rhs) { return rhs - lhs; }
Vector3D operator*(decimal lhs, const Vector3D& rhs) { return rhs * lhs; }
Vector3D operator/(decimal lhs, const Vector3D& rhs) { return rhs / lhs; }

// ============================================================================
// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Vector3D& v)
{
    return os << "(" << v[0] << "," << v[1] << "," << v[2] << ")";
}
