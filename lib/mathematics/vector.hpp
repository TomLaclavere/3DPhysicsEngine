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
    // ===== Constructors =====
    Vector3D() = default;
    Vector3D(decimal value)
        : v { value, value, value }
    {}
    Vector3D(decimal x, decimal y, decimal z)
        : v { x, y, z }
    {}

    // ===== Accessors =====
    decimal getX() const { return v[0]; }
    decimal getY() const { return v[1]; }
    decimal getZ() const { return v[2]; }

    // ===== Utilities =====
    decimal         getNormSquare() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }
    decimal         getNorm() const { return std::sqrt(getNormSquare()); }
    decimal         getMinValue() const { return std::min({ v[0], v[1], v[2] }); }
    decimal         getMaxValue() const { return std::max({ v[0], v[1], v[2] }); }
    Vector3D        getAbsoluteVector() const;
    Vector3D        getNormalizedVector() const;
    void            normalize();
    static Vector3D min(const Vector3D& a, const Vector3D& b);
    static Vector3D max(const Vector3D& a, const Vector3D& b);

    // ===== Setters =====
    void setX(decimal);
    void setY(decimal);
    void setZ(decimal);
    void setToZero();
    void setAllValues(decimal);
    void setAllValues(decimal, decimal, decimal);

    // ===== Property Checks =====
    bool isZero() const;
    bool isUnit() const;
    bool isLengthEqual(decimal) const;
    bool isFinite() const;

    // ===== Vector Operations =====
    decimal  dotProduct(const Vector3D&) const;
    Vector3D crossProduct(const Vector3D&) const;

    // ===== Comparison Operators =====
    bool operator==(const Vector3D&) const;
    bool operator!=(const Vector3D&) const;
    bool operator<(const Vector3D&) const;
    bool operator<=(const Vector3D&) const;
    bool operator>(const Vector3D&) const;
    bool operator>=(const Vector3D&) const;
    bool approxEqual(const Vector3D&, decimal) const;

    // ===== Element Access =====
    decimal& operator[](int);
    decimal  operator[](int) const;

    // ===== In-Place Arithmetic Operators =====
    Vector3D  operator-() const;
    Vector3D& operator+=(const Vector3D&);
    Vector3D& operator-=(const Vector3D&);
    Vector3D& operator*=(const Vector3D&);
    Vector3D& operator/=(const Vector3D&);
    Vector3D& operator+=(decimal);
    Vector3D& operator-=(decimal);
    Vector3D& operator*=(decimal);
    Vector3D& operator/=(decimal);

    // ===== Helper for Free Arithmetic Operators =====
    template <class F>
    static Vector3D apply(const Vector3D& A, const Vector3D& B, F&& f);
    template <class F>
    static Vector3D apply(const Vector3D& A, decimal s, F&& f);
};

// ===== Free Arithmetic Operators =====
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

// ===== Printing =====
std::ostream& operator<<(std::ostream&, const Vector3D&);
