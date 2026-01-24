/**
 * @file vector.hpp
 * @brief 3D vector class and operations for physics/mathematical computations.
 *
 * Part of the math foundation of the physics engine.
 *
 * Conventions:
 *  - Vector is represented as (x, y, z).
 *  - Norm is Euclidean.
 *  - Cross product follows right-hand rule.
 */
#pragma once
#include "mathematics/common.hpp"
#include "precision.hpp"

#include <array>
#include <ostream>

/**
 * @defgroup VectorMaths
 * Utilities for vectorial operations. Contains Vector3D class and related free functions.
 */

/**
 * @ingroup VectorMaths
 * @brief 3D vector class with basic math operations.
 *
 * Uses `decimal` type defined in precision.hpp.
 * Stored internally as `std::array<decimal, 3>`.
 *
 * Example usage:
 * @code
 * Vector3D v1(1.0, 2.0, 3.0);
 * Vector3D v2(4.0, 5.0, 6.0);
 * decimal dot   = v1.dotProduct(v2);
 * Vector3D cross = v1.crossProduct(v2);
 * @endcode
 */
struct Vector3D
{
private:
    std::array<decimal, 3> v { 0, 0, 0 };

public:
    // ============================================================================
    /// @name Constructors
    // ============================================================================
    /// @{
    constexpr Vector3D() = default;
    constexpr explicit Vector3D(decimal value) noexcept
        : v { value, value, value }
    {}
    constexpr Vector3D(decimal x, decimal y, decimal z) noexcept
        : v { x, y, z }
    {}
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    constexpr decimal                getX() const noexcept { return v[0]; }
    constexpr decimal                getY() const noexcept { return v[1]; }
    constexpr decimal                getZ() const noexcept { return v[2]; }
    constexpr std::array<decimal, 3> getV() const noexcept { return v; }
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// Return max element of the vector
    constexpr decimal getMax() const noexcept
    {
        decimal maxVal = v[0];
        for (unsigned int i = 1; i < 3; i++)
        {
            if (v[i] > maxVal)
            {
                maxVal = v[i];
            }
        }
        return maxVal;
    }
    /// Return min element of the vector
    constexpr decimal getMin() const noexcept
    {
        decimal minVal = v[0];
        for (unsigned int i = 1; i < 3; i++)
        {
            if (v[i] < minVal)
            {
                minVal = v[i];
            }
        }
        return minVal;
    }
    /// Convert each element to its absolute value (in-place).
    constexpr void absolute() noexcept
    {
        v[0] = commonMaths::absVal(v[0]);
        v[1] = commonMaths::absVal(v[1]);
        v[2] = commonMaths::absVal(v[2]);
    }
    /// Normalise this vector (in-place). If zero-length, becomes null vector.
    void normalise();
    /// Squared Euclidian norm. Cheaper than `getNorm()`.
    constexpr decimal getNormSquare() const noexcept { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }
    /// Euclidean norm.
    decimal getNorm() const;
    /// Minimum element value.
    constexpr decimal getMinValue() const noexcept
    {
        return v[0] < v[1] ? (v[0] < v[2] ? v[0] : v[2]) : (v[1] < v[2] ? v[1] : v[2]);
    }
    /// Maximum element value.
    constexpr decimal getMaxValue() const noexcept
    {
        return v[0] > v[1] ? (v[0] > v[2] ? v[0] : v[2]) : (v[1] > v[2] ? v[1] : v[2]);
    }
    /// Return a new vector with element-wise absolute values.
    constexpr Vector3D getAbsolute() const noexcept
    {
        Vector3D absV = Vector3D((*this));
        absV.absolute();
        return absV;
    }
    /// Return a normalised copy of the vector. If zero-length, return null vector.
    Vector3D getNormalised() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    constexpr void setX(decimal _x) noexcept { v[0] = _x; }
    constexpr void setY(decimal _y) noexcept { v[1] = _y; }
    constexpr void setZ(decimal _z) noexcept { v[2] = _z; }
    constexpr void setToNull() noexcept { v = { 0, 0, 0 }; }
    constexpr void setAllValues(decimal d) noexcept { v = { d, d, d }; }
    constexpr void setAllValues(decimal _x, decimal _y, decimal _z) noexcept { v = { _x, _y, _z }; }
    /// @}

    // ============================================================================
    /// @name Properties checks
    // ============================================================================
    /// @{
    constexpr bool isNull() const noexcept { return v[0] == 0 && v[1] == 0 && v[2] == 0; }
    /// Check if vector length equals a given value.
    constexpr bool isLengthEqual(decimal length) const noexcept
    {
        return commonMaths::approxEqual(getNormSquare(), length);
    }
    /// Check if all elements are finite (not NaN, not Inf).
    constexpr bool isFinite() const noexcept
    {
        return commonMaths::isFinite(v[0]) && commonMaths::isFinite(v[1]) && commonMaths::isFinite(v[2]);
    }
    /// Check if vector has unit length.
    constexpr bool isNormalised() const noexcept { return commonMaths::approxEqual(getNorm(), decimal(1)); }
    /// @}

    // ============================================================================
    /// @name Vector Operations
    // ============================================================================
    /// @{

    /// Dot product: a · b.
    constexpr decimal dotProduct(const Vector3D& other) const noexcept
    {
        return v[0] * other[0] + v[1] * other[1] + v[2] * other[2];
    }

    /// Cross product: a × b (right-hand rule).
    constexpr Vector3D crossProduct(const Vector3D& other) const noexcept
    {
        return Vector3D { v[1] * other[2] - v[2] * other[1], v[2] * other[0] - v[0] * other[2],
                          v[0] * other[1] - v[1] * other[0] };
    }
    /// @}

    /// Element-wise comparisons.
    // ============================================================================
    /// @name Operators Comparison
    // ============================================================================
    /// @{
    constexpr bool operator==(const Vector3D& other) const noexcept
    {
        return commonMaths::approxEqual(v[0], other[0]) && commonMaths::approxEqual(v[1], other[1]) &&
               commonMaths::approxEqual(v[2], other[2]);
    }
    constexpr bool operator!=(const Vector3D& other) const noexcept { return !(*this == other); }
    constexpr bool approxEqual(const Vector3D& other, decimal p) const noexcept
    {
        return commonMaths::approxEqual(v[0], other[0], p) && commonMaths::approxEqual(v[1], other[1], p) &&
               commonMaths::approxEqual(v[2], other[2], p);
    }

    /// @}

    // ============================================================================
    /// @name Element Access Operators
    // ============================================================================
    /// @{

    /// Access vector element with index range checking.
    decimal& at(std::size_t);
    /// Access vector element with index range checking (const version).
    decimal at(std::size_t) const;
    /// Access vector element without index range checking.
    constexpr decimal& operator[](std::size_t i) noexcept { return v[i]; }
    /// Access vector element without index range checking (const version).
    constexpr decimal operator[](std::size_t i) const noexcept { return v[i]; }
    /// @}

    /// Element-wise arithmetic operators (in-place).
    // ============================================================================
    /// @name Operators Arithmetic In-Place
    // ============================================================================
    /// @{

    /// Negate each element of the vector.
    constexpr Vector3D  operator-() const noexcept { return Vector3D { -v[0], -v[1], -v[2] }; }
    constexpr Vector3D& operator+=(const Vector3D& other) noexcept
    {
        v[0] += other[0];
        v[1] += other[1];
        v[2] += other[2];
        return *this;
    }
    constexpr Vector3D& operator-=(const Vector3D& other) noexcept
    {
        v[0] -= other[0];
        v[1] -= other[1];
        v[2] -= other[2];
        return *this;
    }
    constexpr Vector3D& operator*=(const Vector3D& other) noexcept
    {
        v[0] *= other[0];
        v[1] *= other[1];
        v[2] *= other[2];
        return *this;
    }
    /// Element-wise division by another vector. Throw `std::invalid_argument` on division by zero.
    Vector3D&           operator/=(const Vector3D& other);
    constexpr Vector3D& operator+=(decimal d) noexcept
    {
        v[0] += d;
        v[1] += d;
        v[2] += d;
        return *this;
    }
    constexpr Vector3D& operator-=(decimal d)
    {
        v[0] -= d;
        v[1] -= d;
        v[2] -= d;
        return *this;
    }
    constexpr Vector3D& operator*=(decimal d) noexcept
    {
        v[0] *= d;
        v[1] *= d;
        v[2] *= d;
        return *this;
    }
    /// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
    Vector3D& operator/=(decimal);
    /// @}
};
// ============================================================================
//  Free Functions
// ============================================================================

/// @addtogroup VectorMaths
/// @{

/// Euclidian vectorial operations.
// ============================================================================
/// @name Vector Operations
// ============================================================================
/// @{

decimal  dotProduct(const Vector3D&, const Vector3D&);
Vector3D crossProduct(const Vector3D&, const Vector3D&);
/// @}

/// Internal functions to compute element-wise operations.
/// @name Internal Utilities
/// @{

/// Apply a binary operation element-wise between two vectors.
template <class F>
inline Vector3D applyVector(const Vector3D& A, const Vector3D& B, F&& f)
{
    return Vector3D { f(A[0], B[0]), f(A[1], B[1]), f(A[2], B[2]) };
}

/// Apply a binary operation element-wise between a vector and a scalar.
template <class F>
inline Vector3D applyVector(const Vector3D& A, decimal s, F&& f)
{
    return Vector3D { f(A[0], s), f(A[1], s), f(A[2], s) };
}

/// Apply a binary operation element-wise between a scalar and a vector.
template <class F>
inline Vector3D applyVector(decimal s, const Vector3D& A, F&& f)
{
    return Vector3D { f(s, A[0]), f(s, A[1]), f(s, A[2]) };
}
/// @}

/// Element-wise arithmetic operations.
// ============================================================================
/// @name Operators
// ============================================================================
/// @{

constexpr Vector3D operator+(const Vector3D& lhs, const Vector3D& rhs) noexcept
{
    return applyVector(lhs, rhs, std::plus<decimal>());
}
constexpr Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs) noexcept
{
    return applyVector(lhs, rhs, std::minus<decimal>());
}
constexpr Vector3D operator*(const Vector3D& lhs, const Vector3D& rhs) noexcept
{
    return applyVector(lhs, rhs, std::multiplies<decimal>());
}
/// Element-wise division between two vectors. Throw `std::invalid_argument` on division by zero.
Vector3D operator/(const Vector3D&, const Vector3D&);

constexpr Vector3D operator+(const Vector3D& lhs, decimal rhs) noexcept
{
    return applyVector(lhs, rhs, std::plus<decimal>());
}
constexpr Vector3D operator-(const Vector3D& lhs, decimal rhs) noexcept
{
    return applyVector(lhs, rhs, std::minus<decimal>());
}
constexpr Vector3D operator*(const Vector3D& lhs, decimal rhs) noexcept
{
    return applyVector(lhs, rhs, std::multiplies<decimal>());
}
/// Element-wise division between a vector and a decimal. Throw `std::invalid_argument` on division by zero.
Vector3D operator/(const Vector3D&, decimal);

constexpr Vector3D operator+(decimal lhs, const Vector3D& rhs) noexcept
{
    return applyVector(lhs, rhs, std::plus<decimal>());
}
constexpr Vector3D operator-(decimal lhs, const Vector3D& rhs) noexcept
{
    return applyVector(lhs, rhs, std::minus<decimal>());
}
constexpr Vector3D operator*(decimal lhs, const Vector3D& rhs) noexcept
{
    return applyVector(lhs, rhs, std::multiplies<decimal>());
}
/// Element-wise division between a decimal and a vector. Throw `std::invalid_argument` on division by zero.
Vector3D operator/(decimal, const Vector3D&);
/// @}

// ============================================================================
/// @name Printing
// ============================================================================
/// @{

/**
 * Stream output operator for `Vector3D`.
 * Return format is (x,y,z).
 */
std::ostream& operator<<(std::ostream&, const Vector3D&);
/// @}
/// @}
/// @}
