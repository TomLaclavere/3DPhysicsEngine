/**
 * @file quaternion.cpp
 * @brief Implementation of Quaternion3D class methods and free functions.
 */
#include "mathematics/quaternion.hpp"

// ============================================================================
//  Constructors
// ============================================================================
/**
 * @brief Construct a quaternion from a rotation matrix using Shepperd's method.
 *
 * Branches on the largest diagonal element to maximise numerical stability
 * (avoids near-zero square roots).
 *
 * @param m A 3×3 rotation matrix.
 */
Quaternion3D::Quaternion3D(const Matrix3x3& m)
{
    decimal trace = m.getTrace();

    if (trace > 0_d)
    {
        decimal s = std::sqrt(trace + 1_d) * 2_d;
        w         = 0.25_d * s;
        v         = Vector3D((m(2, 1) - m(1, 2)) / s, (m(0, 2) - m(2, 0)) / s, (m(1, 0) - m(0, 1)) / s);
    }
    else
    {
        // Find the index of the largest diagonal element
        std::size_t i = 0;
        if (m(1, 1) > m(0, 0))
            i = 1;
        if (m(2, 2) > m(i, i))
            i = 2;

        decimal  s;
        Vector3D qv;

        switch (i)
        {
        case 0: // m00 largest
            s     = std::sqrt(1_d + m(0, 0) - m(1, 1) - m(2, 2)) * 2_d;
            qv[0] = 0.25_d * s;
            qv[1] = (m(0, 1) + m(1, 0)) / s;
            qv[2] = (m(0, 2) + m(2, 0)) / s;
            w     = (m(2, 1) - m(1, 2)) / s;
            break;

        case 1: // m11 largest
            s     = std::sqrt(1_d + m(1, 1) - m(0, 0) - m(2, 2)) * 2_d;
            qv[0] = (m(0, 1) + m(1, 0)) / s;
            qv[1] = 0.25_d * s;
            qv[2] = (m(1, 2) + m(2, 1)) / s;
            w     = (m(0, 2) - m(2, 0)) / s;
            break;

        default: // m22 largest
            s     = std::sqrt(1_d + m(2, 2) - m(0, 0) - m(1, 1)) * 2_d;
            qv[0] = (m(0, 2) + m(2, 0)) / s;
            qv[1] = (m(1, 2) + m(2, 1)) / s;
            qv[2] = 0.25_d * s;
            w     = (m(1, 0) - m(0, 1)) / s;
            break;
        }

        v = qv;
    }
}

// ============================================================================
//  Utilities
// ============================================================================
/// @brief Normalise in-place. If norm ~ 0, resets to the null quaternion (0, 0, 0, 0).
void Quaternion3D::normalise()

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
/// @brief Invert in-place: negate the imaginary part (conjugate), then normalise.
void Quaternion3D::inverse()
{
    (*this).conjugate();
    (*this).normalise();
}
decimal      Quaternion3D::getNorm() const { return std::sqrt(getNormSquare()); }
Quaternion3D Quaternion3D::getNormalise() const
{
    Quaternion3D q = *this;
    q.normalise();
    return q;
}
Quaternion3D Quaternion3D::getInverse() const
{
    Quaternion3D q = *this;
    q.inverse();
    return q;
}

// ============================================================================
//  Properties Check
// ============================================================================
bool Quaternion3D::isUnit() const { return (commonMaths::approxEqual(getNorm(), decimal(1))); }
bool Quaternion3D::isInvertible() const { return !(commonMaths::approxEqual(getNorm(), decimal(0))); }
bool Quaternion3D::isOrthogonal() const { return isUnit(); }
bool Quaternion3D::isNormalised() const { return isUnit(); }

// ============================================================================
//  Element Access Operators
// ============================================================================
decimal& Quaternion3D::at(std::size_t i) { return v.at(i); }
decimal  Quaternion3D::at(std::size_t i) const { return v.at(i); }

// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
/**
 * @brief Element-wise division by another quaternion.
 * This is NOT a quaternion algebraic inverse; each component (w, x, y, z) is divided independently.
 * @throws std::invalid_argument if any component of `other` is zero.
 */
Quaternion3D& Quaternion3D::operator/=(const Quaternion3D& other)
{
    // Check for division by zero
    for (std::size_t i = 0; i < 3; ++i)
        if (commonMaths::approxEqual(other.v[i], decimal(0)))
            throw std::invalid_argument("Division by zero");
    if (commonMaths::approxEqual(other.w, decimal(0)))
        throw std::invalid_argument("Division by zero");

    w /= other.w;
    v /= other.v;
    return (*this);
}

/// @throws std::invalid_argument if scalar ~ 0.
Quaternion3D& Quaternion3D::operator/=(decimal scalar)
{
    if (commonMaths::approxEqual(scalar, decimal(0)))
        throw std::invalid_argument("Division by zero");
    w /= scalar;
    v /= scalar;
    return (*this);
}

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// Quaternion3D op Quaternion3D (element-wise)
/// @throws std::invalid_argument if any component of rhs is zero.
Quaternion3D operator/(const Quaternion3D& lhs, const Quaternion3D& rhs)
{
    // Check for division by zero
    for (std::size_t i = 0; i < 3; ++i)
        if (commonMaths::approxEqual(rhs.getImaginaryPartElement(i), decimal(0)))
            throw std::invalid_argument("Division by zero");
    if (commonMaths::approxEqual(rhs.getRealPart(), decimal(0)))
        throw std::invalid_argument("Division by zero");
    return Quaternion3D::apply(lhs, rhs, std::divides<decimal>());
}
// Quaternion3D op decimal
/// @throws std::invalid_argument if rhs ≈ 0.
Quaternion3D operator/(const Quaternion3D& lhs, decimal rhs)
{
    if (commonMaths::approxEqual(rhs, decimal(0)))
        throw std::invalid_argument("Division by zero");
    return Quaternion3D::apply(lhs, rhs, std::divides<decimal>());
}
// decimal op Quaternion3D (element-wise lhs / rhs[i])
/// @throws std::invalid_argument if any component of rhs is zero.
Quaternion3D operator/(decimal lhs, const Quaternion3D& rhs)
{
    // Check for division by zero
    for (std::size_t i = 0; i < 3; ++i)
        if (commonMaths::approxEqual(rhs.getImaginaryPartElement(i), decimal(0)))
            throw std::invalid_argument("Division by zero");
    if (commonMaths::approxEqual(rhs.getRealPart(), decimal(0)))
        throw std::invalid_argument("Division by zero");
    return Quaternion3D::apply(lhs, rhs, std::divides<decimal>());
}

// ============================================================================
//  Printing
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Quaternion3D& q)
{
    return os << "(" << q.getRealPart() << "," << q.getImaginaryPart() << ")";
}
