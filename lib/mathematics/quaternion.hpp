#pragma once

#include "matrix.hpp"
#include "precision.hpp"
#include "vector.hpp"

#include <array>

// Class Quaternion
/**
 * This class represents a quaternion. We use the notation :
 * q = (x*i, y*j, z*k, w) to represent a quaternion.
 * Imaginary part : Vector3D.
 * Real part : decimal.
 */
struct Quaternion
{
private:
    Vector3D v { Vector3D() };
    decimal  w { 0 };

public:
    // ============================================================================
    //  Constructors
    // ============================================================================
    Quaternion() = default;
    Quaternion(decimal newx, decimal newy, decimal newz, decimal neww)
        : v { Vector3D(newx, newy, newz) }
        , w { neww }
    {}
    Quaternion(const Vector3D& v, decimal w)
        : v { v }
        , w { w }
    {}
    Quaternion(decimal w, const Vector3D& v)
        : v { v }
        , w { w }
    {}
    Quaternion(const Matrix3x3& m);

    // ============================================================================
    //  Getters
    // ============================================================================
    decimal   getRealPart() const { return w; }
    Vector3D  getImaginaryPart() const { return v; }
    Vector3D  getImaginaryPartElement(int index) const { return v[index]; }
    Matrix3x3 getRotationMatrix() const;

    // ============================================================================
    //  Utilities
    // ============================================================================
    decimal getLength() const;
    decimal getLengthSquare() const;
};
