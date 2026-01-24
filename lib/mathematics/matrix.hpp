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
    constexpr Matrix3x3() = default;
    constexpr Matrix3x3(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23,
                        decimal m31, decimal m32, decimal m33) noexcept
        : m { m11, m12, m13, m21, m22, m23, m31, m32, m33 }
    {}
    constexpr explicit Matrix3x3(decimal value) noexcept
        : m { value, value, value, value, value, value, value, value, value }
    {}
    constexpr explicit Matrix3x3(const Vector3D& v) noexcept
        : m { v.getX(), v.getY(), v.getZ(), v.getX(), v.getY(), v.getZ(), v.getX(), v.getY(), v.getZ() }
    {}
    constexpr Matrix3x3(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3) noexcept
        : m { v1.getX(), v1.getY(), v1.getZ(), v2.getX(), v2.getY(),
              v2.getZ(), v3.getX(), v3.getY(), v3.getZ() }
    {}
    constexpr Matrix3x3(const Matrix3x3& _m)
        : m { _m.m }
    {} /// @}

    // ============================================================================
    /// @name Element Access Operators
    // ============================================================================
    /// @{

    /// Mapping from 2D indices to 1D index for a 3x3 matrix.
    std::size_t mapping(std::size_t ind_x, std::size_t ind_y) const;

    /// 2D matrix element with index range checking.
    decimal& at(std::size_t ind_x, std::size_t ind_y);
    /// 2D matrix element with index range checking (const version).
    decimal at(std::size_t ind_x, std::size_t ind_y) const;
    /// 2D matrix element without index range checking.
    decimal& operator()(std::size_t ind_x, std::size_t ind_y);
    /// 2D matrix element without index range checking (const version).
    decimal operator()(std::size_t ind_x, std::size_t ind_y) const;

    /// 1D matrix element with index range checking.
    decimal& at(std::size_t ind);
    /// 1D matrix element with index range checking (const version).
    decimal at(std::size_t ind) const;
    /// 1D matrix element without index range checking.
    constexpr decimal& operator()(std::size_t ind) noexcept { return m[ind]; }
    /// 1D matrix element without index range checking (const version).
    constexpr decimal operator()(std::size_t ind) const noexcept { return m[ind]; }
    /// 1D matrix element without index range checking.
    constexpr decimal& operator[](std::size_t ind) noexcept { return m[ind]; }
    /// 1D matrix element without index range checking (const version).
    constexpr decimal operator[](std::size_t ind) const noexcept { return m[ind]; }
    /// @}

    // ============================================================================
    /// @name Getters
    // ============================================================================
    /// @{
    Vector3D           getRow(std::size_t index) const;
    Vector3D           getColumn(std::size_t index) const;
    constexpr Vector3D getDiagonal() const noexcept { return Vector3D(m[0], m[4], m[8]); }
    /// @}

    // ============================================================================
    /// @name Utilities
    // ============================================================================
    /// @{

    /// Convert each element to its absolute value (in-place).
    constexpr void absolute() noexcept
    {
        for (std::size_t i = 0; i < 9; i++)
        {
            m[i] = commonMaths::absVal(m[i]);
        }
    }
    /// Normalise the matrix (in-place). If not invertible, becomes identity.
    void normalise();
    /// Transpose the matrix (in-place).
    void transpose();
    /// Invert the matrix (in-place). Throw `std::invalid_argument` if not invertible.
    void inverse();
    /// Return matrix determinant.
    constexpr decimal getDeterminant() const noexcept
    {
        return (*this)(0, 0) * (*this)(1, 1) * (*this)(2, 2) + (*this)(0, 1) * (*this)(1, 2) * (*this)(2, 0) +
               (*this)(0, 2) * (*this)(1, 0) * (*this)(2, 1) - (*this)(0, 2) * (*this)(1, 1) * (*this)(2, 0) -
               (*this)(0, 1) * (*this)(1, 0) * (*this)(2, 2) - (*this)(0, 0) * (*this)(1, 2) * (*this)(2, 1);
    }
    /// Return matrix trace (sum of diagonal elements).
    constexpr decimal getTrace() const noexcept { return (*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2); }
    /// Return identity matrix.
    Matrix3x3 getIdentity() const;
    /// Return a new matrix with element-wise absolute values.
    Matrix3x3 getAbsolute() const;
    /// Return a normalised copy of the matrix. If not invertible, return identity.
    Matrix3x3 getNormalised() const;
    /// Return the transposed matrix.
    Matrix3x3 getTranspose() const;
    /// Return the inverted matrix. Throw `std::invalid_argument` if not invertible.
    Matrix3x3 getInverse() const;
    /// @}

    // ============================================================================
    /// @name Setters
    // ============================================================================
    /// @{
    void           setRow(std::size_t, const Vector3D&);
    void           setColumn(std::size_t, const Vector3D&);
    constexpr void setDiagonal(const Vector3D& diagonal) noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            (*this)(i, i) = diagonal[i];
    }
    constexpr void setToIdentity() noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                (*this)(i, j) = (i == j) ? 1 : 0;
    }
    constexpr void setToNull() noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            (*this)(i) = 0;
    }
    constexpr void setAllValues(decimal value) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            (*this)(i) = value;
    }
    constexpr void setAllValues(const Vector3D& v) noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                (*this)(i, j) = v[j];
    }
    constexpr void setAllValues(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3) noexcept
    {
        (*this).setRow(0, v1);
        (*this).setRow(1, v2);
        (*this).setRow(2, v3);
    }
    constexpr void setAllValues(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23,
                                decimal m31, decimal m32, decimal m33) noexcept
    {
        (*this)(0) = m11;
        (*this)(1) = m12;
        (*this)(2) = m13;
        (*this)(3) = m21;
        (*this)(4) = m22;
        (*this)(5) = m23;
        (*this)(6) = m31;
        (*this)(7) = m32;
        (*this)(8) = m33;
    }
    constexpr void setAllValues(const Matrix3x3& other) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] = other.m[i];
    }

    /// @}

    // ============================================================================
    /// @name Property Checks
    // ============================================================================
    /// @{
    constexpr bool isIdentity() const noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                if (i == j)
                {
                    if (!commonMaths::approxEqual((*this)(i, j), decimal(1)))
                        return false;
                }
                else
                {
                    if (!commonMaths::approxEqual((*this)(i, j), decimal(0)))
                        return false;
                }
        return true;
    }
    constexpr bool isZero() const noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            if ((*this)(i) != 0)
                return false;
        return true;
    }
    constexpr bool isFinite() const noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            if (!commonMaths::isFinite((*this)(i)))
                return false;
        return true;
    }
    constexpr bool isDiagonal() const noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                if (i != j && !commonMaths::approxEqual((*this)(i, j), decimal(0)))
                    return false;
        return true;
    }
    constexpr bool isSymmetric() const noexcept
    {
        for (std::size_t i = 1; i < 3; ++i)
            for (std::size_t j = 0; j < i; ++j)
                if (!commonMaths::approxEqual((*this)(i, j), (*this)(j, i)))
                    return false;
        return true;
    }
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isNormalised() const;
    /// @}

    // ============================================================================
    /// @name Matrix Operations
    // ============================================================================
    /// @{

    constexpr Matrix3x3 matrixProduct(const Matrix3x3& matrix) const noexcept
    {
        Matrix3x3 result;
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                result(i, j) = (*this)(i, 0) * matrix(0, j) + (*this)(i, 1) * matrix(1, j) +
                               (*this)(i, 2) * matrix(2, j);
        return result;
    }
    constexpr Vector3D matrixVectorProduct(const Vector3D& vector) const noexcept
    {
        Vector3D result;
        for (std::size_t i = 0; i < 3; ++i)
            result[i] = (*this).getRow(i).dotProduct(vector);
        return result;
    }
    constexpr Vector3D vectorMatrixProduct(const Vector3D& vector) const noexcept
    {
        Vector3D result;
        for (std::size_t i = 0; i < 3; ++i)
            result[i] = vector.dotProduct(getColumn(i));
        return result;
    }
    /// @}

    /// Element-wise comparisons.
    // ============================================================================
    /// @name Comparisons Operators
    // ============================================================================
    /// @{
    constexpr Matrix3x3& operator=(const Matrix3x3&) = default;
    constexpr bool       operator==(const Matrix3x3& matrix) const noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            if (!(m[i] == matrix.m[i]))
                return false;
        return true;
    }
    constexpr bool operator!=(const Matrix3x3& matrix) const noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            if (m[i] != matrix.m[i])
                return true;
        return false;
    }
    constexpr bool approxEqual(const Matrix3x3& matrix, decimal precision = PRECISION_MACHINE) const noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            if (!commonMaths::approxEqual(m[i], matrix[i], precision))
                return false;
        return true;
    }
    /// @}

    /// Element-wise arithmetic operations.
    // ============================================================================
    /// @name In-Place Arithmetic Operators
    // ============================================================================
    /// @{

    constexpr Matrix3x3 operator-() const noexcept
    {
        Matrix3x3 result;
        for (std::size_t i = 0; i < 9; ++i)
            result[i] = -m[i];
        return result;
    }
    constexpr Matrix3x3& operator+=(const Matrix3x3& other) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] += other[i];
        return *this;
    }
    constexpr Matrix3x3& operator-=(const Matrix3x3& other) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] -= other[i];
        return *this;
    }
    constexpr Matrix3x3& operator*=(const Matrix3x3& other) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] *= other[i];
        return *this;
    }
    /// Element-wise division. Throw `std::invalid_argument` on division by zero
    Matrix3x3&           operator/=(const Matrix3x3&);
    constexpr Matrix3x3& operator+=(const Vector3D& vector) noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                (*this)(i, j) += vector[j];
        return *this;
    }
    constexpr Matrix3x3& operator-=(const Vector3D& vector) noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                (*this)(i, j) -= vector[j];
        return *this;
    }
    constexpr Matrix3x3& operator*=(const Vector3D& vector) noexcept
    {
        for (std::size_t i = 0; i < 3; ++i)
            for (std::size_t j = 0; j < 3; ++j)
                (*this)(i, j) *= vector[j];
        return *this;
    }
    /// Element-wise division. Throw `std::invalid_argument` on division by zero
    Matrix3x3&           operator/=(const Vector3D& vector);
    constexpr Matrix3x3& operator+=(decimal scalar) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] += scalar;
        return *this;
    }
    constexpr Matrix3x3& operator-=(decimal scalar) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] -= scalar;
        return *this;
    }
    constexpr Matrix3x3& operator*=(decimal scalar) noexcept
    {
        for (std::size_t i = 0; i < 9; ++i)
            m[i] *= scalar;
        return *this;
    }
    /// Element-wise division. Throw `std::invalid_argument` on division by zero
    Matrix3x3& operator/=(decimal value);
    /// @}
};

/// @addtogroup MatrixMaths
/// @{

// ============================================================================
/// @name Matrix Operations
// ============================================================================
/// @{
constexpr Matrix3x3 matrixProduct(const Matrix3x3& matrix1, const Matrix3x3& matrix2) noexcept
{
    return matrix1.matrixProduct(matrix2);
}
constexpr Vector3D matrixVectorProduct(const Matrix3x3& matrix, const Vector3D& vector) noexcept
{
    return matrix.matrixVectorProduct(vector);
}
constexpr Vector3D vectorMatrixProduct(const Vector3D& vector, const Matrix3x3& matrix) noexcept
{
    return matrix.vectorMatrixProduct(vector);
} /// @}

// ============================================================================
/// @name Free Arithmetic Operators
// ============================================================================
/// @{
constexpr Matrix3x3 operator+(const Matrix3x3& A, const Matrix3x3& B) noexcept
{
    Matrix3x3 m(A);
    return m += B;
}
constexpr Matrix3x3 operator-(const Matrix3x3& A, const Matrix3x3& B) noexcept
{
    Matrix3x3 m(A);
    return m -= B;
}
constexpr Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B) noexcept
{
    Matrix3x3 m(A);
    return m *= B;
}
Matrix3x3 operator/(const Matrix3x3&, const Matrix3x3&);

constexpr Matrix3x3 operator+(const Matrix3x3& A, decimal s) noexcept
{
    Matrix3x3 m(A);
    return m += s;
}
constexpr Matrix3x3 operator-(const Matrix3x3& A, decimal s) noexcept
{
    Matrix3x3 m(A);
    return m -= s;
}
constexpr Matrix3x3 operator*(const Matrix3x3& A, decimal s) noexcept
{
    Matrix3x3 m(A);
    return m *= s;
}
Matrix3x3 operator/(const Matrix3x3&, decimal);

constexpr Matrix3x3 operator+(decimal s, const Matrix3x3& A) noexcept { return A + s; }
constexpr Matrix3x3 operator-(decimal s, const Matrix3x3& A) noexcept { return -A + s; }
constexpr Matrix3x3 operator*(decimal s, const Matrix3x3& A) noexcept { return A * s; }
Matrix3x3           operator/(decimal, const Matrix3x3&);
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
