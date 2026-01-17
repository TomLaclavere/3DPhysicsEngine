/**
 * @file quaternion.hpp
 * @brief Quaternion class and operations for physics/mathematical computations.
 *
 * Provides a quaternion representation (real + imaginary part), along with
 * constructors, utilities, algebraic operations, and conversion helpers.
 *
 * Part of the math foundation of the physics engine.
 *
 * Conventions:
 * - Quaternion q = (x*i, y*j, z*k, w).
 * - Imaginary part: `Vector3D` (x, y, z).
 * - Real part: `decimal` (w).
 */

#pragma once

#include "matrix.hpp"
#include "precision.hpp"
#include "vector.hpp"

#include <array>
#include <cmath>

/**
 * @defgroup QuaternionMaths
 * Utilities for quaternions. Contains Quaternion class and related free functions.
 */

/**
 * @ingroup QuaternionMaths
 * @brief Quaternion representation with real and imaginary parts.
 *
 * Use `decimal` type defined in precision.hpp.
 * Stored internally as `Vector3D` for imaginary part and `decimal` for real part.
 *
 * Example usage:
 * @code
 * Quaternion q1(0, 0, 0, 1);     // Identity
 * Quaternion q2(Vector3D(1,0,0), 0.5);
 * q1.normalise();
 * Quaternion q3 = q1 * q2;
 * @endcode
 */
struct Quaternion3D
{
private:
    Vector3D v { Vector3D() }; ///< Imaginary part (x, y, z).
    decimal  w { 0 };          ///< Real part (w).

public:
    // ============================================================================
    /// @name Constructors
    // ============================================================================
    /// @{
    constexpr Quaternion3D() = default;
    constexpr Quaternion3D(decimal _x, decimal _y, decimal _z, decimal _w) noexcept
        : v { Vector3D(_x, _y, _z) }
        , w { _w }
    {}
    constexpr Quaternion3D(const Vector3D& _v, decimal _w) noexcept
        : v { _v }
        , w { _w }
    {}
    constexpr Quaternion3D(decimal _w, const Vector3D& _v) noexcept
        : Quaternion3D(_v, _w)
    {}
    /// Constructor from rotation matrix.
    explicit Quaternion3D(const Matrix3x3& m);
    /// Constructor from Euler angles.
    constexpr Quaternion3D(decimal angleX, decimal angleY, decimal angleZ) noexcept
    {
        std::array<decimal, 4> quaternionElements = eulerAngles_to_Quaternion(angleX, angleY, angleZ);

        w = quaternionElements[0];
        v = Vector3D(quaternionElements[1], quaternionElements[2], quaternionElements[3]);
    }
    /// Constructor from Euler angles.
    constexpr explicit Quaternion3D(const Vector3D& eulerAngles) noexcept
        : Quaternion3D(eulerAngles[0], eulerAngles[1], eulerAngles[2])
    {}
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    constexpr decimal  getRealPart() const noexcept { return w; }
    constexpr Vector3D getImaginaryPart() const noexcept { return v; }
    constexpr decimal  getImaginaryPartElement(std::size_t index) const noexcept { return v[index]; }

    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// In-place conjugate (negate imaginary part).
    constexpr void conjugate() noexcept { v = -v; }
    /// In-place normalise. If zero-length quaternion, becomes null quaternion.
    void normalise();
    /// In-place inverse (conjugate then normalise).
    void inverse();
    /// Squared Euclidean norm. Cheaper than `getNorm()`.
    constexpr decimal getNormSquare() const noexcept { return w * w + v.getNormSquare(); }
    /// Euclidean norm.
    decimal getNorm() const;
    /// Return identity quaternion (0, 0, 0, 1).
    constexpr static Quaternion3D getIdentity() noexcept { return Quaternion3D(0, 0, 0, 1); };
    /// Return null quaternion (0, 0, 0, 0).
    constexpr static Quaternion3D getNull() noexcept { return Quaternion3D(0, 0, 0, 0); };
    /// Return a conjugated copy of the quaternion.
    constexpr Quaternion3D getConjugate() const noexcept
    {
        Quaternion3D q = *this;
        q.conjugate();
        return q;
    }
    /// Return a normalised copy of the quaternion.
    Quaternion3D getNormalise() const;
    /// Return an inverted copy of the quaternion.
    Quaternion3D getInverse() const;
    Matrix3x3    getRotationMatrix() const
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
    /// Compute quaternion from the three Euler angles. Used to implement constructors.
    constexpr std::array<decimal, 4> eulerAngles_to_Quaternion(decimal angleX, decimal angleY,
                                                               decimal angleZ) noexcept
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
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    constexpr void setRealPart(decimal value) noexcept { w = value; }
    constexpr void setImaginaryPart(decimal newx, decimal newy, decimal newz) noexcept
    {
        v = Vector3D(newx, newy, newz);
    }
    constexpr void setImaginaryPart(const Vector3D& newv) noexcept { v = newv; }
    constexpr void setToNull() noexcept
    {
        w = 0;
        v.setToNull();
    }
    constexpr void setToIdentity() noexcept
    {
        w    = 1;
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
    }
    constexpr void setAllValues(decimal newx, decimal newy, decimal newz, decimal neww) noexcept
    {
        w    = neww;
        v[0] = newx;
        v[1] = newy;
        v[2] = newz;
    }
    constexpr void setAllValues(const Vector3D& newv, decimal neww) noexcept
    {
        w = neww;
        v = newv;
    }
    constexpr void setAllValues(decimal neww, const Vector3D& newv) noexcept
    {
        w = neww;
        v = newv;
    }
    /// Set all values from rotation matrix.
    constexpr void setAllValues(const Matrix3x3& m) noexcept { *this = Quaternion3D(m); }
    /// Set all values from the three Euler angles.
    constexpr void setAllValues(decimal newangleX, decimal newangleY, decimal newangleZ) noexcept
    {
        std::array<decimal, 4> quaternionElements =
            eulerAngles_to_Quaternion(newangleX, newangleY, newangleZ);

        w    = quaternionElements[0];
        v[0] = quaternionElements[1];
        v[1] = quaternionElements[2];
        v[2] = quaternionElements[3];
    }
    /// Set all values from the three Euler angles.
    constexpr void setAllValues(const Vector3D& newangles) noexcept
    {
        setAllValues(newangles[0], newangles[1], newangles[2]);
    }
    /// @}

    // ============================================================================
    /// @name Property Checks
    // ============================================================================
    /// @{
    constexpr bool isFinite() const noexcept { return (commonMaths::isFinite(w) && v.isFinite()); }
    constexpr bool isZero() const noexcept { return (commonMaths::approxEqual(w, decimal(0)) && v.isNull()); }
    bool           isUnit() const;
    constexpr bool isIdentity() const noexcept
    {
        return (commonMaths::approxEqual(w, decimal(1)) && v.isNull());
    }
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isNormalised() const;
    /// @}

    // ============================================================================
    /// @name Quaternion Operations
    // ============================================================================
    /// @{
    constexpr decimal dotProduct(const Quaternion3D& other) const noexcept
    {
        return w * other.w + v.dotProduct(other.v);
    }
    constexpr Quaternion3D crossProduct(const Quaternion3D& other) const noexcept
    {
        return Quaternion3D(w * other.v + other.w * v + v.crossProduct(other.v),
                            w * other.w - v.dotProduct(other.v));
    }
    /// @}

    /// Element-wise comparisons.
    // ============================================================================
    /// @name Comparisons Operators
    // ============================================================================
    /// @{
    constexpr bool operator==(const Quaternion3D& other) const noexcept
    {
        return commonMaths::approxEqual(w, other.w) && v == other.v;
    }
    constexpr bool operator!=(const Quaternion3D& other) const noexcept
    {
        return !commonMaths::approxEqual(w, other.w) || v != other.v;
    }
    constexpr bool approxEqual(const Quaternion3D& other, decimal d) const noexcept
    {
        return (commonMaths::approxEqual(w, other.w, d) && v.approxEqual(other.v, d));
    }
    /// @}

    /// Imaginary part element access operator. Rely on `Vector3D` operators.
    // ============================================================================
    /// @name Element Access Operators
    // ============================================================================
    /// @{

    /// Access quaternion element with index range checking.
    decimal& at(std::size_t);
    /// Access quaternion element with index range checking (const version).
    decimal at(std::size_t) const;
    /// Access quaternion element without index range checking.
    decimal& operator[](std::size_t i) { return v[i]; }
    /// Access quaternion element without index range checking (const version).
    decimal operator[](std::size_t i) const { return v[i]; }
    /// @}

    /// Element-wise arithmetic operators (in-place).
    // ============================================================================
    /// @name In-Place Arithmetic Operators
    // ============================================================================
    /// @{

    /// Negate each element of the quaternion.
    constexpr Quaternion3D  operator-() noexcept { return Quaternion3D(-v, -w); }
    constexpr Quaternion3D& operator+=(const Quaternion3D& other) noexcept
    {
        w += other.w;
        v += other.v;
        return (*this);
    }
    constexpr Quaternion3D& operator-=(const Quaternion3D& other) noexcept
    {
        w -= other.w;
        v -= other.v;
        return (*this);
    }
    constexpr Quaternion3D& operator*=(const Quaternion3D& other) noexcept
    {
        w *= other.w;
        v *= other.v;
        return (*this);
    }
    /// Element-wise division by another quaternion. Throw `std::invalid_argument` on division by zero.
    Quaternion3D&           operator/=(const Quaternion3D&);
    constexpr Quaternion3D& operator+=(decimal scalar) noexcept
    {
        w += scalar;
        v += scalar;
        return (*this);
    }
    constexpr Quaternion3D& operator-=(decimal scalar) noexcept
    {
        w -= scalar;
        v -= scalar;
        return (*this);
    }
    constexpr Quaternion3D& operator*=(decimal scalar) noexcept
    {
        w *= scalar;
        v *= scalar;
        return (*this);
    }
    /// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
    Quaternion3D& operator/=(decimal);
    /// @}

    /// Internal functions to compute element-wise operations.
    // ============================================================================
    /// @name Internal Utilities
    // ============================================================================
    /// @{

    /// Apply a binary operation element-wise between two quaternions.
    template <class F>
    constexpr static Quaternion3D apply(const Quaternion3D& A, const Quaternion3D& B, F&& func) noexcept
    {
        return Quaternion3D(func(A.w, B.w), applyVector(A.v, B.v, func));
    }
    /// Apply a binary operation element-wise between a quaternion and a decimal.
    template <class F>
    constexpr static Quaternion3D apply(const Quaternion3D& A, decimal s, F&& func) noexcept
    {
        return Quaternion3D(func(A.w, s), applyVector(A.v, s, func));
    }
    /// Apply a binary operation element-wise between a decimal and a quaternion.
    template <class F>
    constexpr static Quaternion3D apply(decimal s, const Quaternion3D& A, F&& func) noexcept
    {
        return Quaternion3D(func(s, A.w), applyVector(s, A.v, func));
    }
    /// @}
};

// ============================================================================
/// @addtogroup QuaternionMaths
/// @{

// ============================================================================
/// @name Quaternion operations
// ============================================================================
/// @{
constexpr decimal dotProduct(const Quaternion3D& lhs, const Quaternion3D& rhs) noexcept
{
    return lhs.getRealPart() * rhs.getRealPart() + dotProduct(lhs.getImaginaryPart(), rhs.getImaginaryPart());
}
constexpr Quaternion3D crossProduct(const Quaternion3D& lhs, const Quaternion3D& rhs) noexcept
{
    decimal real =
        lhs.getRealPart() * rhs.getRealPart() - lhs.getImaginaryPart().dotProduct(rhs.getImaginaryPart());
    Vector3D img = lhs.getRealPart() * rhs.getImaginaryPart() + rhs.getRealPart() * lhs.getImaginaryPart() +
                   lhs.getImaginaryPart().crossProduct(rhs.getImaginaryPart());
    return Quaternion3D(img, real);
}
/// @}

/// Element-wise arithmetic operations.
// ============================================================================
/// @name Operators
// ============================================================================
/// @{
constexpr Quaternion3D operator+(const Quaternion3D& lhs, const Quaternion3D& rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::plus<decimal>());
}
constexpr Quaternion3D operator-(const Quaternion3D& lhs, const Quaternion3D& rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::minus<decimal>());
}
constexpr Quaternion3D operator*(const Quaternion3D& lhs, const Quaternion3D& rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::multiplies<decimal>());
}
/// Element-wise division by another quaternion. Throw `std::invalid_argument` on division by zero.
Quaternion3D operator/(const Quaternion3D&, const Quaternion3D&);

constexpr Quaternion3D operator+(const Quaternion3D& lhs, decimal rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::plus<decimal>());
}
constexpr Quaternion3D operator-(const Quaternion3D& lhs, decimal rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::minus<decimal>());
}
constexpr Quaternion3D operator*(const Quaternion3D& lhs, decimal rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::multiplies<decimal>());
}
/// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
Quaternion3D operator/(const Quaternion3D&, decimal);

constexpr Quaternion3D operator+(decimal lhs, const Quaternion3D& rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::plus<decimal>());
}
constexpr Quaternion3D operator-(decimal lhs, const Quaternion3D& rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::minus<decimal>());
}
constexpr Quaternion3D operator*(decimal lhs, const Quaternion3D& rhs) noexcept
{
    return Quaternion3D::apply(lhs, rhs, std::multiplies<decimal>());
}
/// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
Quaternion3D operator/(decimal, const Quaternion3D&);
/// @}

// ============================================================================
/// @name Printing
// ============================================================================
/// @{

/**
 * Stream output operator for `Quaternion`.
 * Return format is (x,y,z,w).
 */
std::ostream& operator<<(std::ostream&, const Quaternion3D&);
/// @}

/// @}
