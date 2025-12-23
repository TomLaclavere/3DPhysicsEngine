#include "mathematics/matrix.hpp"

#include "mathematics/common.hpp"

#include <cmath>
// #include <ostream>
#include <stdexcept>

// Convention :
// Matrices are stored in row-major order.
// The element (i, j) (row i, column j) is at index [i * 3 + j] in the array m[9].

// ============================================================================
//  Element Access Operators
// ============================================================================
std::size_t Matrix3x3::mapping(std::size_t ind_x, std::size_t ind_y) const
{
    {
        if (ind_x >= 3 || ind_y >= 3)
        {
            throw std::out_of_range("Matrix3x3 indices out of range");
        }
        return ind_x * 3 + ind_y;
    }
}
// 2D element access
decimal& Matrix3x3::at(std::size_t ind_x, std::size_t ind_y)
{
    if (ind_x >= 3 || ind_y >= 3)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[mapping(ind_x, ind_y)];
}
decimal Matrix3x3::at(std::size_t ind_x, std::size_t ind_y) const
{
    if (ind_x >= 3 || ind_y >= 3)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[mapping(ind_x, ind_y)];
}
decimal& Matrix3x3::operator()(std::size_t ind_x, std::size_t ind_y) { return m[mapping(ind_x, ind_y)]; }
decimal Matrix3x3::operator()(std::size_t ind_x, std::size_t ind_y) const { return m[mapping(ind_x, ind_y)]; }

// 1D element access
decimal& Matrix3x3::at(std::size_t ind)
{
    if (ind >= 9)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[ind];
}
decimal Matrix3x3::at(std::size_t ind) const
{
    if (ind >= 9)
        throw std::out_of_range("Matrix3x3 index out of range");
    return m[ind];
}

// ============================================================================
//  Getters
// ============================================================================
Vector3D Matrix3x3::getRow(std::size_t index) const
{
    if (index >= 3)
        throw std::out_of_range("Matrix3x3 index out of range");
    return Vector3D(m[index * 3], m[index * 3 + 1], m[index * 3 + 2]);
}
Vector3D Matrix3x3::getColumn(std::size_t index) const
{
    if (index >= 3)
        throw std::out_of_range("Matrix3x3 index out of range");
    return Vector3D(m[index], m[index + 3], m[index + 6]);
}

// ============================================================================
//  Utilities
// ============================================================================
/// @brief Normalise the matrix using the Gram-Schmidt process on its rows. If the matrix is not invertible,
/// it is set to the identity matrix.
void Matrix3x3::normalise()
{
    // Copy rows
    Vector3D r0(m[0], m[1], m[2]);
    Vector3D r1(m[3], m[4], m[5]);
    Vector3D r2(m[6], m[7], m[8]);

    // Gram-Schmidt process (rows)
    // 1. Normalise first row
    r0.normalise();

    // 2. Make r1 orthogonal to r0, then normalise
    r1 = r1 - r0 * r1.dotProduct(r0);
    r1.normalise();

    // 3. Make r2 orthogonal to r0 and r1, then normalise
    r2 = r2 - r0 * r2.dotProduct(r0) - r1 * r2.dotProduct(r1);
    r2.normalise();

    // Modify matrix rows
    (*this).setRow(0, r0);
    (*this).setRow(1, r1);
    (*this).setRow(2, r2);
}
void Matrix3x3::transpose()
{
    std::swap((*this)(0, 1), (*this)(1, 0));
    std::swap((*this)(0, 2), (*this)(2, 0));
    std::swap((*this)(1, 2), (*this)(2, 1));
}
void Matrix3x3::inverse()
{
    decimal det = getDeterminant();
    if (commonMaths::approxEqual(det, decimal(0)))
        throw std::invalid_argument("Matrix is singular and cannot be inverted");
    decimal invDet = decimal(1) / det;

    Matrix3x3 inv;
    inv(0, 0) = ((*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1)) * invDet;
    inv(0, 1) = ((*this)(0, 2) * (*this)(2, 1) - (*this)(0, 1) * (*this)(2, 2)) * invDet;
    inv(0, 2) = ((*this)(0, 1) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 1)) * invDet;

    inv(1, 0) = ((*this)(1, 2) * (*this)(2, 0) - (*this)(1, 0) * (*this)(2, 2)) * invDet;
    inv(1, 1) = ((*this)(0, 0) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 0)) * invDet;
    inv(1, 2) = ((*this)(0, 2) * (*this)(1, 0) - (*this)(0, 0) * (*this)(1, 2)) * invDet;

    inv(2, 0) = ((*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0)) * invDet;
    inv(2, 1) = ((*this)(0, 1) * (*this)(2, 0) - (*this)(0, 0) * (*this)(2, 1)) * invDet;
    inv(2, 2) = ((*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0)) * invDet;

    *this = inv;
}
Matrix3x3 Matrix3x3::getIdentity() const
{
    Matrix3x3 I = { Matrix3x3(*this) };
    I.setToIdentity();
    return I;
}
Matrix3x3 Matrix3x3::getAbsolute() const
{
    Matrix3x3 normalisedM = Matrix3x3((*this));
    normalisedM.absolute();
    return normalisedM;
}
Matrix3x3 Matrix3x3::getNormalised() const
{
    Matrix3x3 normalisedM { Matrix3x3((*this)) };
    normalisedM.normalise();
    return normalisedM;
}
Matrix3x3 Matrix3x3::getTranspose() const
{
    Matrix3x3 transposeM = Matrix3x3((*this));
    transposeM.transpose();
    return transposeM;
}
Matrix3x3 Matrix3x3::getInverse() const
{
    Matrix3x3 inverseM = Matrix3x3((*this));
    inverseM.inverse();
    return inverseM;
}

// ============================================================================
//  Setters
// ============================================================================
void Matrix3x3::setRow(std::size_t index, const Vector3D& row)
{
    if (index >= 3)
        throw std::out_of_range("Matrix3x3 row index out of range");
    (*this)(index, 0) = row[0];
    (*this)(index, 1) = row[1];
    (*this)(index, 2) = row[2];
}
void Matrix3x3::setColumn(std::size_t index, const Vector3D& column)
{
    if (index >= 3)
        throw std::out_of_range("Matrix3x3 column index out of range");
    (*this)(0, index) = column[0];
    (*this)(1, index) = column[1];
    (*this)(2, index) = column[2];
}

// ============================================================================
//  Property Checks
// ============================================================================
bool Matrix3x3::isInvertible() const { return !commonMaths::approxEqual(getDeterminant(), decimal(0)); }
bool Matrix3x3::isOrthogonal() const { return (getTranspose().matrixProduct(*this)).isIdentity(); }
bool Matrix3x3::isNormalised() const
{
    // Verify norm of columns
    for (std::size_t i = 0; i < 3; ++i)
    {
        if (!commonMaths::approxEqual(getColumn(i).getNormSquare(), decimal(1)))
            return false;
    }

    // Verify orthogonality of columns
    for (std::size_t i = 0; i < 3; ++i)
    {
        for (std::size_t j = i + 1; j < 3; ++j)
        {
            if (std::abs(getColumn(i).dotProduct(getColumn(j))) > PRECISION_MACHINE)
                return false;
        }
    }

    // Verify determinant
    if (!commonMaths::approxEqual(getDeterminant(), decimal(1)))
        return false;

    return true;
}

// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
/**
 * Element-wise division by another matrix.
 * This is NOT standard matrix division (multiplication by the inverse).
 * Each element is divided by the corresponding element of the other matrix.
 * Throws `std::invalid_argument` on division by zero.
 */
Matrix3x3& Matrix3x3::operator/=(const Matrix3x3& other)
{
    // Check if the divisor matrix has a zero element
    for (std::size_t i = 0; i < 9; ++i)
    {
        if (commonMaths::approxEqual(other[i], decimal(0)))
        {
            throw std::invalid_argument("Division by zero element in matrix");
        }
    }

    // Perform element-wise division
    for (std::size_t i = 0; i < 9; ++i)
    {
        m[i] /= other.m[i];
    }
    return *this;
}

Matrix3x3& Matrix3x3::operator/=(const Vector3D& vector)
{
    // Check if the divisor vector has a zero element
    if (commonMaths::approxEqual(vector[0], decimal(0)) || commonMaths::approxEqual(vector[1], decimal(0)) ||
        commonMaths::approxEqual(vector[2], decimal(0)))
        throw std::invalid_argument("Division by zero");

    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            (*this)(i, j) /= vector[j];
    return *this;
}

Matrix3x3& Matrix3x3::operator/=(decimal scalar)
{
    if (commonMaths::approxEqual(scalar, decimal(0)))
        throw std::invalid_argument("Division by zero");
    decimal inv = 1 / scalar;
    for (std::size_t i = 0; i < 9; ++i)
        m[i] *= inv;
    return *this;
}

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ===== Matrix3x3 op Matrix3x3 (element-wise) =====
Matrix3x3 operator/(const Matrix3x3& A, const Matrix3x3& B)
{
    for (std::size_t i = 0; i < 9; ++i)
    {
        if (commonMaths::approxEqual(B[i], decimal(0)))
            throw std::invalid_argument("Division by zero element in matrix");
    }
    Matrix3x3 m(A);
    return m /= B;
}

// ===== Matrix3x3 op decimal =====
Matrix3x3 operator/(const Matrix3x3& A, decimal s)
{
    if (commonMaths::approxEqual(s, decimal(0)))
        throw std::invalid_argument("Division by zero");
    decimal   inv = 1 / s;
    Matrix3x3 m(A);
    return m *= inv;
}

// ===== decimal op Matrix3x3 =====
Matrix3x3 operator/(decimal s, const Matrix3x3& A)
{
    Matrix3x3 result;
    for (std::size_t i = 0; i < 9; ++i)
    {
        if (commonMaths::approxEqual(A[i], decimal(0)))
            throw std::invalid_argument("Division by zero");
        result[i] = s / A[i];
    }
    return result;
}

// ============================================================================
//  Printing =====
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Matrix3x3& m)
{
    os << "Matrix3x3:\n";
    os << m(0, 0) << ", " << m(0, 1) << ", " << m(0, 2) << "\n";
    os << m(1, 0) << ", " << m(1, 1) << ", " << m(1, 2) << "\n";
    os << m(2, 0) << ", " << m(2, 1) << ", " << m(2, 2);
    return os;
}
