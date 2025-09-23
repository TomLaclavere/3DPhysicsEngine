/**
 * @file quaternion.hpp
 * @brief Quaternion class and operations for 3D rotations and algebra.
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
 * q1.normalize();
 * Quaternion q3 = q1 * q2;
 * @endcode
 */
struct Quaternion
{
private:
    Vector3D v { Vector3D() }; ///< Imaginary part (x, y, z).
    decimal  w { 0 };          ///< Real part (w).

public:
    // ============================================================================
    /// @name Constructors
    // ============================================================================
    /// @{
    Quaternion() = default;
    Quaternion(decimal _x, decimal _y, decimal _z, decimal _w)
        : v { Vector3D(_x, _y, _z) }
        , w { _w }
    {}
    Quaternion(const Vector3D& _v, decimal _w)
        : v { _v }
        , w { _w }
    {}
    Quaternion(decimal w, const Vector3D& v)
        : Quaternion(v, w)
    {}
    /// Constructor from rotation matrix.
    Quaternion(const Matrix3x3& m);
    /// Constructor from Euler angles.
    Quaternion(decimal angleX, decimal angleY, decimal angleZ);
    /// Constructor from Euler angles.
    Quaternion(const Vector3D& eulerAngles);
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    decimal   getRealPart() const { return w; };
    Vector3D  getImaginaryPart() const { return v; };
    Vector3D  getImaginaryPartElement(int index) const { return v[index]; };
    Matrix3x3 getRotationMatrix() const;
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// In-place conjugate (negate imaginary part).
    void conjugate();
    /// In-place normalise. If zero-length quaternion, becomes null quaternion.
    void normalize();
    /// In-place inverse (conjugate then normalize).
    void inverse();
    /// Squared Euclidean norm. Cheaper than `getNorm()`.
    decimal getNormSquare() const;
    /// Euclidean norm.
    decimal getNorm() const;
    /// Return identity quaternion (0, 0, 0, 1).
    Quaternion getIdentity() const;
    /// Return null quaternion (0, 0, 0, 0).
    Quaternion getNull() const;
    /// Return a conjugated copy of the quaternion.
    Quaternion getConjugate() const;
    /// Return a normalized copy of the quaternion.
    Quaternion getNormalize() const;
    /// Return an inverted copy of the quaternion.
    Quaternion getInverse() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setRealPart(decimal value);
    void setImaginaryPart(decimal, decimal, decimal);
    void setImaginaryPart(const Vector3D& newv);
    void setToZero();
    void setToIdentity();
    void setAllValues(decimal, decimal, decimal, decimal);
    void setAllValues(const Vector3D&, decimal);
    void setAllValues(decimal, const Vector3D&);
    /// Set all values from rotation matrix.
    void setAllValues(const Matrix3x3&);
    /// Set all values from the three Euler angles.
    void setAllValues(decimal, decimal, decimal);
    /// Set all values from the three Euler angles.
    void setAllValues(const Vector3D&);
    /// @}

    // ============================================================================
    /// @name Property Checks
    // ============================================================================
    /// @{
    bool isFinite() const;
    bool isZero() const;
    bool isUnit() const;
    bool isIdentity() const;
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isNormalized() const;
    /// @}

    // ============================================================================
    /// @name Quaternion Operations
    // ============================================================================
    /// @{
    decimal    dotProduct(const Quaternion&) const;
    Quaternion crossProduct(const Quaternion&) const;
    /// @}

    /// Element-wise comparisons.
    // ============================================================================
    /// @name Comparisons Operators
    // ============================================================================
    /// @{
    bool operator==(const Quaternion&) const;
    bool operator!=(const Quaternion&) const;
    bool operator<(const Quaternion&) const;
    bool operator<=(const Quaternion&) const;
    bool operator>(const Quaternion&) const;
    bool operator>=(const Quaternion&) const;
    bool approxEqual(const Quaternion&, decimal) const;
    /// @}

    /// Imaginary part element access operator. Rely on `Vector3D` operators.
    // ============================================================================
    /// @name Element Access Operators
    // ============================================================================
    /// @{

    /// Access quaternion element with index range checking.
    decimal& at(int);
    /// Access quaternion element with index range checking (const version).
    decimal at(int) const;
    /// Access quaternion element without index range checking.
    decimal& operator()(int);
    /// Access quaternion element without index range checking (const version).
    decimal operator()(int) const;
    /// Access quaternion element without index range checking.
    decimal& operator[](int);
    /// Access quaternion element without index range checking (const version).
    decimal operator[](int) const;
    /// @}

    /// Element-wise arithmetic operators (in-place).
    // ============================================================================
    /// @name In-Place Arithmetic Operators
    // ============================================================================
    /// @{

    /// Negate each element of the quaternion.
    Quaternion& operator-();
    Quaternion& operator+=(const Quaternion&);
    Quaternion& operator-=(const Quaternion&);
    Quaternion& operator*=(const Quaternion&);
    /// Element-wise division by another quaternion. Throw `std::invalid_argument` on division by zero.
    Quaternion& operator/=(const Quaternion&);
    Quaternion& operator+=(decimal);
    Quaternion& operator-=(decimal);
    Quaternion& operator*=(decimal);
    /// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
    Quaternion& operator/=(decimal);
    /// @}

    /// Internal functions to compute element-wise operations.
    // ============================================================================
    /// @name Internal Utilities
    // ============================================================================
    /// @{

    /// Apply a binary operation element-wise between two quaternions.
    template <class F>
    static Quaternion apply(const Quaternion& A, const Quaternion& B, F&& func);
    /// Apply a binary operation element-wise between a quaternion and a decimal.
    template <class F>
    static Quaternion apply(const Quaternion& A, decimal s, F&& func);
    /// @}
};

// ============================================================================
/// @addtogroup QuaternionMaths
/// @{

// ============================================================================
/// @name Quaternion operations
// ============================================================================
/// @{
decimal    dotProduct(const Quaternion&, const Quaternion&);
Quaternion crossProduct(const Quaternion&, const Quaternion&);
/// @}

// ============================================================================
/// @name Utilities
// ============================================================================
/// @{
Quaternion min(const Quaternion&, const Quaternion&);
Quaternion max(const Quaternion&, const Quaternion&);
/// Compute quaternion from the three Euler angles. Used to implement constructors.
std::array<decimal, 4> eulerAngles_to_Quaternion(decimal angleX, decimal angleY, decimal angleZ);
/// @}

/// Element-wise arithmetic operations.
// ============================================================================
/// @name Operators
// ============================================================================
/// @{
Quaternion operator+(const Quaternion&, const Quaternion&);
Quaternion operator-(const Quaternion&, const Quaternion&);
Quaternion operator*(const Quaternion&, const Quaternion&);
/// Element-wise division by another quaternion. Throw `std::invalid_argument` on division by zero.
Quaternion operator/(const Quaternion&, const Quaternion&);

Quaternion operator+(const Quaternion&, decimal);
Quaternion operator-(const Quaternion&, decimal);
Quaternion operator*(const Quaternion&, decimal);
/// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
Quaternion operator/(const Quaternion&, decimal);

Quaternion operator+(decimal, const Quaternion&);
Quaternion operator-(decimal, const Quaternion&);
Quaternion operator*(decimal, const Quaternion&);
/// Element-wise division by a decimal. Throw `std::invalid_argument` on division by zero.
Quaternion operator/(decimal, const Quaternion&);
/// @}

// ============================================================================
/// @name Printing
// ============================================================================
/// @{

/**
 * Stream output operator for `Quaternion`.
 * Return format is (x,y,z,w).
 */
std::ostream& operator<<(std::ostream&, const Quaternion&);
/// @}

/// @}
