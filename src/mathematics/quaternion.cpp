#include "mathematics/quaternion.hpp"

#include <array>
#include <cmath>

// ============================================================================
//  Constructors
// ============================================================================
Quaternion3D::Quaternion3D(const Matrix3x3& m)
{
    decimal trace = m.getTrace();
    if (trace > 0)
    {
        decimal s = std::sqrt(trace + 1.0) * 2;
        w         = 0.25 * s;
        decimal x = (m(2, 1) - m(1, 2)) / s;
        decimal y = (m(0, 2) - m(2, 0)) / s;
        decimal z = (m(1, 0) - m(0, 1)) / s;
        v         = Vector3D(x, y, z);
    }
    else if ((m(0, 0) > m(1, 1)) && (m(0, 0) > m(2, 2)))
    {
        decimal s = std::sqrt(1.0 + m(0, 0) - m(1, 1) - m(2, 2)) * 2;
        w         = (m(2, 1) - m(1, 2)) / s;
        decimal x = 0.25 * s;
        decimal y = (m(0, 1) + m(1, 0)) / s;
        decimal z = (m(0, 2) + m(2, 0)) / s;
        v         = Vector3D(x, y, z);
    }
    else if (m(1, 1) > m(2, 2))
    {
        decimal s = std::sqrt(1.0 + m(1, 1) - m(0, 0) - m(2, 2)) * 2;
        w         = (m(0, 2) - m(2, 0)) / s;
        decimal x = (m(0, 1) + m(1, 0)) / s;
        decimal y = 0.25 * s;
        decimal z = (m(1, 2) + m(2, 1)) / s;
        v         = Vector3D(x, y, z);
    }
    else
    {
        decimal s = std::sqrt(1.0 + m(2, 2) - m(0, 0) - m(1, 1)) * 2;
        w         = (m(1, 0) - m(0, 1)) / s;
        decimal x = (m(0, 2) + m(2, 0)) / s;
        decimal y = (m(1, 2) + m(2, 1)) / s;
        decimal z = 0.25 * s;
        v         = Vector3D(x, y, z);
    }
}
Quaternion3D::Quaternion3D(decimal angleX, decimal angleY, decimal angleZ)
{
    std::array<decimal, 4> quaternionElements = eulerAngles_to_Quaternion(angleX, angleY, angleZ);

    w = quaternionElements[0];
    v = Vector3D(quaternionElements[1], quaternionElements[2], quaternionElements[3]);
}
Quaternion3D::Quaternion3D(const Vector3D& eulerAngles)
{
    Quaternion3D(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
}

// ============================================================================
//  Getters
// ============================================================================
Matrix3x3 Quaternion3D::getRotationMatrix() const
{
    decimal x = v[0];
    decimal y = v[1];
    decimal z = v[2];

    decimal xx = x * x;
    decimal yy = y * y;
    decimal zz = z * z;
    decimal xy = x * y;
    decimal xz = x * z;
    decimal yz = y * z;
    decimal wx = w * x;
    decimal wy = w * y;
    decimal wz = w * z;

    return Matrix3x3(1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 2 * (xy + wz), 1 - 2 * (xx + zz),
                     2 * (yz - wx), 2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy));
}

// ============================================================================
//  Utilities
// ============================================================================
void Quaternion3D::conjugate() { v = -v; }
void Quaternion3D::normalize()
{
    decimal norm = getNorm();
    if (commonMaths::approxEqual(norm, decimal(0)))
        *this = Quaternion3D();
    else
    {
        w /= norm;
        v /= norm;
    }
}
void Quaternion3D::inverse()
{
    (*this).conjugate();
    (*this).normalize();
}
decimal      Quaternion3D::getNormSquare() const { return w * w + v.getNormSquare(); }
decimal      Quaternion3D::getNorm() const { return std::sqrt(getNormSquare()); }
Quaternion3D Quaternion3D::getConjugate() const
{
    Quaternion3D q = *this;
    q.conjugate();
    return q;
}
Quaternion3D Quaternion3D::getNormalize() const
{
    Quaternion3D q = *this;
    q.normalize();
    return q;
}
Quaternion3D Quaternion3D::getInverse() const
{
    Quaternion3D q = *this;
    q.inverse();
    return q;
}

// ============================================================================
//  Setters
// ============================================================================
void Quaternion3D::setRealPart(decimal value) { w = value; }
void Quaternion3D::setImaginaryPart(decimal newx, decimal newy, decimal newz)
{
    v = Vector3D(newx, newy, newz);
}
void Quaternion3D::setImaginaryPart(const Vector3D& newv) { v = newv; }
void Quaternion3D::setToZero()
{
    w = 0;
    v.setToZero();
}
void Quaternion3D::setToIdentity()
{
    w    = 1;
    v[0] = 0;
    v[1] = 0;
    v[2] = 0;
}
void Quaternion3D::setAllValues(decimal newx, decimal newy, decimal newz, decimal neww)
{
    w    = neww;
    v[0] = newx;
    v[1] = newy;
    v[2] = newz;
}
void Quaternion3D::setAllValues(const Vector3D& newv, decimal neww)
{
    w = neww;
    v = newv;
}
void Quaternion3D::setAllValues(decimal neww, const Vector3D& newv)
{
    w = neww;
    v = newv;
}
void Quaternion3D::setAllValues(const Matrix3x3& m) { *this = Quaternion3D(m); }
void Quaternion3D::setAllValues(decimal newangleX, decimal newangleY, decimal newangleZ)
{
    std::array<decimal, 4> quaternionElements = eulerAngles_to_Quaternion(newangleX, newangleY, newangleZ);

    w    = quaternionElements[0];
    v[0] = quaternionElements[1];
    v[1] = quaternionElements[2];
    v[2] = quaternionElements[3];
}
void Quaternion3D::setAllValues(const Vector3D& newangles)
{
    setAllValues(newangles[0], newangles[1], newangles[2]);
}

// ============================================================================
//  Property Checks
// ============================================================================
bool Quaternion3D::isFinite() const { return (std::isfinite(w) && v.isFinite()); }
bool Quaternion3D::isZero() const { return (commonMaths::approxEqual(w, decimal(0)) && v.isNull()); }
bool Quaternion3D::isUnit() const { return (commonMaths::approxEqual(getNorm(), decimal(1))); }
bool Quaternion3D::isIdentity() const { return (commonMaths::approxEqual(w, decimal(1)) && v.isNull()); }
bool Quaternion3D::isInvertible() const { return !(commonMaths::approxEqual(getNorm(), decimal(0))); }
bool Quaternion3D::isOrthogonal() const { return isUnit(); }
bool Quaternion3D::isNormalized() const { return isUnit(); }

// ============================================================================
//  Quaternion3D Operations
// ============================================================================
decimal Quaternion3D::dotProduct(const Quaternion3D& other) const
{
    return w * other.w + v.dotProduct(other.v);
}
Quaternion3D Quaternion3D::crossProduct(const Quaternion3D& other) const
{
    return Quaternion3D(w * other.v + other.w * v + v.crossProduct(other.v),
                        w * other.w - v.dotProduct(other.v));
}

// ============================================================================
//  Comparisons Operators
// ============================================================================
bool Quaternion3D::operator==(const Quaternion3D& other) const
{
    return commonMaths::approxEqual(w, other.w) && v == other.v;
}
bool Quaternion3D::operator!=(const Quaternion3D& other) const
{
    return !commonMaths::approxEqual(w, other.w) || v != other.v;
}
bool Quaternion3D::operator<(const Quaternion3D& other) const { return (w < other.w && v < other.v); }
bool Quaternion3D::operator<=(const Quaternion3D& other) const { return (w <= other.w && v <= other.v); }
bool Quaternion3D::operator>(const Quaternion3D& other) const { return (w > other.w && v > other.v); }
bool Quaternion3D::operator>=(const Quaternion3D& other) const { return (w >= other.w && v >= other.v); }
bool Quaternion3D::approxEqual(const Quaternion3D& other, decimal p) const
{
    return (commonMaths::approxEqual(w, other.w, p) && v.approxEqual(other.v, p));
}

// ============================================================================
//  Element Access Operators
// ============================================================================
decimal& Quaternion3D::at(int i) { return v.at(i); }
decimal  Quaternion3D::at(int i) const { return v.at(i); }
decimal& Quaternion3D::operator()(int i) { return v(i); }
decimal  Quaternion3D::operator()(int i) const { return v(i); }
decimal& Quaternion3D::operator[](int i) { return v[i]; }
decimal  Quaternion3D::operator[](int i) const { return v[i]; }

// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
Quaternion3D& Quaternion3D::operator-()
{
    w = -w;
    v = -v;
    return (*this);
}
Quaternion3D& Quaternion3D::operator+=(const Quaternion3D& other)
{
    w += other.w;
    v += other.v;
    return (*this);
}
Quaternion3D& Quaternion3D::operator-=(const Quaternion3D& other)
{
    w -= other.w;
    v -= other.v;
    return (*this);
}
Quaternion3D& Quaternion3D::operator*=(const Quaternion3D& other)
{
    w *= other.w;
    v *= other.v;
    return (*this);
}
Quaternion3D& Quaternion3D::operator/=(const Quaternion3D& other)
{
    // Check for division by zero
    for (int i = 0; i < 3; ++i)
        if (commonMaths::approxEqual(other.v[i], decimal(0)))
            throw std::invalid_argument("Division by zero");
    if (commonMaths::approxEqual(other.w, decimal(0)))
        throw std::invalid_argument("Division by zero");

    w /= other.w;
    v /= other.v;
    return (*this);
}
Quaternion3D& Quaternion3D::operator+=(decimal scalar)
{
    w += scalar;
    v += scalar;
    return (*this);
}
Quaternion3D& Quaternion3D::operator-=(decimal scalar)
{
    w -= scalar;
    v -= scalar;
    return (*this);
}
Quaternion3D& Quaternion3D::operator*=(decimal scalar)
{
    w *= scalar;
    v *= scalar;
    return (*this);
}
Quaternion3D& Quaternion3D::operator/=(decimal scalar)
{
    if (commonMaths::approxEqual(scalar, decimal(0)))
        throw std::invalid_argument("Division by zero");
    w /= scalar;
    v /= scalar;
    return (*this);
}

// ============================================================================
//  Helper for Free Arithmetic Operators
// ============================================================================
template <class F>
Quaternion3D Quaternion3D::apply(const Quaternion3D& A, const Quaternion3D& B, F&& func)
{
    return Quaternion3D(func(A.w, B.w), applyVector(A.v, B.v, func));
}
template <class F>
Quaternion3D Quaternion3D::apply(const Quaternion3D& A, decimal s, F&& func)
{
    return Quaternion3D(func(A.w, s), applyVector(A.v, s, func));
}
// ============================================================================
//  Quaternions Operations
// ============================================================================
decimal dotProduct(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    return lhs.getRealPart() * rhs.getRealPart() + dotProduct(lhs.getImaginaryPart(), rhs.getImaginaryPart());
}
Quaternion3D crossProduct(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    decimal real =
        lhs.getRealPart() * rhs.getRealPart() - lhs.getImaginaryPart().dotProduct(rhs.getImaginaryPart());
    Vector3D img = lhs.getRealPart() * rhs.getImaginaryPart() + rhs.getRealPart() * lhs.getImaginaryPart() +
                   lhs.getImaginaryPart().crossProduct(rhs.getImaginaryPart());
    return Quaternion3D(img, real);
}

// ============================================================================
//  Utilities
// ============================================================================
Quaternion3D           min(const Quaternion3D&, const Quaternion3D&);
Quaternion3D           max(const Quaternion3D&, const Quaternion3D&);
std::array<decimal, 4> eulerAngles_to_Quaternion(decimal angleX, decimal angleY, decimal angleZ)
{
    std::array<decimal, 4> quaternionElements;

    decimal cosX = std::cos(angleX * decimal(0.5));
    decimal sinX = std::sin(angleX * decimal(0.5));
    decimal cosY = std::cos(angleY * decimal(0.5));
    decimal sinY = std::sin(angleY * decimal(0.5));
    decimal cosZ = std::cos(angleZ * decimal(0.5));
    decimal sinZ = std::sin(angleZ * decimal(0.5));

    decimal w = cosX * cosY * cosZ + sinX * sinY * sinZ;
    decimal x = sinX * cosY * cosZ - cosX * sinY * sinZ;
    decimal y = cosX * sinY * cosZ + sinX * cosY * sinZ;
    decimal z = cosX * cosY * sinZ - sinX * sinY * sinZ;

    quaternionElements[0] = w;
    quaternionElements[1] = x;
    quaternionElements[2] = y;
    quaternionElements[3] = z;

    return quaternionElements;
}

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// Quaternion3D op Quaternion3D
Quaternion3D operator+(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    return Quaternion3D::apply(lhs, rhs, std::plus<decimal>());
}
Quaternion3D operator-(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    return Quaternion3D::apply(lhs, rhs, std::minus<decimal>());
}
Quaternion3D operator*(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    return Quaternion3D::apply(lhs, rhs, std::multiplies<decimal>());
}
Quaternion3D operator/(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    // Check for division by zero
    for (int i = 0; i < 3; ++i)
        if (commonMaths::approxEqual(rhs.getImaginaryPartElement(i), decimal(0)))
            throw std::invalid_argument("Division by zero");
    if (commonMaths::approxEqual(rhs.getRealPart(), decimal(0)))
        throw std::invalid_argument("Division by zero");
    return Quaternion3D::apply(lhs, rhs, std::divides<decimal>());
}
// Quaternion3D op decimal
Quaternion3D operator+(const Quaternion3D& lhs, decimal rhs)
{
    return Quaternion3D::apply(lhs, rhs, std::plus<decimal>());
}
Quaternion3D operator-(const Quaternion3D& lhs, decimal rhs)
{
    return Quaternion3D::apply(lhs, rhs, std::minus<decimal>());
}
Quaternion3D operator*(const Quaternion3D& lhs, decimal rhs)
{
    return Quaternion3D::apply(lhs, rhs, std::multiplies<decimal>());
}
Quaternion3D operator/(const Quaternion3D& lhs, decimal rhs)
{
    if (commonMaths::approxEqual(rhs, decimal(0)))
        throw std::invalid_argument("Division by zero");
    return Quaternion3D::apply(lhs, rhs, std::divides<decimal>());
}
// decimal op Quaternion3D
Quaternion3D operator+(decimal lhs, const Quaternion3D& rhs) { return rhs + lhs; }
Quaternion3D operator-(decimal lhs, const Quaternion3D& rhs) { return rhs - lhs; }
Quaternion3D operator*(decimal lhs, const Quaternion3D& rhs) { return rhs * lhs; }
Quaternion3D operator/(decimal lhs, const Quaternion3D& rhs)
{
    // Check for division by zero
    for (int i = 0; i < 3; ++i)
        if (commonMaths::approxEqual(rhs.getImaginaryPartElement(i), decimal(0)))
            throw std::invalid_argument("Division by zero");
    if (commonMaths::approxEqual(rhs.getRealPart(), decimal(0)))
        throw std::invalid_argument("Division by zero");
    return rhs / lhs;
}

// ============================================================================
//  Printing
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Quaternion3D& q)
{
    return os << "(" << q.getRealPart() << "," << q.getImaginaryPart() << ")";
}
