#include "mathematics/quaternion.hpp"

#include <array>
#include <cmath>

// ============================================================================
//  Constructors
// ============================================================================
Quaternion::Quaternion(const Matrix3x3& m)
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
Quaternion::Quaternion(decimal angleX, decimal angleY, decimal angleZ)
{
    std::array<decimal, 4> quaternionElements = eulerAngles_to_Quaternion(angleX, angleY, angleZ);

    w = quaternionElements[0];
    v = Vector3D(quaternionElements[1], quaternionElements[2], quaternionElements[3]);
}
Quaternion::Quaternion(const Vector3D& eulerAngles)
{
    Quaternion(eulerAngles[0], eulerAngles[1], eulerAngles[2]);
}

// ============================================================================
//  Getters
// ============================================================================
Matrix3x3 Quaternion::getRotationMatrix() const
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
void Quaternion::conjugate() { v = -v; }
void Quaternion::normalize()
{
    decimal norm = getNorm();
    if (norm < PRECISION_MACHINE)
        *this = Quaternion();
    else
    {
        w /= norm;
        v /= norm;
    }
}
void Quaternion::inverse()
{
    (*this).conjugate();
    (*this).normalize();
}
decimal    Quaternion::getNormSquare() const { return w * w + v.getNormSquare(); }
decimal    Quaternion::getNorm() const { return std::sqrt(getNormSquare()); }
Quaternion Quaternion::getIdentity() const { return Quaternion(0, 0, 0, 1); }
Quaternion Quaternion::getNull() const { return Quaternion(); }
Quaternion Quaternion::getConjugate() const
{
    Quaternion q = *this;
    q.conjugate();
    return q;
}
Quaternion Quaternion::getNormalize() const
{
    Quaternion q = *this;
    q.normalize();
    return q;
}
Quaternion Quaternion::getInverse() const
{
    Quaternion q = *this;
    q.inverse();
    return q;
}

// ============================================================================
//  Setters
// ============================================================================
void Quaternion::setRealPart(decimal value) { w = value; }
void Quaternion::setImaginaryPart(decimal newx, decimal newy, decimal newz)
{
    v = Vector3D(newx, newy, newz);
}
void Quaternion::setImaginaryPart(const Vector3D& newv) { v = newv; }
void Quaternion::setToZero()
{
    w = 0;
    v.setToZero();
}
void Quaternion::setToIdentity()
{
    w    = 1;
    v[0] = 0;
    v[1] = 0;
    v[2] = 0;
}
void Quaternion::setAllValues(decimal newx, decimal newy, decimal newz, decimal neww)
{
    w    = neww;
    v[0] = newx;
    v[1] = newy;
    v[2] = newz;
}
void Quaternion::setAllValues(const Vector3D& newv, decimal neww)
{
    w = neww;
    v = newv;
}
void Quaternion::setAllValues(decimal neww, const Vector3D& newv)
{
    w = neww;
    v = newv;
}
void Quaternion::setAllValues(const Matrix3x3& m) { *this = Quaternion(m); }
void Quaternion::setAllValues(decimal newangleX, decimal newangleY, decimal newangleZ)
{
    std::array<decimal, 4> quaternionElements = eulerAngles_to_Quaternion(newangleX, newangleY, newangleZ);

    w    = quaternionElements[0];
    v[0] = quaternionElements[1];
    v[1] = quaternionElements[2];
    v[2] = quaternionElements[3];
}
void Quaternion::setAllValues(const Vector3D& newangles)
{
    setAllValues(newangles[0], newangles[1], newangles[2]);
}

// ============================================================================
//  Property Checks
// ============================================================================
bool Quaternion::isFinite() const { return (std::isfinite(w) && v.isFinite()); }
bool Quaternion::isZero() const { return (commonMaths::approxEqual(w, decimal(0)) && v.isNull()); }
bool Quaternion::isUnit() const { return (commonMaths::approxEqual(getNorm(), decimal(1))); }
bool Quaternion::isIdentity() const { return (commonMaths::approxEqual(w, decimal(1)) && v.isNull()); }
bool Quaternion::isInvertible() const { return !(commonMaths::approxEqual(getNorm(), decimal(0))); }
bool Quaternion::isOrthogonal() const { return isUnit(); }
bool Quaternion::isNormalized() const { return isUnit(); }

// ============================================================================
//  Quaternion Operations
// ============================================================================
decimal Quaternion::dotProduct(const Quaternion& other) const { return w * other.w + v.dotProduct(other.v); }
Quaternion Quaternion::crossProduct(const Quaternion& other) const
{
    return Quaternion(w * other.v + other.w * v + v.crossProduct(other.v),
                      w * other.w - v.dotProduct(other.v));
}

// ============================================================================
//  Comparisons Operators
// ============================================================================
bool Quaternion::operator==(const Quaternion& other) const
{
    return commonMaths::approxEqual(w, other.w) && v == other.v;
}
bool Quaternion::operator!=(const Quaternion& other) const
{
    return !commonMaths::approxEqual(w, other.w) || v != other.v;
}
bool Quaternion::operator<(const Quaternion& other) const { return (w < other.w && v < other.v); }
bool Quaternion::operator<=(const Quaternion& other) const { return (w <= other.w && v <= other.v); }
bool Quaternion::operator>(const Quaternion& other) const { return (w > other.w && v > other.v); }
bool Quaternion::operator>=(const Quaternion& other) const { return (w >= other.w && v >= other.v); }
bool Quaternion::approxEqual(const Quaternion& other, decimal p) const
{
    return (commonMaths::approxEqual(w, other.w, p) && v.approxEqual(other.v, p));
}

// ============================================================================
//  Element Access Operators
// ============================================================================
decimal& Quaternion::at(int i) { return v.at(i); }
decimal  Quaternion::at(int i) const { return v.at(i); }
decimal& Quaternion::operator()(int i) { return v(i); }
decimal  Quaternion::operator()(int i) const { return v(i); }
decimal& Quaternion::operator[](int i) { return v[i]; }
decimal  Quaternion::operator[](int i) const { return v[i]; }

// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
Quaternion& Quaternion::operator-()
{
    w = -w;
    v = -v;
    return (*this);
}
Quaternion& Quaternion::operator+=(const Quaternion& other)
{
    w += other.w;
    v += other.v;
    return (*this);
}
Quaternion& Quaternion::operator-=(const Quaternion& other)
{
    w -= other.w;
    v -= other.v;
    return (*this);
}
Quaternion& Quaternion::operator*=(const Quaternion& other)
{
    w *= other.w;
    v *= other.v;
    return (*this);
}
Quaternion& Quaternion::operator/=(const Quaternion& other)
{
    if (other.isZero())
        throw std::invalid_argument("Division by zero");
    w /= other.w;
    v /= other.v;
    return (*this);
}
Quaternion& Quaternion::operator+=(decimal scalar)
{
    w += scalar;
    v += scalar;
    return (*this);
}
Quaternion& Quaternion::operator-=(decimal scalar)
{
    w -= scalar;
    v -= scalar;
    return (*this);
}
Quaternion& Quaternion::operator*=(decimal scalar)
{
    w *= scalar;
    v *= scalar;
    return (*this);
}
Quaternion& Quaternion::operator/=(decimal scalar)
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
Quaternion Quaternion::apply(const Quaternion& A, const Quaternion& B, F&& func)
{
    return Quaternion(func(A.w, B.w), applyVector(A.v, B.v, func));
}
template <class F>
Quaternion Quaternion::apply(const Quaternion& A, decimal s, F&& func)
{
    return Quaternion(func(A.w, s), applyVector(A.v, s, func));
}
// ============================================================================
//  Quaternions Operations
// ============================================================================
decimal dotProduct(const Quaternion& lhs, const Quaternion& rhs)
{
    return lhs.getRealPart() * rhs.getRealPart() + dotProduct(lhs.getImaginaryPart(), rhs.getImaginaryPart());
}
Quaternion crossProduct(const Quaternion& lhs, const Quaternion& rhs)
{
    decimal real =
        lhs.getRealPart() * rhs.getRealPart() - lhs.getImaginaryPart().dotProduct(rhs.getImaginaryPart());
    Vector3D img = lhs.getRealPart() * rhs.getImaginaryPart() + rhs.getRealPart() * lhs.getImaginaryPart() +
                   lhs.getImaginaryPart().crossProduct(rhs.getImaginaryPart());
    return Quaternion(img, real);
}

// ============================================================================
//  Utilities
// ============================================================================
Quaternion             min(const Quaternion&, const Quaternion&);
Quaternion             max(const Quaternion&, const Quaternion&);
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
// Quaternion op Quaternion
Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion::apply(lhs, rhs, std::plus<decimal>());
}
Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion::apply(lhs, rhs, std::minus<decimal>());
}
Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion::apply(lhs, rhs, std::multiplies<decimal>());
}
Quaternion operator/(const Quaternion& lhs, const Quaternion& rhs)
{
    return Quaternion::apply(lhs, rhs, std::divides<decimal>());
}
// Quaternion op decimal
Quaternion operator+(const Quaternion& lhs, decimal rhs)
{
    return Quaternion::apply(lhs, rhs, std::plus<decimal>());
}
Quaternion operator-(const Quaternion& lhs, decimal rhs)
{
    return Quaternion::apply(lhs, rhs, std::minus<decimal>());
}
Quaternion operator*(const Quaternion& lhs, decimal rhs)
{
    return Quaternion::apply(lhs, rhs, std::multiplies<decimal>());
}
Quaternion operator/(const Quaternion& lhs, decimal rhs)
{
    return Quaternion::apply(lhs, rhs, std::divides<decimal>());
}
// decimal op Quaternion
Quaternion operator+(decimal lhs, const Quaternion& rhs) { return rhs + lhs; }
Quaternion operator-(decimal lhs, const Quaternion& rhs) { return rhs - lhs; }
Quaternion operator*(decimal lhs, const Quaternion& rhs) { return rhs * lhs; }
Quaternion operator/(decimal lhs, const Quaternion& rhs) { return rhs / lhs; }

// ============================================================================
//  Printing
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Quaternion& q)
{
    return os << "(" << q.getRealPart() << "," << q.getImaginaryPart() << ")";
}
