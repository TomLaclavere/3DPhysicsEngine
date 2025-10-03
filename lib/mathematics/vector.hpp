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
#include "common.hpp"
#include "config.hpp"
#include "precision.hpp"

#include <algorithm>
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
    Vector3D() = default;
    explicit Vector3D(decimal value)
        : v { value, value, value }
    {}
    Vector3D(decimal x, decimal y, decimal z)
        : v { x, y, z }
    {}
    Vector3D(const Vector3D& newv)
        : v { newv.v }
    {}
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    decimal                getX() const { return v[0]; }
    decimal                getY() const { return v[1]; }
    decimal                getZ() const { return v[2]; }
    std::array<decimal, 3> getV() const { return v; }
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// Convert each element to its absolute value (in-place).
    void absolute();
    /// Normalize this vector (in-place). If zero-length, becomes null vector.
    void normalize();
    /// Squared Euclidian norm. Cheaper than `getNorm()`.
    decimal getNormSquare() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }
    /// Euclidean norm.
    decimal getNorm() const { return std::sqrt(getNormSquare()); }
    /// Minimum element value.
    decimal getMinValue() const { return std::min({ v[0], v[1], v[2] }); }
    /// Maximum element value.
    decimal getMaxValue() const { return std::max({ v[0], v[1], v[2] }); }
    /// Return a new vector with element-wise absolute values.
    Vector3D getAbsolute() const;
    /// Return a normalized copy of the vector. If zero-length, return null vector.
    Vector3D getNormalized() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setX(decimal);
    void setY(decimal);
    void setZ(decimal);
    void setToZero();
    void setAllValues(decimal);
    void setAllValues(decimal, decimal, decimal);
    /// @}

    // ============================================================================
    /// @name Properties checks
    // ============================================================================
    /// @{
    bool isNull() const;
    /// Check if vector length equals a given value.
    bool isLengthEqual(decimal length) const;
    /// Check if all elements are finite (not NaN, not Inf).
    bool isFinite() const;
    /// Check if vector has unit length.
    bool isNormalized() const;
    /// @}

    // ============================================================================
    /// @name Vector Operations
    // ============================================================================
    /// @{

    /// Dot product: a · b.
    decimal dotProduct(const Vector3D&) const;

    /// Cross product: a × b (right-hand rule).
    Vector3D crossProduct(const Vector3D&) const;
    /// @}

    /// Element-wise comparisons.
    // ============================================================================
    /// @name Operators Comparison
    // ============================================================================
    /// @{
    bool operator==(const Vector3D&) const;
    bool operator!=(const Vector3D&) const;
    bool operator<(const Vector3D&) const;
    bool operator<=(const Vector3D&) const;
    bool operator>(const Vector3D&) const;
    bool operator>=(const Vector3D&) const;
    bool approxEqual(const Vector3D&, decimal) const;
    /// @}

    // ============================================================================
    /// @name Operators Element Access
    // ============================================================================
    /// @{

    /// Access vector element with index range checking.
    decimal& at(int);
    /// Access vector element with index range checking (const version).
    decimal at(int) const;
    /// Access vector element without index range checking.
    decimal& operator()(int);
    /// Access vector element without index range checking (const version).
    decimal operator()(int) const;
    /// Access vector element without index range checking.
    decimal& operator[](int);
    /// Access vector element without index range checking (const version).
    decimal operator[](int) const;
    /// @}

    /// Element-wise arithmetic operators (in-place).
    // ============================================================================
    /// @name Operators Arithmetic In-Place
    // ============================================================================
    /// @{

    /// Negate each element of the vector.
    Vector3D  operator-() const;
    Vector3D& operator+=(const Vector3D&);
    Vector3D& operator-=(const Vector3D&);
    Vector3D& operator*=(const Vector3D&);
    /// Element-wise division by another vector. Throw `std::invalid_argument` on division by zero.
    Vector3D& operator/=(const Vector3D&);
    Vector3D& operator+=(decimal);
    Vector3D& operator-=(decimal);
    Vector3D& operator*=(decimal);
    /// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
    Vector3D& operator/=(decimal);
    /// @}
};
// ============================================================================
//  Free Functions
// ============================================================================

/// @addtogroup VectorMaths
/// @{

// ============================================================================
/// @name Utilities
// ============================================================================
/// @{

/// Element-wise minimum between two vectors.
Vector3D min(const Vector3D& a, const Vector3D& b);

/// Element-wise maximum between two vectors.
Vector3D max(const Vector3D& a, const Vector3D& b);
/// @}

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
/// @}

/// Element-wise arithmetic operations.
// ============================================================================
/// @name Operators
// ============================================================================
/// @{

Vector3D operator+(const Vector3D&, const Vector3D&);
Vector3D operator-(const Vector3D&, const Vector3D&);
Vector3D operator*(const Vector3D&, const Vector3D&);
/// Element-wise division between two vectors. Throw `std::invalid_argument` on division by zero.
Vector3D operator/(const Vector3D&, const Vector3D&);

Vector3D operator+(const Vector3D&, decimal);
Vector3D operator-(const Vector3D&, decimal);
Vector3D operator*(const Vector3D&, decimal);
/// Element-wise division between a vector and a decimal. Throw `std::invalid_argument` on division by zero.
Vector3D operator/(const Vector3D&, decimal);

Vector3D operator+(decimal, const Vector3D&);
Vector3D operator-(decimal, const Vector3D&);
Vector3D operator*(decimal, const Vector3D&);
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
