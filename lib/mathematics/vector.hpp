#pragma once
#include "common.hpp"
#include "config.hpp"
#include "precision.hpp"

#include <array>
#include <ostream>

struct Vector3D
{
private:
    std::array<decimal, 3> v { { 0, 0, 0 } };

public:
    // Constructors
    Vector3D() = default;
    Vector3D(decimal x, decimal y, decimal z)
        : v { x, y, z }
    {}

    // Getters
    decimal getX() const { return v[0]; }
    decimal getY() const { return v[1]; }
    decimal getZ() const { return v[2]; }
    decimal getNormSquare() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }
    decimal getNorm() const { return std::sqrt(getNormSquare()); }
    decimal getMinValue() const { return std::min({ v[0], v[1], v[2] }); }
    decimal getMaxValue() const { return std::max({ v[0], v[1], v[2] }); }

    // utility
    Vector3D        getAbsoluteVector() const;
    Vector3D        getNormalizedVector() const;
    void            normalize();
    static Vector3D min(const Vector3D& a, const Vector3D& b);
    static Vector3D max(const Vector3D& a, const Vector3D& b);

    // setters
    void setX(decimal);
    void setY(decimal);
    void setZ(decimal);
    void setToZero();
    void setAllValues(decimal);
    void setAllValues(decimal, decimal, decimal);

    // predicates
    bool isZero() const;
    bool isUnit() const;
    bool isLengthEqual(decimal) const;
    bool isFinite() const;

    // products
    decimal  dotProduct(const Vector3D&) const;
    Vector3D crossProduct(const Vector3D&) const;

    // comparisons
    bool operator==(const Vector3D&) const;
    bool operator!=(const Vector3D&) const;
    bool operator<(const Vector3D&) const;
    bool approxEqual(const Vector3D&, decimal) const;

    // indexing
    decimal& operator[](int);
    decimal  operator[](int) const;

    // in-place arithmetic
    Vector3D  operator-() const;
    Vector3D& operator+=(const Vector3D&);
    Vector3D& operator-=(const Vector3D&);
    Vector3D& operator*=(const Vector3D&);
    Vector3D& operator/=(const Vector3D&);
    Vector3D& operator+=(decimal);
    Vector3D& operator-=(decimal);
    Vector3D& operator*=(decimal);
    Vector3D& operator/=(decimal);

    // helper for free operators
    template <class F>
    static Vector3D apply(const Vector3D& A, const Vector3D& B, F&& f);
    template <class F>
    static Vector3D apply(const Vector3D& A, decimal s, F&& f);
};

// free arithmetic operators
Vector3D operator+(Vector3D, const Vector3D&);
Vector3D operator-(Vector3D, const Vector3D&);
Vector3D operator*(Vector3D, const Vector3D&);
Vector3D operator/(Vector3D, const Vector3D&);

Vector3D operator+(Vector3D, decimal);
Vector3D operator-(Vector3D, decimal);
Vector3D operator*(Vector3D, decimal);
Vector3D operator/(Vector3D, decimal);

Vector3D operator+(decimal, const Vector3D&);
Vector3D operator-(decimal, const Vector3D&);
Vector3D operator*(decimal, const Vector3D&);
Vector3D operator/(decimal, const Vector3D&);

// comparison with stream
std::ostream& operator<<(std::ostream&, const Vector3D&);
