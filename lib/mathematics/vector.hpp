#pragma once
#include "common.hpp"
#include "config.hpp"
#include "precision.hpp"

#include <algorithm>
#include <array>
#include <ostream>

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
    Vector3D() = default;
    Vector3D(decimal value)
        : v { value, value, value }
    {}
    Vector3D(decimal x, decimal y, decimal z)
        : v { x, y, z }
    {}
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
    void     absolute();
    void     normalize();
    decimal  getNormSquare() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }
    decimal  getNorm() const { return std::sqrt(getNormSquare()); }
    decimal  getMinValue() const { return std::min({ v[0], v[1], v[2] }); }
    decimal  getMaxValue() const { return std::max({ v[0], v[1], v[2] }); }
    Vector3D getAbsoluteVector() const;
    Vector3D getNormalized() const;

    // ============================================================================
    // ============================================================================
    //  Setters
    // ============================================================================
    // ============================================================================
    void setX(decimal);
    void setY(decimal);
    void setZ(decimal);
    void setToZero();
    void setAllValues(decimal);
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
    Vector3D& operator-();
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
