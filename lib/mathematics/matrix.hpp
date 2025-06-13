#pragma once
#include "precision.hpp"
#include "vector.hpp"

#include <array>

struct Matrix3x3
{
private:
    std::array<Vector3D, 3> m { Vector3D(), Vector3D(), Vector3D() };

public:
    // ============================================================================
    //  Constructors
    // ============================================================================
    Matrix3x3() = default;
    Matrix3x3(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23, decimal m31,
              decimal m32, decimal m33)
        : m { Vector3D(m11, m12, m13), Vector3D(m21, m22, m23), Vector3D(m31, m32, m33) }
    {}
    Matrix3x3(decimal value)
        : m { Vector3D(value, value, value), Vector3D(value, value, value), Vector3D(value, value, value) }
    {}
    Matrix3x3(const Vector3D& row1, const Vector3D& row2, const Vector3D& row3)
        : m { row1, row2, row3 }
    {}
    Matrix3x3(const Matrix3x3& m)
        : m { m.m }
    {}

    // ============================================================================
    //  Getters
    // ============================================================================
    Vector3D getRow(int index) const { return m[index]; }
    Vector3D getColumn(int index) const { return Vector3D(m[0][index], m[1][index], m[2][index]); }
    Vector3D getDiagonal() const { return Vector3D(m[0][0], m[1][1], m[2][2]); }

    // ============================================================================
    //  Utilities
    // ============================================================================
    decimal   getMinValue() const;
    decimal   getMaxValue() const;
    decimal   getDeterminant() const;
    decimal   getTrace() const;
    Matrix3x3 getInverse() const;
    Matrix3x3 getTranspose() const;
    Matrix3x3 getAbsolute() const;

    // ============================================================================
    //  Setters
    // ============================================================================
    void setRow(int index, const Vector3D& row);
    void setColumn(int index, const Vector3D& column);
    void setDiagonal(const Vector3D& diagonal);
    void setToIdentity();
    void setToZero();
    void setAllValues(decimal value);
    void setAllValues(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23,
                      decimal m31, decimal m32, decimal m33);

    // ============================================================================
    //  Property Checks
    // ============================================================================
    bool isIdentity() const;
    bool isZero() const;
    bool isFinite() const;
    bool isDiagonal() const;
    bool isSymmetric() const;
    bool isInvertible() const;
    bool isOrthogonal() const;

    // ============================================================================
    //  Matrix Operations
    // ============================================================================
    Matrix3x3 matrixProduct(const Matrix3x3& matrix) const;
    Vector3D  matrixVectorProduct(const Vector3D& vector) const;
    Vector3D  vectorMatrixProduct(const Vector3D& vector) const;

    // ============================================================================
    //  Comparisons Operators
    // ============================================================================
    bool operator==(const Matrix3x3& matrix) const;
    bool operator!=(const Matrix3x3& matrix) const;
    bool operator<(const Matrix3x3& matrix) const;
    bool operator<=(const Matrix3x3& matrix) const;
    bool operator>(const Matrix3x3& matrix) const;
    bool operator>=(const Matrix3x3& matrix) const;
    bool approxEqual(const Matrix3x3& matrix, decimal precision = PRECISION_MACHINE) const;

    // ============================================================================
    //  Element Access Operators
    // ============================================================================
    // Element access with index checking
    decimal&  at(int row, int column);
    decimal   at(int row, int column) const;
    Vector3D& at(int row);
    Vector3D  at(int row) const;
    // Element access without index checking
    decimal&  operator()(int row, int column);
    decimal   operator()(int row, int column) const;
    Vector3D& operator()(int row);
    Vector3D  operator()(int row) const;
    Vector3D& operator[](int row);
    Vector3D  operator[](int row) const;

    // ============================================================================
    //  In-Place Arithmetic Operators
    // ============================================================================
    Matrix3x3  operator-() const;
    Matrix3x3& operator+=(const Matrix3x3& matrix);
    Matrix3x3& operator-=(const Matrix3x3& matrix);
    Matrix3x3& operator*=(const Matrix3x3& matrix);
    Matrix3x3& operator/=(const Matrix3x3& matrix);
    Matrix3x3& operator+=(const Vector3D& vector);
    Matrix3x3& operator-=(const Vector3D& vector);
    Matrix3x3& operator*=(const Vector3D& vector);
    Matrix3x3& operator/=(const Vector3D& vector);
    Matrix3x3& operator+=(decimal value);
    Matrix3x3& operator-=(decimal value);
    Matrix3x3& operator*=(decimal value);
    Matrix3x3& operator/=(decimal value);

    // ============================================================================
    //  Helper for Free Arithmetic Operators
    // ============================================================================
    template <class F>
    static Matrix3x3 applyMatrix(const Matrix3x3& A, const Matrix3x3& B, F&& f);
    template <class F>
    static Matrix3x3 applyVector(const Matrix3x3& A, const Vector3D& B, F&& f);
    template <class F>
    static Matrix3x3 applyDecimal(const Matrix3x3& A, decimal s, F&& f);
};
// ============================================================================
//  Matrix Operations
// ============================================================================
Matrix3x3 matrixProduct(const Matrix3x3& matrix1, const Matrix3x3& matrix2);
Vector3D  matrixVectorProduct(const Matrix3x3& matrix, const Vector3D& vector);
Vector3D  vectorMatrixProduct(const Vector3D& vector, const Matrix3x3& matrix);

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
Matrix3x3 operator+(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator-(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator*(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator/(const Matrix3x3&, const Matrix3x3&);

Matrix3x3 operator+(const Matrix3x3&, const Vector3D&);
Matrix3x3 operator-(const Matrix3x3&, const Vector3D&);
Matrix3x3 operator*(const Matrix3x3&, const Vector3D&);
Matrix3x3 operator/(const Matrix3x3&, const Vector3D&);

Matrix3x3 operator+(const Vector3D&, const Matrix3x3&);
Matrix3x3 operator-(const Vector3D&, const Matrix3x3&);
Matrix3x3 operator*(const Vector3D&, const Matrix3x3&);
Matrix3x3 operator/(const Vector3D&, const Matrix3x3&);

Matrix3x3 operator+(const Matrix3x3&, decimal);
Matrix3x3 operator-(const Matrix3x3&, decimal);
Matrix3x3 operator*(const Matrix3x3&, decimal);
Matrix3x3 operator/(const Matrix3x3&, decimal);

Matrix3x3 operator+(decimal, const Matrix3x3&);
Matrix3x3 operator-(decimal, const Matrix3x3&);
Matrix3x3 operator*(decimal, const Matrix3x3&);
Matrix3x3 operator/(decimal, const Matrix3x3&);

// ============================================================================
//  Printing
// ============================================================================
std::ostream& operator<<(std::ostream&, const Matrix3x3&);
