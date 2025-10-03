/**
 * @file matrix.hpp
 * @brief 3×3 matrix class and operations for physics/mathematical computations.
 *
 * Part of the math foundation of the physics engine.
 *
 * Conventions:
 *  - Matrix is stored as (m11, m12, m13, m21, m22, m23, m31, m32, m33), with a 3D mapping.
 *  - Mapping in row-major order.
 *  - Determinant, trace, and normalization follow standard linear algebra definitions.
 *  - Indexing: (i, j) maps to i*3 + j.
 */
#pragma once
#include "precision.hpp"
#include "vector.hpp"

#include <array>

/**
 * @defgroup MatrixMaths
 * Utilities for 3×3 matrices. Contains Matrix3x3 class and related free functions.
 */

/**
 * @ingroup MatrixMaths
 * @brief 3×3 matrix class with basic linear algebra operations.
 *
 * Stored internally as `std::array<decimal, 9>` in row-major order.
 *
 * Example usage:
 * @code
 * Matrix3x3 M(1,0,0, 0,1,0, 0,0,1);  // Identity
 * Vector3D v(1,2,3);
 * Vector3D result = M.matrixVectorProduct(v);
 * @endcode
 */
struct Matrix3x3
{
private:
    std::array<decimal, 9> m { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

public:
    // ============================================================================
    /// @name Constructors
    // ============================================================================
    /// @{
    Matrix3x3() = default;
    Matrix3x3(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23, decimal m31,
              decimal m32, decimal m33)
        : m { m11, m12, m13, m21, m22, m23, m31, m32, m33 }
    {}
    explicit Matrix3x3(decimal value)
        : m { value, value, value, value, value, value, value, value, value }
    {}
    explicit Matrix3x3(const Vector3D& v)
        : m { v.getX(), v.getY(), v.getZ(), v.getX(), v.getY(), v.getZ(), v.getX(), v.getY(), v.getZ() }
    {}
    Matrix3x3(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3)
        : m { v1.getX(), v1.getY(), v1.getZ(), v2.getX(), v2.getY(),
              v2.getZ(), v3.getX(), v3.getY(), v3.getZ() }
    {}
    Matrix3x3(const Matrix3x3& m)
        : m { m.m }
    {}
    /// @}

    // ============================================================================
    /// @name Element Access Operators
    // ============================================================================
    /// @{

    /// Mapping from 2D indices to 1D index for a 3x3 matrix.
    int mapping(int ind_x, int ind_y) const;

    /// 2D matrix element with index range checking.
    decimal& at(int ind_x, int ind_y);
    /// 2D matrix element with index range checking (const version).
    decimal at(int ind_x, int ind_y) const;
    /// 2D matrix element without index range checking.
    decimal& operator()(int ind_x, int ind_y);
    /// 2D matrix element without index range checking (const version).
    decimal operator()(int ind_x, int ind_y) const;

    /// 1D matrix element with index range checking.
    decimal& at(int ind);
    /// 1D matrix element with index range checking (const version).
    decimal at(int ind) const;
    /// 1D matrix element without index range checking.
    decimal& operator()(int ind);
    /// 1D matrix element without index range checking (const version).
    decimal operator()(int ind) const;
    /// 1D matrix element without index range checking.
    decimal& operator[](int ind);
    /// 1D matrix element without index range checking (const version).
    decimal operator[](int ind) const;
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    Vector3D getRow(int index) const { return Vector3D(m[index * 3], m[index * 3 + 1], m[index * 3 + 2]); }
    Vector3D getColumn(int index) const { return Vector3D(m[index], m[index + 3], m[index + 6]); }
    Vector3D getDiagonal() const { return Vector3D(m[0], m[4], m[8]); }
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// Convert each element to its absolute value (in-place).
    void absolute();
    /// Normalize the matrix (in-place). If not invertible, becomes identity.
    void normalize();
    /// Transpose the matrix (in-place).
    void transpose();
    /// Invert the matrix (in-place). Throw `std::runtime_error` if not invertible.
    void inverse();
    /// Return matrix determinant.
    decimal getDeterminant() const;
    /// Return matrix trace (sum of diagonal elements).
    decimal getTrace() const;
    /// Return identity matrix.
    Matrix3x3 getIdentity() const;
    /// Return a new matrix with element-wise absolute values.
    Matrix3x3 getAbsolute() const;
    /// Return a normalized copy of the matrix. If not invertible, return identity.
    Matrix3x3 getNormalized() const;
    /// Return the transposed matrix.
    Matrix3x3 getTranspose() const;
    /// Return the inverted matrix. Throw `std::runtime_error` if not invertible.
    Matrix3x3 getInverse() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void setRow(int, const Vector3D&);
    void setColumn(int, const Vector3D&);
    void setDiagonal(const Vector3D&);
    void setToIdentity();
    void setToNull();
    void setAllValues(decimal);
    void setAllValues(const Vector3D&);
    void setAllValues(const Vector3D&, const Vector3D&, const Vector3D&);
    void setAllValues(decimal, decimal, decimal, decimal, decimal, decimal, decimal, decimal, decimal);
    void setAllValues(const Matrix3x3&);
    /// @}

    // ============================================================================
    /// @name Property Checks
    // ============================================================================
    /// @{
    bool isIdentity() const;
    bool isZero() const;
    bool isFinite() const;
    bool isDiagonal() const;
    bool isSymmetric() const;
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isNormalized() const;
    /// @}

    // ============================================================================
    /// @name Matrix Operations
    // ============================================================================
    /// @{

    Matrix3x3 matrixProduct(const Matrix3x3&) const;
    Vector3D  matrixVectorProduct(const Vector3D&) const;
    Vector3D  vectorMatrixProduct(const Vector3D&) const;
    /// @}

    /// Element-wise comparisons.
    // ============================================================================
    /// @name Comparisons Operators
    // ============================================================================
    /// @{
    bool operator==(const Matrix3x3&) const;
    bool operator!=(const Matrix3x3&) const;
    bool operator<(const Matrix3x3&) const;
    bool operator<=(const Matrix3x3&) const;
    bool operator>(const Matrix3x3&) const;
    bool operator>=(const Matrix3x3&) const;
    bool approxEqual(const Matrix3x3&, decimal precision = PRECISION_MACHINE) const;
    /// @}

    /// Element-wise arithmetic operations.
    // ============================================================================
    /// @name In-Place Arithmetic Operators
    // ============================================================================
    /// @{

    /// Negate each element of the matrix.
    Matrix3x3& operator-();
    Matrix3x3& operator+=(const Matrix3x3&);
    Matrix3x3& operator-=(const Matrix3x3&);
    Matrix3x3& operator*=(const Matrix3x3&);
    /// Element-wise division. Throw `std::invalid_argument` on division by zero
    Matrix3x3& operator/=(const Matrix3x3&);
    Matrix3x3& operator+=(const Vector3D&);
    Matrix3x3& operator-=(const Vector3D&);
    Matrix3x3& operator*=(const Vector3D&);
    /// Element-wise division. Throw `std::invalid_argument` on division by zero
    Matrix3x3& operator/=(const Vector3D&);
    Matrix3x3& operator+=(decimal value);
    Matrix3x3& operator-=(decimal value);
    Matrix3x3& operator*=(decimal value);
    /// Element-wise division. Throw `std::invalid_argument` on division by zero
    Matrix3x3& operator/=(decimal value);
    /// @}
};

/// @addtogroup MatrixMaths
/// @{

// ============================================================================
/// @name Element Access Operators
// ============================================================================
/// @{
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
/// @}

// ============================================================================
/// @name Matrix Operations
// ============================================================================
/// @{
Matrix3x3 matrixProduct(const Matrix3x3& matrix1, const Matrix3x3& matrix2);
Vector3D  matrixVectorProduct(const Matrix3x3& matrix, const Vector3D&);
Vector3D  vectorMatrixProduct(const Vector3D& vector, const Matrix3x3&);
/// @}

// ============================================================================
/// @name Free Arithmetic Operators
// ============================================================================
/// @{
Matrix3x3 operator+(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator-(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator*(const Matrix3x3&, const Matrix3x3&);
Matrix3x3 operator/(const Matrix3x3&, const Matrix3x3&);

Matrix3x3 operator+(const Matrix3x3&, decimal);
Matrix3x3 operator-(const Matrix3x3&, decimal);
Matrix3x3 operator*(const Matrix3x3&, decimal);
Matrix3x3 operator/(const Matrix3x3&, decimal);

Matrix3x3 operator+(decimal, const Matrix3x3&);
Matrix3x3 operator-(decimal, const Matrix3x3&);
Matrix3x3 operator*(decimal, const Matrix3x3&);
Matrix3x3 operator/(decimal, const Matrix3x3&);
/// @}

// ============================================================================
/// @name Printing
// ============================================================================
/// @{

/// Stream output operator for Matrix3x3.
std::ostream& operator<<(std::ostream&, const Matrix3x3&);
/// @}
/// @}
/// @}
