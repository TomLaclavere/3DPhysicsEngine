#pragma once
#include "precision.hpp"
#include "vector.hpp"

#include <array>

struct Matrix3x3
{
private:
    std::array<decimal, 9> m { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

public:
    // ============================================================================
    // ============================================================================
    //  Constructors
    // ============================================================================
    // ============================================================================
    Matrix3x3() = default;
    Matrix3x3(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23, decimal m31,
              decimal m32, decimal m33)
        : m { m11, m12, m13, m21, m22, m23, m31, m32, m33 }
    {}
    Matrix3x3(decimal value)
        : m { value, value, value, value, value, value, value, value, value }
    {}
    Matrix3x3(const Vector3D& v)
        : m { v.getX(), v.getY(), v.getZ(), v.getX(), v.getY(), v.getZ(), v.getX(), v.getY(), v.getZ() }
    {}
    Matrix3x3(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3)
        : m { v1.getX(), v1.getY(), v1.getZ(), v2.getX(), v2.getY(),
              v2.getZ(), v3.getX(), v3.getY(), v3.getZ() }
    {}
    Matrix3x3(const Matrix3x3& m)
        : m { m.m }
    {}

    // ============================================================================
    // ============================================================================
    //  Element Access Operators
    // ============================================================================
    // ============================================================================
    // Mapping from 2D indices to 1D index for a 3x3 matrix
    int mapping(int ind_x, int ind_y) const;

    // 2D element access
    // Element access with index checking
    decimal& at(int ind_x, int ind_y);
    decimal  at(int ind_x, int ind_y) const;

    // Element access without index checking
    decimal& operator()(int ind_x, int ind_y);
    decimal  operator()(int ind_x, int ind_y) const;

    // 1D element access
    // Element access with index checking
    decimal& at(int ind);
    decimal  at(int ind) const;

    // Element access without index checking
    decimal& operator()(int ind);
    decimal  operator()(int ind) const;
    decimal& operator[](int ind);
    decimal  operator[](int ind) const;

    // ============================================================================
    // ============================================================================
    //  Getters
    // ============================================================================
    // ============================================================================
    Vector3D getRow(int index) const { return Vector3D(m[index * 3], m[index * 3 + 1], m[index * 3 + 2]); }
    Vector3D getColumn(int index) const { return Vector3D(m[index], m[index + 3], m[index + 6]); }
    Vector3D getDiagonal() const { return Vector3D(m[0], m[4], m[8]); }

    // ============================================================================
    // ============================================================================
    //  Utilities
    // ============================================================================
    // ============================================================================
    void      absolute();
    void      normalize(); // Normalization of a rotation matrix : Gram-Schmidt othonormalisation
    void      transpose();
    void      inverse();
    decimal   getMinValue() const;
    decimal   getMaxValue() const;
    decimal   getDeterminant() const;
    decimal   getTrace() const;
    Matrix3x3 getIdentity() const;
    Matrix3x3 getAbsolute() const;
    Matrix3x3 getNormalized() const;
    Matrix3x3 getTranspose() const;
    Matrix3x3 getInverse() const;

    // ============================================================================
    // ============================================================================
    //  Setters
    // ============================================================================
    // ============================================================================
    void setRow(int, const Vector3D&);
    void setColumn(int, const Vector3D&);
    void setDiagonal(const Vector3D&);
    void setToIdentity();
    void setToZero();
    void setAllValues(decimal);
    void setAllValues(Vector3D&);
    void setAllValues(const Vector3D&, const Vector3D&, const Vector3D&);
    void setAllValues(decimal, decimal, decimal, decimal, decimal, decimal, decimal, decimal, decimal);
    void setAllValues(const Matrix3x3&);

    // ============================================================================
    // ============================================================================
    //  Property Checks
    // ============================================================================
    // ============================================================================
    bool isIdentity() const;
    bool isZero() const;
    bool isFinite() const;
    bool isDiagonal() const;
    bool isSymmetric() const;
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isNormalized() const;

    // ============================================================================
    // ============================================================================
    //  Matrix Operations
    // ============================================================================
    // ============================================================================
    Matrix3x3 matrixProduct(const Matrix3x3&) const;
    Vector3D  matrixVectorProduct(const Vector3D&) const;
    Vector3D  vectorMatrixProduct(const Vector3D&) const;

    // ============================================================================
    // ============================================================================
    //  Comparisons Operators
    // ============================================================================
    // ============================================================================
    bool operator==(const Matrix3x3&) const;
    bool operator!=(const Matrix3x3&) const;
    bool operator<(const Matrix3x3&) const;
    bool operator<=(const Matrix3x3&) const;
    bool operator>(const Matrix3x3&) const;
    bool operator>=(const Matrix3x3&) const;
    bool approxEqual(const Matrix3x3&, decimal precision = PRECISION_MACHINE) const;

    // ============================================================================
    // ============================================================================
    //  In-Place Arithmetic Operators
    // ============================================================================
    // ============================================================================
    Matrix3x3& operator-();
    Matrix3x3& operator+=(const Matrix3x3&);
    Matrix3x3& operator-=(const Matrix3x3&);
    Matrix3x3& operator*=(const Matrix3x3&);
    Matrix3x3& operator/=(const Matrix3x3&);
    Matrix3x3& operator+=(const Vector3D&);
    Matrix3x3& operator-=(const Vector3D&);
    Matrix3x3& operator*=(const Vector3D&);
    Matrix3x3& operator/=(const Vector3D&);
    Matrix3x3& operator+=(decimal value);
    Matrix3x3& operator-=(decimal value);
    Matrix3x3& operator*=(decimal value);
    Matrix3x3& operator/=(decimal value);
};
// ============================================================================
// ============================================================================
//  Element Access Operators
// ============================================================================
// ============================================================================
inline int Matrix3x3::mapping(int ind_x, int ind_y) const
{
    if (ind_x < 0 || ind_x >= 3 || ind_y < 0 || ind_y >= 3)
    {
        throw std::out_of_range("Matrix3x3 indices out of range");
    }
    return ind_x * 3 + ind_y;
}

// 2D element access
inline decimal& Matrix3x3::at(int ind_x, int ind_y)
{
    if (ind_x < 0 || ind_x >= 3 || ind_y < 0 || ind_y >= 3)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[mapping(ind_x, ind_y)];
}
inline decimal Matrix3x3::at(int ind_x, int ind_y) const
{
    if (ind_x < 0 || ind_x >= 3 || ind_y < 0 || ind_y >= 3)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[mapping(ind_x, ind_y)];
}
inline decimal& Matrix3x3::operator()(int ind_x, int ind_y) { return m[mapping(ind_x, ind_y)]; }
inline decimal  Matrix3x3::operator()(int ind_x, int ind_y) const { return m[mapping(ind_x, ind_y)]; }

// 1D element access
inline decimal& Matrix3x3::at(int ind)
{
    if (ind < 0 || ind >= 9)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[ind];
}
inline decimal Matrix3x3::at(int ind) const
{
    if (ind < 0 || ind >= 9)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[ind];
}
inline decimal& Matrix3x3::operator()(int ind) { return m[ind]; }
inline decimal  Matrix3x3::operator()(int ind) const { return m[ind]; }
inline decimal& Matrix3x3::operator[](int ind) { return m[ind]; }
inline decimal  Matrix3x3::operator[](int ind) const { return m[ind]; }

// // ============================================================================
// // ============================================================================
// //  Helper for Free Arithmetic Operators
// // ============================================================================
// // ============================================================================
// template <class F>
// inline Matrix3x3 applyMatrix(const Matrix3x3& A, const Matrix3x3& B, F&& func)
// {
//     Matrix3x3 result;
//     for (int i = 0; i < 9; ++i)
//         result[i] = func(A[i], B[i]);
//     return result;
// }
// template <class F>
// inline Matrix3x3 applyMatrix(const Matrix3x3& A, const Vector3D& B, F&& func)
// {
//     Matrix3x3 result;
//     for (int i = 0; i < 3; ++i)
//         for (int j = 0; j < 3; ++j)
//             result(i, j) = func(A(i, j), B[i]);
//     return result;
// }
// template <class F>
// inline Matrix3x3 applyMatrix(const Matrix3x3& A, decimal B, F&& func)
// {
//     Matrix3x3 result;
//     for (int i = 0; i < 9; ++i)
//         result[i] = func(A[i], B);
//     return result;
// }

// ============================================================================
// ============================================================================
//  Matrix Operations
// ============================================================================
// ============================================================================
Matrix3x3 matrixProduct(const Matrix3x3& matrix1, const Matrix3x3& matrix2);
Vector3D  matrixVectorProduct(const Matrix3x3& matrix, const Vector3D&);
Vector3D  vectorMatrixProduct(const Vector3D& vector, const Matrix3x3&);

// ============================================================================
// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ============================================================================
Matrix3x3 operator+(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator-(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator*(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator/(const Matrix3x3&, const Matrix3x3&);

//! Matrix3x3 Vector3D operations need to be redifiened if needed
// Matrix3x3 operator+(const Matrix3x3&, const Vector3D&);
// Matrix3x3 operator-(const Matrix3x3&, const Vector3D&);
// Matrix3x3 operator*(const Matrix3x3&, const Vector3D&);
// Matrix3x3 operator/(const Matrix3x3&, const Vector3D&);

// Matrix3x3 operator+(const Vector3D&, const Matrix3x3&);
// Matrix3x3 operator-(const Vector3D&, const Matrix3x3&);
// Matrix3x3 operator*(const Vector3D&, const Matrix3x3&);
// Matrix3x3 operator/(const Vector3D&, const Matrix3x3&);

Matrix3x3 operator+(const Matrix3x3&, decimal);
Matrix3x3 operator-(const Matrix3x3&, decimal);
Matrix3x3 operator*(const Matrix3x3&, decimal);
Matrix3x3 operator/(const Matrix3x3&, decimal);

Matrix3x3 operator+(decimal, const Matrix3x3&);
Matrix3x3 operator-(decimal, const Matrix3x3&);
Matrix3x3 operator*(decimal, const Matrix3x3&);
Matrix3x3 operator/(decimal, const Matrix3x3&);

// ============================================================================
// ============================================================================
//  Printing
// ============================================================================
// ============================================================================
std::ostream& operator<<(std::ostream&, const Matrix3x3&);
