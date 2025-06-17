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
    // From Euler Angles
    Quaternion(decimal angleX, decimal angleY, decimal angleZ);
    Quaternion(const Vector3D& eulerAngles);

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
    decimal    getLength() const;
    decimal    getLengthSquare() const;
    Matrix3x3  getMatrix() const;
    Quaternion getIdentity() const;
    Quaternion getZero() const;
    Quaternion getUnit() const;
    Quaternion getConjugate() const;
    Quaternion getInverse() const;
    Quaternion getNormalize() const;

    // ============================================================================
    //  Setters
    // ============================================================================
    void setRealPart(decimal value);
    void setImaginaryPart(decimal newx, decimal newy, decimal newz);
    void setImaginaryPart(Vector3D& v);
    void setToIdentity();
    void setToZero();
    void normalize();
    void inverse();
    void setAllValues(decimal newx, decimal newy, decimal newz, decimal neww);
    void setAllValues(const Vector3D& v, decimal w);
    void setAllValues(decimal w, const Vector3D& v);
    void setAllValues(const Matrix3x3& m);
    // From Euler Angles
    void setAllValues(decimal angleX, decimal angleY, decimal angleZ);
    void setAllValues(const Vector3D& eulerAngles);

    // ============================================================================
    //  Property Checks
    // ============================================================================
    void isValid() const;
    bool isIdentity() const;
    bool isZero() const;
    bool isFinite() const;
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isNormalize() const;

    // ============================================================================
    //  Quaternion Operations
    // ============================================================================

    // ============================================================================
    //  Comparisons Operators
    // ============================================================================

    // ============================================================================
    //  Element Access Operators
    // ============================================================================

    // ============================================================================
    //  In-Place Arithmetic Operators
    // ============================================================================

    // ============================================================================
    //  Helper for Free Arithmetic Operators
    // ============================================================================
};
