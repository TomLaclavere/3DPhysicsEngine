#include "mathematics/matrix.hpp"

#include <cmath>
#include <ostream>
#include <stdexcept>

// ============================================================================
// ============================================================================
//  Utilities
// ============================================================================
// ============================================================================
void Matrix3x3::absolute()
{
    for (auto& row : m)
    {
        row[0] = std::abs(row[0]);
        row[1] = std::abs(row[1]);
        row[2] = std::abs(row[2]);
    }
}
void Matrix3x3::normalize()
{
    // Copy rows
    Vector3D r0 = m[0];
    Vector3D r1 = m[1];
    Vector3D r2 = m[2];

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
    m[0] = r0;
    m[1] = r1;
    m[2] = r2;
}
void Matrix3x3::transpose()
{
    std::swap(m[0][1], m[1][0]);
    std::swap(m[0][2], m[2][0]);
    std::swap(m[1][2], m[2][1]);
}
void Matrix3x3::inverse()
{
    decimal det = getDeterminant();
    if (commonMaths::approxEqual(det, decimal(0)))
        throw std::runtime_error("Matrix is singular and cannot be inverted");
    decimal invDet = decimal(1) / det;

    Matrix3x3 inv;
    inv(0, 0) = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
    inv(0, 1) = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet;
    inv(0, 2) = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;

    inv(1, 0) = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet;
    inv(1, 1) = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
    inv(1, 2) = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet;

    inv(2, 0) = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
    inv(2, 1) = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet;
    inv(2, 2) = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

    *this = inv;
}
decimal Matrix3x3::getMinValue() const
{
    return std::min({ m[0].getMinValue(), m[1].getMinValue(), m[2].getMinValue() });
}
decimal Matrix3x3::getMaxValue() const
{
    return std::max({ m[0].getMaxValue(), m[1].getMaxValue(), m[2].getMaxValue() });
}
decimal Matrix3x3::getDeterminant() const
{
    return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[0][2] * m[1][0] * m[2][1] -
           m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
}
decimal   Matrix3x3::getTrace() const { return m[0][0] + m[1][1] + m[2][2]; }
Matrix3x3 Matrix3x3::getIdentity() const
{
    Matrix3x3 I = { Matrix3x3(*this) };
    I.setToIdentity();
    return I;
}
Matrix3x3 Matrix3x3::getAbsolute() const
{
    return Matrix3x3(m[0].getAbsoluteVector(), m[1].getAbsoluteVector(), m[2].getAbsoluteVector());
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
// ============================================================================
//  Setters
// ============================================================================
// ============================================================================
void Matrix3x3::setRow(int index, const Vector3D& row) { m[index] = row; }
void Matrix3x3::setColumn(int index, const Vector3D& column)
{
    for (int i = 0; i < 3; ++i)
        m[i][index] = column[i];
}
void Matrix3x3::setDiagonal(const Vector3D& diagonal)
{
    for (int i = 0; i < 3; ++i)
        m[i][i] = diagonal[i];
}
void Matrix3x3::setToIdentity()
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] = (i == j) ? 1 : 0;
}
void Matrix3x3::setToZero()
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] = 0;
}
void Matrix3x3::setAllValues(decimal value)
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] = value;
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
    m[0][0] = m11;
    m[0][1] = m12;
    m[0][2] = m13;
    m[1][0] = m21;
    m[1][1] = m22;
    m[1][2] = m23;
    m[2][0] = m31;
    m[2][1] = m32;
    m[2][2] = m33;
}
void Matrix3x3::setAllValues(const Matrix3x3& m)
{
    for (int i = 0; i < 3; ++i)
        (*this).setRow(i, m[i]);
}

// ============================================================================
// ============================================================================
//  Property Checks
// ============================================================================
// ============================================================================
bool Matrix3x3::isIdentity() const
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (i == j)
            {
                if (!commonMaths::approxEqual(m[i][j], decimal(1)))
                    return false;
            }
            else
            {
                if (!commonMaths::approxEqual(m[i][j], decimal(0)))
                    return false;
            }
    return true;
}
bool Matrix3x3::isZero() const
{
    for (int i = 0; i < 3; ++i)
        if (!m[i].isZero())
            return false;
    return true;
}
bool Matrix3x3::isFinite() const
{
    for (int i = 0; i < 3; ++i)
        if (!m[i].isFinite())
            return false;
    return true;
}
bool Matrix3x3::isDiagonal() const
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (i != j && !commonMaths::approxEqual(m[i][j], decimal(0)))
                return false;
    return true;
}
bool Matrix3x3::isSymmetric() const
{
    for (int i = 1; i < 3; ++i)
        for (int j = 0; j < i; ++j)
            if (!commonMaths::approxEqual(m[i][j], m[j][i]))
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
// ============================================================================
//  Matrix Operations
// ============================================================================
// ============================================================================
Matrix3x3 Matrix3x3::matrixProduct(const Matrix3x3& matrix) const
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            result.m[i][j] = m[i].dotProduct(matrix.getColumn(j));
    return result;
}
Vector3D Matrix3x3::matrixVectorProduct(const Vector3D& vector) const
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = m[i].dotProduct(vector);
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
            result[i][j] = matrix1[i].dotProduct(matrix2.getColumn(j));
    return result;
}
Vector3D matrixVectorProduct(const Matrix3x3& matrix, const Vector3D& vector)
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = matrix[i].dotProduct(vector);
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
// ============================================================================
//  Comparison Operators
// ============================================================================
// ============================================================================
bool Matrix3x3::operator==(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] == matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator!=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (m[i] != matrix.m[i])
            return true;
    return false;
}
bool Matrix3x3::operator<(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] < matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator<=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] <= matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator>(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] > matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::operator>=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] >= matrix.m[i]))
            return false;
    return true;
}
bool Matrix3x3::approxEqual(const Matrix3x3& matrix, decimal tolerance) const
{
    for (int i = 0; i < 3; ++i)
        if (!m[i].approxEqual(matrix.m[i], tolerance))
            return false;
    return true;
}

// ============================================================================
// ============================================================================
//  Element Access Operators
// ============================================================================
// ============================================================================
decimal& Matrix3x3::at(int row, int column)
{
    if (row < 0 || row >= 3)
        throw std::out_of_range("Matrix3x3 row index out of range");
    return m[row](column);
}
decimal Matrix3x3::at(int row, int column) const
{
    if (row < 0 || row >= 3)
        throw std::out_of_range("Matrix3x3 row index out of range");
    return m[row](column);
}
Vector3D& Matrix3x3::at(int row)
{
    if (row < 0 || row >= 3)
        throw std::out_of_range("Matrix3x3 row index out of range");
    return m[row];
}
Vector3D Matrix3x3::at(int row) const
{
    if (row < 0 || row >= 3)
        throw std::out_of_range("Matrix3x3 row index out of range");
    return m[row];
}
decimal&  Matrix3x3::operator()(int row, int column) { return m[row][column]; }
decimal   Matrix3x3::operator()(int row, int column) const { return m[row][column]; }
Vector3D& Matrix3x3::operator()(int row) { return m[row]; }
Vector3D  Matrix3x3::operator()(int row) const { return m[row]; }
Vector3D& Matrix3x3::operator[](int row) { return m[row]; }
Vector3D  Matrix3x3::operator[](int row) const { return m[row]; }

// ============================================================================
// ============================================================================
//  In-Place Arithmetic Operators
// ============================================================================
// ============================================================================
Matrix3x3& Matrix3x3::operator-()
{
    for (auto& row : m)
        row = -row;
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& matrix)
{
    m[0] += matrix[0];
    m[1] += matrix[1];
    m[2] += matrix[2];
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& matrix)
{
    m[0] -= matrix[0];
    m[1] -= matrix[1];
    m[2] -= matrix[2];
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& matrix)
{
    m[0] *= matrix[0];
    m[1] *= matrix[1];
    m[2] *= matrix[2];
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(const Matrix3x3& matrix)
{
    m[0] /= matrix[0];
    m[1] /= matrix[1];
    m[2] /= matrix[2];
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(const Vector3D& vector)
{
    m[0] += vector;
    m[1] += vector;
    m[2] += vector;
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(const Vector3D& vector)
{
    m[0] -= vector;
    m[1] -= vector;
    m[2] -= vector;
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(const Vector3D& vector)
{
    m[0] *= vector;
    m[1] *= vector;
    m[2] *= vector;
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(const Vector3D& vector)
{
    m[0] /= vector;
    m[1] /= vector;
    m[2] /= vector;
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(decimal scalar)
{
    m[0] += scalar;
    m[1] += scalar;
    m[2] += scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(decimal scalar)
{
    m[0] -= scalar;
    m[1] -= scalar;
    m[2] -= scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(decimal scalar)
{
    m[0] *= scalar;
    m[1] *= scalar;
    m[2] *= scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(decimal scalar)
{
    if (scalar == 0)
        throw std::invalid_argument("Division by zero");
    m[0] /= scalar;
    m[1] /= scalar;
    m[2] /= scalar;
    return *this;
}

// ============================================================================
// ============================================================================
//  Free Arithmetic Operators
// ============================================================================
// ============================================================================
// ===== Matrix3x3 op Matrix3x3 (element-wise) =====
Matrix3x3 operator+(const Matrix3x3& A, const Matrix3x3& B)
{
    return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a + b; });
}
Matrix3x3 operator-(const Matrix3x3& A, const Matrix3x3& B)
{
    return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a - b; });
}
Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B)
{
    return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a * b; });
}
Matrix3x3 operator/(const Matrix3x3& A, const Matrix3x3& B)
{
    return applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a / b; });
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
    return applyMatrix(A, s, [](const Vector3D& a, decimal s) { return a + s; });
}
Matrix3x3 operator-(const Matrix3x3& A, decimal s)
{
    return applyMatrix(A, s, [](const Vector3D& a, decimal s) { return a - s; });
}
Matrix3x3 operator*(const Matrix3x3& A, decimal s)
{
    return applyMatrix(A, s, [](const Vector3D& a, decimal s) { return a * s; });
}
Matrix3x3 operator/(const Matrix3x3& A, decimal s)
{
    if (s == 0)
        throw std::invalid_argument("Division by zero");
    return applyMatrix(A, s, [](const Vector3D& a, decimal s) { return a / s; });
}

// ===== decimal op Matrix3x3 =====
Matrix3x3 operator+(decimal s, const Matrix3x3& A)
{
    return applyMatrix(A, s, [](const Vector3D& a, decimal s) { return a + s; });
}
Matrix3x3 operator-(decimal s, const Matrix3x3& A)
{
    return applyMatrix(A, s,
                       [s](const Vector3D& a, decimal) { return Vector3D(s - a[0], s - a[1], s - a[2]); });
}
Matrix3x3 operator*(decimal s, const Matrix3x3& A)
{
    return applyMatrix(A, s, [](const Vector3D& a, decimal s) { return a * s; });
}
Matrix3x3 operator/(decimal s, const Matrix3x3& A)
{
    return applyMatrix(A, s,
                       [s](const Vector3D& a, decimal) { return Vector3D(s / a[0], s / a[1], s / a[2]); });
}

// ============================================================================
// ============================================================================
//  Printing =====
// ============================================================================
// ============================================================================
std::ostream& operator<<(std::ostream& os, const Matrix3x3& m)
{
    os << "Matrix3x3(" << m[0] << ", " << m[1] << ", " << m[2] << ")";
    return os;
}
