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
    void       setToZero();
    void       conjugate();
    void       normalize();
    void       inverse();
    decimal    getNormSquare() const { return w * w + v.getNormSquare(); }
    decimal    getNorm() const { return std::sqrt(getNormSquare()); }
    Quaternion getIdentity() const { return Quaternion(1, Vector3D()); }
    Quaternion getZero() const { return Quaternion(); }
    Quaternion getConjugate() const
    {
        Quaternion conjugatedQ = Quaternion((*this));
        conjugatedQ.conjugate();
        return conjugatedQ;
    }
    Quaternion getNormalize() const
    {
        Quaternion normalizedQ = Quaternion((*this));
        normalizedQ.normalize();
        return normalizedQ;
    }
    Quaternion getInverse() const
    {
        Quaternion inverseQ = Quaternion((*this));
        inverseQ.inverse();
        return inverseQ;
    }

    // ============================================================================
    //  Setters
    // ============================================================================
    void setRealPart(decimal value) { w = value; }
    void setImaginaryPart(decimal newx, decimal newy, decimal newz);
    void setImaginaryPart(Vector3D& v);
    void setToIdentity();
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
    decimal dotProduct(const Quaternion& q);

    // ============================================================================
    //  Comparisons Operators
    // ============================================================================
    bool operator==(const Quaternion&) const;
    bool operator!=(const Quaternion&) const;
    bool operator<(const Quaternion&) const;
    bool operator<=(const Quaternion&) const;
    bool operator>(const Quaternion&) const;
    bool operator>=(const Quaternion&) const;
    bool approxEqual(const Quaternion&, decimal) const;

    // ============================================================================
    //  Element Access Operators
    // ============================================================================
    // Element access with index checking
    decimal& operator()(int);
    decimal  operator()(int) const;
    // Element acces without index checking
    decimal& operator[](int);
    decimal  operator[](int) const;

    // ============================================================================
    //  In-Place Arithmetic Operators
    // ============================================================================
    Quaternion  operator-() const;
    Quaternion& operator+=(const Quaternion&);
    Quaternion& operator-=(const Quaternion&);
    Quaternion& operator*=(const Quaternion&);
    Quaternion& operator/=(const Quaternion&);
    Quaternion& operator+=(decimal);
    Quaternion& operator-=(decimal);
    Quaternion& operator*=(decimal);
    Quaternion& operator/=(decimal);

    // ============================================================================
    //  Helper for Free Arithmetic Operators
    // ============================================================================
    template <class F>
    static Quaternion apply(const Quaternion& A, const Quaternion& B, F&& f);
    template <class F>
    static Quaternion apply(const Quaternion& A, decimal s, F&& f);
};
// ============================================================================
//  Utilities
// ============================================================================
Quaternion min(const Quaternion& a, const Quaternion& b);
Quaternion max(const Vector3D& a, const Quaternion& b);

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
Quaternion operator+(const Quaternion&, const Quaternion&);
Quaternion operator-(const Quaternion&, const Quaternion&);
Quaternion operator*(const Quaternion&, const Quaternion&);
Quaternion operator/(const Quaternion&, const Quaternion&);

Quaternion operator+(const Quaternion&, decimal);
Quaternion operator-(const Quaternion&, decimal);
Quaternion operator*(const Quaternion&, decimal);
Quaternion operator/(const Quaternion&, decimal);

Quaternion operator+(decimal, const Quaternion&);
Quaternion operator-(decimal, const Quaternion&);
Quaternion operator*(decimal, const Quaternion&);
Quaternion operator/(decimal, const Quaternion&);

// ============================================================================
//  Printing
// ============================================================================
std::ostream& operator<<(std::ostream&, const Quaternion&);
