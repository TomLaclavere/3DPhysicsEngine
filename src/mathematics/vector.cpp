#include "mathematics/vector.hpp"

#include "mathematics/common.hpp"

#include <cmath>
#include <stdexcept>

Vector3D::Vector3D()
    : x(0.0)
    , y(0.0)
    , z(0.0)
{}
Vector3D::Vector3D(decimal new_x, decimal new_y, decimal new_z)
    : x(new_x)
    , y(new_y)
    , z(new_z)
{}

decimal  Vector3D::getX() const { return x; }
decimal  Vector3D::getY() const { return y; }
decimal  Vector3D::getZ() const { return z; }
decimal  Vector3D::getNorm() const { return sqrt(x * x + y * y + z * z); }
decimal  Vector3D::getNormSquare() const { return x * x + y * y + z * z; }
decimal  Vector3D::getMinValue() const { return std::min(x, std::min(y, z)); }
decimal  Vector3D::getMaxValue() const { return std::max(x, std::max(y, z)); }
Vector3D Vector3D::getAbsoluteVector() const { return Vector3D(std::abs(x), std::abs(y), std::abs(z)); }
Vector3D Vector3D::getNormalizedVector() const { return *this / getNorm(); }

void Vector3D::setX(decimal val) { x = val; }
void Vector3D::setY(decimal val) { y = val; }
void Vector3D::setZ(decimal val) { z = val; }
void Vector3D::setToZero() { x = y = z = 0.0; }
void Vector3D::setAllValues(decimal newx, decimal newy, decimal newz)
{
    x = newx;
    y = newy;
    z = newz;
}
void Vector3D::normalize()
{
    decimal norm = getNorm();
    if (norm < PRECISION_MACHINE)
        return;
    x /= norm;
    y /= norm;
    z /= norm;
}

bool Vector3D::isZero() { return commonMaths::approxEqual(Vector3D::getNormSquare(), decimal(0.0)); }
bool Vector3D::isUnit() { return commonMaths::approxEqual(Vector3D::getNormSquare(), decimal(1.0)); }
bool Vector3D::isLengthEqual(decimal value)
{
    return commonMaths::approxEqual(Vector3D::getNormSquare(), value);
}
bool Vector3D::isFinite() { return (std::isfinite(x) && std::isfinite(y) && std::isfinite(z)); }

decimal  Vector3D::dotProduct(const Vector3D& v) const { return x * v.x + y * v.y + z * v.z; }
Vector3D Vector3D::crossProduct(const Vector3D& v) const
{
    return Vector3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

bool Vector3D::operator==(const Vector3D& vector) const
{
    return (x == vector.x && y == vector.y && z == vector.z);
}
bool Vector3D::operator!=(const Vector3D& vector) const { return !(*this == vector); }
bool Vector3D::operator<(const Vector3D& vector) const
{
    return (x == vector.x ? (y == vector.y ? z < vector.z : y < vector.y) : x < vector.x);
}
bool Vector3D::approxEqual(const Vector3D& vector, decimal precision) const
{
    return (commonMaths::approxEqual(x, vector.x, precision) &&
            commonMaths::approxEqual(y, vector.y, precision) &&
            commonMaths::approxEqual(z, vector.z, precision));
}

decimal& Vector3D::operator[](int index)
{
    switch (index)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        throw std::invalid_argument("Index must be between 0 and 2 for three dimensional vector.");
    }
}
decimal Vector3D::operator[](int index) const
{
    switch (index)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        throw std::invalid_argument("Index must be between 0 and 2 for three dimensional vector.");
    }
}

Vector3D& Vector3D::operator+=(const Vector3D& vector)
{
    x += vector.x;
    y += vector.y;
    z += vector.z;
    return *this;
}
Vector3D& Vector3D::operator-=(const Vector3D& vector)
{
    x -= vector.x;
    y -= vector.y;
    z -= vector.z;
    return *this;
}
Vector3D& Vector3D::operator*=(const Vector3D& vector)
{
    x *= vector.x;
    y *= vector.y;
    z *= vector.z;
    return *this;
}
Vector3D& Vector3D::operator/=(const Vector3D& vector)
{
    x /= vector.x;
    y /= vector.y;
    z /= vector.z;
    return *this;
}

Vector3D Vector3D::min(const Vector3D& vector1, const Vector3D& vector2) const
{
    return Vector3D(std::min(vector1.x, vector2.x), std::min(vector1.y, vector2.y),
                    std::min(vector1.z, vector2.z));
}
Vector3D Vector3D::max(const Vector3D& vector1, const Vector3D& vector2) const
{
    return Vector3D(std::max(vector1.x, vector2.x), std::max(vector1.y, vector2.y),
                    std::max(vector1.z, vector2.z));
}

Vector3D operator+(const decimal& value, const Vector3D& vector)
{
    return Vector3D(vector.x + value, vector.y + value, vector.z + value);
}
Vector3D operator+(const Vector3D& vector, const decimal& value) { return value + vector; }
Vector3D operator+(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}
Vector3D operator-(const Vector3D& vector) { return Vector3D(-vector.x, -vector.y, -vector.z); }
Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}
Vector3D operator*(const decimal& value, const Vector3D& vector)
{
    return Vector3D(vector.x * value, vector.y * value, vector.z * value);
}
Vector3D operator*(const Vector3D& vector, const decimal& value) { return value + vector; }
Vector3D operator*(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}
Vector3D operator/(const Vector3D& vector, const decimal& value)
{
    return Vector3D(vector.x / value, vector.y / value, vector.z / value);
}
Vector3D operator/(const Vector3D& lhs, const Vector3D& rhs)
{
    return Vector3D(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}
