/**
 * @file vector.hpp
 * @author Tom Laclavère
 * @brief Declaration of a 3D vector class for mathematical operations.
 */
#pragma once
#include "common.hpp"
#include "config.hpp"
#include "precision.hpp"

#include <algorithm>
#include <array>
#include <ostream>

/**
 * @brief 3D vector class for mathematical operations.
 *
 * The Vector3D class represents a three-dimensional vectors and provides various operations for mathematical
 * computations. The implementation uses the `decimal` type defined in `precision.hpp` for its components, and
 * `std::array<decimal, 3>` for internal storage.
 * The representation of the vector is v = (x, y, z).
 *
 *  * Example usage:
 * @code
 * Vector3D v1(1.0, 2.0, 3.0);
 * Vector3D v2(4.0, 5.0, 6.0);
 * auto dot = v1.dotProduct(v2);
 * auto cross = v1.crossProduct(v2);
 * @endcode
 *
 */
struct Vector3D
{
private:
    std::array<decimal, 3> v { 0, 0, 0 };

public:
    // ============================================================================
    // ============================================================================
    //  Constructors
    // ============================================================================
    // ============================================================================
    /**
     * @brief Default constructor.
     *
     * Initializes all components to zero.
     * Equivalent to Vector3D(0, 0, 0).
     */
    Vector3D() = default;
    /**
     * @brief Construct a vector with all components set to the same value.
     *
     * @param value  The value to set all components to.
     */
    Vector3D(decimal value)
        : v { value, value, value }
    {}
    /**
     * @brief Construct a vector with specified components.
     *
     * @param x
     * @param y
     * @param z
     */
    Vector3D(decimal x, decimal y, decimal z)
        : v { x, y, z }
    {}
    /**
     * @brief Copy constructor.
     *
     * @param newv  The vector to copy from.
     */
    Vector3D(const Vector3D& newv)
        : v { newv.v }
    {}

    // ============================================================================
    // ============================================================================
    //  Getters
    // ============================================================================
    // ============================================================================
    decimal                getX() const { return v[0]; }
    decimal                getY() const { return v[1]; }
    decimal                getZ() const { return v[2]; }
    std::array<decimal, 3> getV() const { return v; }

    // ============================================================================
    // ============================================================================
    //  Utilities
    // ============================================================================
    // ============================================================================
    /**
     * @brief Convert this vector to its absolute value in-place.
     *
     * Each component is replaced with its absolute value.
     */
    void absolute();

    /**
     * @brief Normalise the vector in-place.
     *
     * The vector is scaled to unit length while preserving its direction.
     * If the vector has zero length, this vector becomes a null vector.
     */
    void normalize();

    /**
     * @brief Return the squared norm of the vector.
     * This is more efficient than `getNorm()` for comparison operations.
     *
     * @return decimal The squared norm value (x² + y² + z^{2}).
     */
    decimal getNormSquare() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }

    /**
     * @brief Return the norm of the vector.
     *
     * @return decimal The norm value \sqrt{x² + y² + z²}.
     */
    decimal getNorm() const { return std::sqrt(getNormSquare()); }

    /**
     * @brief Return the minimun component value.
     *
     * @return decimal The smallest of the x, y, and z components.
     */
    decimal getMinValue() const { return std::min({ v[0], v[1], v[2] }); }

    /**
     * @brief Return the maximum component value.
     *
     * @return decimal The largest of the x, y, and z components.
     */
    decimal getMaxValue() const { return std::max({ v[0], v[1], v[2] }); }

    /**
     * @brief Return a new vector with absolute values of components.
     *
     * @return Vector3D New vector with each component set to the absolute value of the original.
     */

    Vector3D getAbsoluteVector() const;
    /**
     * @brief Return a normalised copy of the vector.
     *
     * @return Vector3D New vector with the same direction but unit lenght.
     */
    Vector3D getNormalized() const;

    // ============================================================================
    // ============================================================================
    //  Setters
    // ============================================================================
    // ============================================================================
    void setX(decimal);
    void setY(decimal);
    void setZ(decimal);

    /**
     * @brief Set to the null vector.
     *
     */
    void setToZero();

    /**
     * @brief Set all components to
     * @param decimal The value to set for all components.
     */
    void setAllValues(decimal);

    /**
     * @brief Sets all components to specified values.
     * @param x The new x-component value.
     * @param y The new y-component value.
     * @param z The new z-component value.
     */
    void setAllValues(decimal, decimal, decimal);

    // ============================================================================
    // ============================================================================
    //  Property Checks
    // ============================================================================
    // ============================================================================
    bool isZero() const;
    bool isLengthEqual(decimal) const;
    bool isFinite() const;
    bool isNormalized() const;

    // ============================================================================
    // ============================================================================
    //  Vector Operations
    // ============================================================================
    // ============================================================================
    decimal  dotProduct(const Vector3D&) const;
    Vector3D crossProduct(const Vector3D&) const;

    // ============================================================================
    // ============================================================================
    //  Comparison Operators
    // ============================================================================
    // ============================================================================
    bool operator==(const Vector3D&) const;
    bool operator!=(const Vector3D&) const;
    bool operator<(const Vector3D&) const;
    bool operator<=(const Vector3D&) const;
    bool operator>(const Vector3D&) const;
    bool operator>=(const Vector3D&) const;
    bool approxEqual(const Vector3D&, decimal) const;

    // ============================================================================
    // ============================================================================
    //  Element Acess Operators
    // ============================================================================
    // ============================================================================
    // Element access with index checking
    decimal& operator()(int);
    decimal  operator()(int) const;
    // Element acces without index checking
    decimal& operator[](int);
    decimal  operator[](int) const;

    // ============================================================================
    // ============================================================================
    //  In-Place Arithmetic Operators
    // ============================================================================
    // ============================================================================
    Vector3D  operator-() const;
    Vector3D& operator+=(const Vector3D&);
    Vector3D& operator-=(const Vector3D&);
    Vector3D& operator*=(const Vector3D&);
    Vector3D& operator/=(const Vector3D&);
    Vector3D& operator+=(decimal);
    Vector3D& operator-=(decimal);
    Vector3D& operator*=(decimal);
    Vector3D& operator/=(decimal);
};
// ============================================================================
// ============================================================================
//  Helper for Free Arithmetic Operators
// ============================================================================
// ============================================================================
template <class F>
inline Vector3D applyVector(const Vector3D& A, const Vector3D& B, F&& f)
{
    return Vector3D { f(A[0], B[0]), f(A[1], B[1]), f(A[2], B[2]) };
}
template <class F>
inline Vector3D applyVector(const Vector3D& A, decimal s, F&& f)
{
    return Vector3D { f(A[0], s), f(A[1], s), f(A[2], s) };
}
// ============================================================================
// ============================================================================
//  Utilities
// ============================================================================
// ============================================================================
Vector3D min(const Vector3D& a, const Vector3D& b);
Vector3D max(const Vector3D& a, const Vector3D& b);

// ============================================================================
// ============================================================================
//  Vector Operations
// ============================================================================
// ============================================================================
decimal  dotProduct(const Vector3D&, const Vector3D&);
Vector3D crossProduct(const Vector3D&, const Vector3D&);

// ============================================================================
// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ============================================================================
Vector3D operator+(const Vector3D&, const Vector3D&);
Vector3D operator-(const Vector3D&, const Vector3D&);
Vector3D operator*(const Vector3D&, const Vector3D&);
Vector3D operator/(const Vector3D&, const Vector3D&);

Vector3D operator+(const Vector3D&, decimal);
Vector3D operator-(const Vector3D&, decimal);
Vector3D operator*(const Vector3D&, decimal);
Vector3D operator/(const Vector3D&, decimal);

Vector3D operator+(decimal, const Vector3D&);
Vector3D operator-(decimal, const Vector3D&);
Vector3D operator*(decimal, const Vector3D&);
Vector3D operator/(decimal, const Vector3D&);

// ============================================================================
// ============================================================================
//  Printing
// ============================================================================
// ============================================================================
std::ostream& operator<<(std::ostream&, const Vector3D&);
