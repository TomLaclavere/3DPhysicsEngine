#include "mathematics/vector.hpp"

#include <cmath>
#include <functional>
#include <stdexcept>

// ===== Utilities =====
Vector3D Vector3D::getAbsoluteVector() const
{
    return Vector3D { std::fabs(v[0]), std::fabs(v[1]), std::fabs(v[2]) };
}
Vector3D Vector3D::getNormalizedVector() const
{
    decimal n = getNorm();
    return (n < PRECISION_MACHINE) ? Vector3D { 0, 0, 0 } : *this / n;
}
void Vector3D::normalize()
{
    decimal n2 = getNormSquare();
    if (n2 < PRECISION_MACHINE)
        return;
    decimal inv = 1.0L / std::sqrt(n2);
    v[0] *= inv;
    v[1] *= inv;
    v[2] *= inv;
}
Vector3D Vector3D::min(const Vector3D& a, const Vector3D& b)
{
    return Vector3D { std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]) };
}
Vector3D Vector3D::max(const Vector3D& a, const Vector3D& b)
{
    return Vector3D { std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]) };
}

// ===== Setters =====
void Vector3D::setX(decimal x_) { v[0] = x_; }
void Vector3D::setY(decimal y_) { v[1] = y_; }
void Vector3D::setZ(decimal z_) { v[2] = z_; }
void Vector3D::setToZero() { v = { 0, 0, 0 }; }
void Vector3D::setAllValues(decimal s) { v = { s, s, s }; }
void Vector3D::setAllValues(decimal x_, decimal y_, decimal z_) { v = { x_, y_, z_ }; }

// ===== Property Checks =====
bool Vector3D::isZero() const { return commonMaths::approxEqual(getNormSquare(), decimal(0)); }
bool Vector3D::isUnit() const { return commonMaths::approxEqual(getNormSquare(), decimal(1)); }
bool Vector3D::isLengthEqual(decimal val) const { return commonMaths::approxEqual(getNormSquare(), val); }
bool Vector3D::isFinite() const { return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]); }

// ===== Vector Operations =====
decimal Vector3D::dotProduct(const Vector3D& other) const
{
    return v[0] * other[0] + v[1] * other[1] + v[2] * other[2];
}
Vector3D Vector3D::crossProduct(const Vector3D& other) const
{
    return Vector3D { v[1] * other[2] - v[2] * other[1], v[2] * other[0] - v[0] * other[2],
                      v[0] * other[1] - v[1] * other[0] };
}

// ===== Comparison Operators =====
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

// ===== Element Access =====
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

// ===== In-Place Arithmetic Operators =====
Vector3D Vector3D::operator-() const { return Vector3D(-v[0], -v[1], -v[2]); }
Vector3D& Vector3D::operator+=(const Vector3D& other)
{
    v[0] += other[0];
    v[1] += other[1];
    v[2] += other[2];
    return *this;
}
Vector3D& Vector3D::operator-=(const Vector3D& other) { return *this += -other; }
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

// ===== Helper for Free Arithmetic Operators =====
template <class F>
Vector3D Vector3D::apply(const Vector3D& A, const Vector3D& B, F&& f)
{
    return Vector3D { f(A.v[0], B.v[0]), f(A.v[1], B.v[1]), f(A.v[2], B.v[2]) };
}
template <class F>
Vector3D Vector3D::apply(const Vector3D& A, decimal s, F&& f)
{
    return Vector3D { f(A.v[0], s), f(A.v[1], s), f(A.v[2], s) };
}

// ===== Free Arithmetic Operators =====
// Vector3D op Vector3D
Vector3D operator+(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D::apply(lhs, rhs, std::plus<decimal>());
}
Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D::apply(lhs, rhs, std::minus<decimal>());
}
Vector3D operator*(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D::apply(lhs, rhs, std::multiplies<decimal>());
}
Vector3D operator/(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D::apply(lhs, rhs, std::divides<decimal>());
}
// Vector3D op decimal
Vector3D operator+(const Vector3D& lhs, decimal rhs)
{
    return Vector3D::apply(lhs, rhs, std::plus<decimal>());
}
Vector3D operator-(const Vector3D& lhs, decimal rhs)
{
    return Vector3D::apply(lhs, rhs, std::minus<decimal>());
}
Vector3D operator*(const Vector3D& lhs, decimal rhs)
{
    return Vector3D::apply(lhs, rhs, std::multiplies<decimal>());
}
Vector3D operator/(const Vector3D& lhs, decimal rhs)
{
    return Vector3D::apply(lhs, rhs, std::divides<decimal>());
}
// decimal op Vector3D
Vector3D operator+(decimal lhs, const Vector3D& rhs) { return rhs + lhs; }
Vector3D operator-(decimal lhs, const Vector3D& rhs)
{
    return Vector3D(lhs - rhs.getX(), lhs - rhs.getY(), lhs - rhs.getZ());
}
Vector3D operator*(decimal lhs, const Vector3D& rhs) { return rhs * lhs; }
Vector3D operator/(decimal lhs, const Vector3D& rhs)
{
    return Vector3D(lhs / rhs.getX(), lhs / rhs.getY(), lhs / rhs.getZ());
}

// ===== Printing =====
std::ostream& operator<<(std::ostream& os, const Vector3D& v)
{
    return os << "(" << v[0] << "," << v[1] << "," << v[2] << ")";
}
