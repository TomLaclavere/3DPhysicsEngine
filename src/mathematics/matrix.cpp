#include "mathematics/matrix.hpp"

#include <cmath>
#include <ostream>
#include <stdexcept>

// Convention :
// Matrices are stored in row-major order.
// The element (i, j) (row i, column j) is at index [i * 3 + j] in the array m[9].

// ============================================================================
//  Utilities
// ============================================================================
void Matrix3x3::absolute()
{
    for (int i = 0; i < 9; ++i)
    {
        m[i] = std::abs(m[i]);
    }
}

/// @brief Normalize the matrix using the Gram-Schmidt process on its rows. If the matrix is not invertible,
/// it is set to the identity matrix.
void Matrix3x3::normalize()
{
    // Copy rows
    Vector3D r0(m[0], m[1], m[2]);
    Vector3D r1(m[3], m[4], m[5]);
    Vector3D r2(m[6], m[7], m[8]);

    // Gram-Schmidt process (rows)
    // 1. Normalize first row
    r0.normalize();

    // 2. Make r1 orthogonal to r0, then normalize
    r1 = r1 - r0 * r1.dotProduct(r0);
    r1.normalize();

    // 3. Make r2 orthogonal to r0 and r1, then normalize
    r2 = r2 - r0 * r2.dotProduct(r0) - r1 * r2.dotProduct(r1);
    r2.normalize();

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
        throw std::runtime_error("Matrix is singular and cannot be inverted");
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
decimal Matrix3x3::getDeterminant() const
{
    return (*this)(0, 0) * (*this)(1, 1) * (*this)(2, 2) + (*this)(0, 1) * (*this)(1, 2) * (*this)(2, 0) +
           (*this)(0, 2) * (*this)(1, 0) * (*this)(2, 1) - (*this)(0, 2) * (*this)(1, 1) * (*this)(2, 0) -
           (*this)(0, 1) * (*this)(1, 0) * (*this)(2, 2) - (*this)(0, 0) * (*this)(1, 2) * (*this)(2, 1);
}
decimal   Matrix3x3::getTrace() const { return (*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2); }
Matrix3x3 Matrix3x3::getIdentity() const
{
    Matrix3x3 I = { Matrix3x3(*this) };
    I.setToIdentity();
    return I;
}
Matrix3x3 Matrix3x3::getAbsolute() const
{
    Matrix3x3 normalizedM = Matrix3x3((*this));
    normalizedM.absolute();
    return normalizedM;
}
Matrix3x3 Matrix3x3::getNormalized() const
{
    Matrix3x3 normalizedM { Matrix3x3((*this)) };
    normalizedM.normalize();
    return normalizedM;
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
void Matrix3x3::setRow(int index, const Vector3D& row)
{
    if (index < 0 || index >= 3)
        throw std::out_of_range("Matrix3x3 row index out of range");
    (*this)(index, 0) = row[0];
    (*this)(index, 1) = row[1];
    (*this)(index, 2) = row[2];
}
void Matrix3x3::setColumn(int index, const Vector3D& column)
{
    if (index < 0 || index >= 3)
        throw std::out_of_range("Matrix3x3 column index out of range");
    (*this)(0, index) = column[0];
    (*this)(1, index) = column[1];
    (*this)(2, index) = column[2];
}
void Matrix3x3::setDiagonal(const Vector3D& diagonal)
{
    for (int i = 0; i < 3; ++i)
        (*this)(i, i) = diagonal[i];
}
void Matrix3x3::setToIdentity()
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            (*this)(i, j) = (i == j) ? 1 : 0;
}
void Matrix3x3::setToZero()
{
    for (int i = 0; i < 9; ++i)
        (*this)(i) = 0;
}
void Matrix3x3::setAllValues(decimal value)
{
    for (int i = 0; i < 9; ++i)
        (*this)(i) = value;
}
void Matrix3x3::setAllValues(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3)
{
    (*this).setRow(0, v1);
    (*this).setRow(1, v2);
    (*this).setRow(2, v3);
}
void Matrix3x3::setAllValues(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23,
                             decimal m31, decimal m32, decimal m33)
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
void Matrix3x3::setAllValues(const Matrix3x3& other)
{
    for (int i = 0; i < 9; ++i)
        m[i] = other.m[i];
}

// ============================================================================
//  Property Checks
// ============================================================================
bool Matrix3x3::isIdentity() const
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
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
bool Matrix3x3::isZero() const
{
    for (int i = 0; i < 9; ++i)
        if ((*this)(i) != 0)
            return false;
    return true;
}
bool Matrix3x3::isFinite() const
{
    for (int i = 0; i < 9; ++i)
        if (!std::isfinite((*this)(i)))
            return false;
    return true;
}
bool Matrix3x3::isDiagonal() const
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (i != j && !commonMaths::approxEqual((*this)(i, j), decimal(0)))
                return false;
    return true;
}
bool Matrix3x3::isSymmetric() const
{
    for (int i = 1; i < 3; ++i)
        for (int j = 0; j < i; ++j)
            if (!commonMaths::approxEqual((*this)(i, j), (*this)(j, i)))
                return false;
    return true;
}
bool Matrix3x3::isInvertible() const { return !commonMaths::approxEqual(getDeterminant(), decimal(0)); }
bool Matrix3x3::isOrthogonal() const { return (getTranspose().matrixProduct(*this)).isIdentity(); }
bool Matrix3x3::isNormalized() const
{
    // Verify norm of columns
    for (int i = 0; i < 3; ++i)
    {
        if (!commonMaths::approxEqual(getColumn(i).getNormSquare(), decimal(1)))
            return false;
    }

    // Verify orthogonality of columns
    for (int i = 0; i < 3; ++i)
    {
        for (int j = i + 1; j < 3; ++j)
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
//  Matrix Operations
// ============================================================================
Matrix3x3 Matrix3x3::matrixProduct(const Matrix3x3& matrix) const
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            result(i, j) =
                (*this)(i, 0) * matrix(0, j) + (*this)(i, 1) * matrix(1, j) + (*this)(i, 2) * matrix(2, j);
    return result;
}
Vector3D Matrix3x3::matrixVectorProduct(const Vector3D& vector) const
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = (*this).getRow(i).dotProduct(vector);
    return result;
}
Vector3D Matrix3x3::vectorMatrixProduct(const Vector3D& vector) const
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = vector.dotProduct(getColumn(i));
    return result;
}
Matrix3x3 matrixProduct(const Matrix3x3& matrix1, const Matrix3x3& matrix2)
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            result(i, j) = matrix1.getRow(i).dotProduct(matrix2.getColumn(j));
    return result;
}
Vector3D matrixVectorProduct(const Matrix3x3& matrix, const Vector3D& vector)
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = matrix.getRow(i).dotProduct(vector);
    return result;
}
Vector3D vectorMatrixProduct(const Vector3D& vector, const Matrix3x3& matrix)
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = vector.dotProduct(matrix.getColumn(i));
    return result;
}

// ============================================================================
//  Comparison Operators
// ============================================================================
bool Matrix3x3::operator==(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 9; ++i)
        if (!(m[i] == matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator!=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 9; ++i)
        if (m[i] != matrix.m[i])
            return true;
    return false;
}
bool Matrix3x3::operator<(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 9; ++i)
        if (!(m[i] < matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator<=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 9; ++i)
        if (!(m[i] <= matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator>(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 9; ++i)
        if (!(m[i] > matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator>=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 9; ++i)
        if (!(m[i] >= matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::approxEqual(const Matrix3x3& matrix, decimal tolerance) const
{
    for (int i = 0; i < 9; ++i)
        if (!commonMaths::approxEqual(m[i], matrix[i], tolerance))
            return false;
    return true;
}

// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
Matrix3x3& Matrix3x3::operator-()
{
    for (int i = 0; i < 9; ++i)
        m[i] = -m[i];
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& other)
{
    for (int i = 0; i < 9; ++i)
        m[i] += other[i];
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& other)
{
    for (int i = 0; i < 9; ++i)
        m[i] -= other[i];
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& other)
{
    for (int i = 0; i < 9; ++i)
        m[i] *= other[i];
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(const Matrix3x3& other)
{
    // Check if the divisor matrix is a zero matrix
    bool isZeroMatrix = true;
    for (int i = 0; i < 9; ++i)
    {
        if (other.m[i] != 0)
        {
            isZeroMatrix = false;
            break;
        }
    }

    if (isZeroMatrix)
    {
        throw std::invalid_argument("Division by zero matrix");
    }

    // Perform element-wise division
    for (int i = 0; i < 9; ++i)
    {
        m[i] /= other.m[i];
    }
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(decimal scalar)
{
    for (int i = 0; i < 9; ++i)
        m[i] += scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(decimal scalar)
{
    for (int i = 0; i < 9; ++i)
        m[i] -= scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(decimal scalar)
{
    for (int i = 0; i < 9; ++i)
        m[i] *= scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(decimal scalar)
{
    if (scalar == 0)
        throw std::invalid_argument("Division by zero");
    decimal inv = 1 / scalar;
    for (int i = 0; i < 9; ++i)
        m[i] *= inv;
    return *this;
}

// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ===== Matrix3x3 op Matrix3x3 (element-wise) =====
Matrix3x3 operator+(const Matrix3x3& A, const Matrix3x3& B)
{
    Matrix3x3 m(A);
    return m += B;
}
Matrix3x3 operator-(const Matrix3x3& A, const Matrix3x3& B)
{
    Matrix3x3 m(A);
    return m -= B;
}
Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B)
{
    Matrix3x3 m(A);
    return m *= B;
}
Matrix3x3 operator/(const Matrix3x3& A, const Matrix3x3& B)
{
    Matrix3x3 m(A);
    return m /= B;
}

//! Matrix3x3 Vector3D operations need to be redifiened if needed
// // ===== Matrix3x3 op Vector3D (element-wise, vector applied to each row) =====
// Matrix3x3 operator+(const Matrix3x3& A, const Vector3D& B)
// {
//     return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a + b; });
// }
// Matrix3x3 operator-(const Matrix3x3& A, const Vector3D& B)
// {
//     return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a - b; });
// }
// Matrix3x3 operator*(const Matrix3x3& A, const Vector3D& B)
// {
//     return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a * b; });
// }
// Matrix3x3 operator/(const Matrix3x3& A, const Vector3D& B)
// {
//     return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a / b; });
// }

// // ===== Vector3D op Matrix3x3 (element-wise, vector applied to each row) =====
// Matrix3x3 operator+(const Vector3D& A, const Matrix3x3& B)
// {
//     return applyMatrix(B, A, [](const Vector3D& a, const Vector3D& b) { return b + a; });
// }
// Matrix3x3 operator-(const Vector3D& A, const Matrix3x3& B)
// {
//     return applyMatrix(B, A, [](const Vector3D& a, const Vector3D& b) { return b - a; });
// }
// Matrix3x3 operator*(const Vector3D& A, const Matrix3x3& B)
// {
//     return applyMatrix(B, A, [](const Vector3D& a, const Vector3D& b) { return b * a; });
// }
// Matrix3x3 operator/(const Vector3D& A, const Matrix3x3& B)
// {
//     return applyMatrix(B, A, [](const Vector3D& a, const Vector3D& b) { return b / a; });
//}

// ===== Matrix3x3 op decimal =====
Matrix3x3 operator+(const Matrix3x3& A, decimal s)
{
    Matrix3x3 m(A);
    return m += s;
}
Matrix3x3 operator-(const Matrix3x3& A, decimal s)
{
    Matrix3x3 m(A);
    return m -= s;
}
Matrix3x3 operator*(const Matrix3x3& A, decimal s)
{
    Matrix3x3 m(A);
    return m *= s;
}
Matrix3x3 operator/(const Matrix3x3& A, decimal s)
{
    if (s == 0)
        throw std::invalid_argument("Division by zero");
    decimal   inv = 1 / s;
    Matrix3x3 m(A);
    return m *= inv;
}

// ===== decimal op Matrix3x3 =====
Matrix3x3 operator+(decimal s, const Matrix3x3& A) { return A + s; }
Matrix3x3 operator-(decimal s, const Matrix3x3& A) { return ((-1) * A) + s; }
Matrix3x3 operator*(decimal s, const Matrix3x3& A) { return A * s; }
Matrix3x3 operator/(decimal s, const Matrix3x3& A)
{
    Matrix3x3 result;
    for (int i = 0; i < 9; ++i)
    {
        if (A[i] == 0)
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
