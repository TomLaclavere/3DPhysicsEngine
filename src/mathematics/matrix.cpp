#include "mathematics/matrix.hpp"

#include <cmath>
#include <ostream>
#include <stdexcept>

// ===== Utilities =====
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
Matrix3x3 Matrix3x3::getInverse() const
{
    decimal det = getDeterminant();
    if (commonMaths::approxEqual(det, decimal(0)))
    {
        throw std::runtime_error("Matrix is singular and cannot be inverted");
    }
    return Matrix3x3(m[1][1] * m[2][2] - m[1][2] * m[2][1], m[0][2] * m[2][1] - m[0][1] * m[2][2],
                     m[0][1] * m[1][2] - m[0][2] * m[1][1], m[1][2] * m[2][0] - m[1][0] * m[2][2],
                     m[0][0] * m[2][2] - m[0][2] * m[2][0], m[0][2] * m[1][0] - m[0][0] * m[1][2],
                     m[1][0] * m[2][1] - m[1][1] * m[2][0], m[0][1] * m[2][0] - m[0][0] * m[2][1],
                     m[0][0] * m[1][1] - m[0][1] * m[1][0]) /
           det;
}
Matrix3x3 Matrix3x3::getTranspose() const
{
    return Matrix3x3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]);
}
Matrix3x3 Matrix3x3::getAbsolute() const
{
    return Matrix3x3(m[0].getAbsoluteVector(), m[1].getAbsoluteVector(), m[2].getAbsoluteVector());
}

// ===== Setters =====
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

// ===== Property Checks =====
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

// ===== Matrix Operations =====
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
        result[i] = vector.dotProduct(m[i]);
    return result;
}

// ===== Comparison Operators =====
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

// ===== Element Access =====
decimal& Matrix3x3::operator()(int row, int column) { return m[row][column]; }
decimal Matrix3x3::operator()(int row, int column) const { return m[row][column]; }
Vector3D& Matrix3x3::operator()(int row) { return m[row]; }
Vector3D Matrix3x3::operator()(int row) const { return m[row]; }
Vector3D& Matrix3x3::operator[](int row) { return m[row]; }
Vector3D Matrix3x3::operator[](int row) const { return m[row]; }

// ===== In-Place Arithmetic Operators =====
Matrix3x3 Matrix3x3::operator-() const { return Matrix3x3(-m[0], -m[1], -m[2]); }
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

// ===== Helper for Free Arithmetic Operators =====
template <class F>
Matrix3x3 Matrix3x3::applyMatrix(const Matrix3x3& A, const Matrix3x3& B, F&& func)
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = func(A[i], B[i]);
    return result;
}
template <class F>
Matrix3x3 Matrix3x3::applyVector(const Matrix3x3& A, const Vector3D& B, F&& func)
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = func(A[i], B);
    return result;
}
template <class F>
Matrix3x3 Matrix3x3::applyDecimal(const Matrix3x3& A, decimal B, F&& func)
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = func(A[i], B);
    return result;
}

// ===== Free Arithmetic Operators =====
// ===== Matrix3x3 op Matrix3x3 (element-wise) =====
Matrix3x3 operator+(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a + b; });
}
Matrix3x3 operator-(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a - b; });
}
Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a * b; });
}
Matrix3x3 operator/(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::applyMatrix(A, B, [](const Vector3D& a, const Vector3D& b) { return a / b; });
}

// ===== Matrix3x3 op Vector3D (element-wise, vector applied to each row) =====
Matrix3x3 operator+(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::applyVector(A, B, [](const Vector3D& a, const Vector3D& b) { return a + b; });
}
Matrix3x3 operator-(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::applyVector(A, B, [](const Vector3D& a, const Vector3D& b) { return a - b; });
}
Matrix3x3 operator*(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::applyVector(A, B, [](const Vector3D& a, const Vector3D& b) { return a * b; });
}
Matrix3x3 operator/(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::applyVector(A, B, [](const Vector3D& a, const Vector3D& b) { return a / b; });
}

// ===== Vector3D op Matrix3x3 (element-wise, vector applied to each row) =====
Matrix3x3 operator+(const Vector3D& A, const Matrix3x3& B)
{
    return Matrix3x3::applyVector(B, A, [](const Vector3D& a, const Vector3D& b) { return b + a; });
}
Matrix3x3 operator-(const Vector3D& A, const Matrix3x3& B)
{
    return Matrix3x3::applyVector(B, A, [](const Vector3D& a, const Vector3D& b) { return b - a; });
}
Matrix3x3 operator*(const Vector3D& A, const Matrix3x3& B)
{
    return Matrix3x3::applyVector(B, A, [](const Vector3D& a, const Vector3D& b) { return b * a; });
}
Matrix3x3 operator/(const Vector3D& A, const Matrix3x3& B)
{
    return Matrix3x3::applyVector(B, A, [](const Vector3D& a, const Vector3D& b) { return b / a; });
}

// ===== Matrix3x3 op decimal =====
Matrix3x3 operator+(const Matrix3x3& A, decimal s)
{
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal s) { return a + s; });
}
Matrix3x3 operator-(const Matrix3x3& A, decimal s)
{
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal s) { return a - s; });
}
Matrix3x3 operator*(const Matrix3x3& A, decimal s)
{
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal s) { return a * s; });
}
Matrix3x3 operator/(const Matrix3x3& A, decimal s)
{
    if (s == 0)
        throw std::invalid_argument("Division by zero");
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal s) { return a / s; });
}

// ===== decimal op Matrix3x3 =====
Matrix3x3 operator+(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal s) { return a + s; });
}
Matrix3x3 operator-(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal b) { return b - a; });
}
Matrix3x3 operator*(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::applyDecimal(A, s, [](const Vector3D& a, decimal s) { return a * s; });
}
Matrix3x3 operator/(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::applyDecimal(A, s,
                                   [](const Vector3D& a, decimal b)
                                   {
                                       if (a == 0)
                                           throw std::invalid_argument("Division by zero");
                                       return b / a;
                                   });
}

// ===== Printing =====
std::ostream& operator<<(std::ostream& os, const Matrix3x3& m)
{
    os << "Matrix3x3(" << m[0] << ", " << m[1] << ", " << m[2] << ")";
    return os;
}
