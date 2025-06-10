#include "mathematics/matrix.hpp"

#include <cmath>
#include <functional>
#include <ostream>

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
    if (det == 0)
    {
        return Matrix3x3();
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
    {
        m[i][i] = diagonal[i];
    }
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
                if (commonMaths::approxEqual(m[i][j], decimal(1)))
                {
                    return false;
                }
            }
            else
            {
                if (commonMaths::approxEqual(m[i][j], decimal(0)))
                {
                    return false;
                }
            }
    return true;
}
bool Matrix3x3::isZero() const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i].isZero()))
        {
            return false;
        }
    return true;
}
bool Matrix3x3::isFinite() const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i].isFinite()))
        {
            return false;
        }
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
        for (int j = 0; i < (j - 1); ++j)
            if (!commonMaths::approxEqual(m[i][j], m[j][i]))
            {
                return false;
            }
    return true;
}
bool Matrix3x3::isInvertible() const
{
    return !commonMaths::approxEqual(Matrix3x3::getDeterminant(), decimal(0));
}
bool Matrix3x3::isOrthogonal() const { return (getTranspose() * (*this)).isIdentity(); }

// ===== Comparison Operators =====
bool Matrix3x3::operator==(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] == matrix.m[i]))
        {
            return false;
        }
    return true;
}
bool Matrix3x3::operator!=(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] != matrix.m[i]))
        {
            return false;
        }
    return true;
}
bool Matrix3x3::operator<(const Matrix3x3& matrix) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i] < matrix.m[i]))
        {
            return false;
        }
    return true;
}
bool Matrix3x3::approxEqual(const Matrix3x3& matrix, decimal tolerance) const
{
    for (int i = 0; i < 3; ++i)
        if (!(m[i].approxEqual(matrix.m[i], tolerance)))
        {
            return false;
        }
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
Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& matrix) { return *this += (-matrix); }
Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& matrix)
{
    m[0] *= matrix[0];
    m[1] *= matrix[1];
    m[2] *= matrix[2];
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(const Matrix3x3& matrix) { return *this *= matrix.getInverse(); }
Matrix3x3& Matrix3x3::operator+=(decimal scalar)
{
    m[0] += scalar;
    m[1] += scalar;
    m[2] += scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(decimal scalar) { return *this += -scalar; }
Matrix3x3& Matrix3x3::operator*=(decimal scalar)
{
    m[0] *= scalar;
    m[1] *= scalar;
    m[2] *= scalar;
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(decimal scalar) { return *this *= decimal(1) / scalar; }

// ===== Helper for Free Arithmetic Operators =====
template <class F>
Matrix3x3 Matrix3x3::apply(const Matrix3x3& A, const Matrix3x3& B, F&& func)
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = func(A[i], B[i]);
    return result;
}
template <class F>
Matrix3x3 Matrix3x3::apply(const Matrix3x3& A, const Vector3D& B, F&& func)
{
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = func(A[i], B);
    return result;
}
template <class F>
Vector3D Matrix3x3::apply(const Matrix3x3& A, const Vector3D& B, F&& func)
{
    Vector3D result;
    for (int i = 0; i < 3; ++i)
        result[i] = func(A[i], B);
    return result;
}
template <class F>
Matrix3x3 Matrix3x3::apply(const Matrix3x3& A, decimal B, F&& func)
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
    return Matrix3x3::apply(A, B, std::plus<Vector3D>());
}
Matrix3x3 operator-(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::apply(A, B, std::minus<Vector3D>());
}
Matrix3x3 operator*(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::apply(A, B, std::multiplies<Vector3D>());
}
Matrix3x3 operator/(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3::apply(A, B, std::divides<Vector3D>());
}

// ===== Matrix3x3 op Vector3D (element-wise, vector applied to each row) =====
Matrix3x3 operator+(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::apply(A, B, std::plus<Vector3D>());
}
Matrix3x3 operator-(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::apply(A, B, std::minus<Vector3D>());
}
Matrix3x3 operator*(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::apply(A, B, std::multiplies<Vector3D>());
}
Matrix3x3 operator/(const Matrix3x3& A, const Vector3D& B)
{
    return Matrix3x3::apply(A, B, std::divides<Vector3D>());
}

// ===== Vector3D op Matrix3x3 (element-wise, vector applied to each row) =====
Matrix3x3 operator+(const Vector3D& A, const Matrix3x3& B)
{
    return Matrix3x3::apply(B, A, std::plus<Vector3D>());
}
Matrix3x3 operator-(const Vector3D& A, const Matrix3x3& B)
{
    // (vector - matrix) element-wise: each row of matrix subtracted from vector
    return Matrix3x3::apply(B, A, [](const Vector3D& row, const Vector3D& vec) { return vec - row; });
}
Matrix3x3 operator*(const Vector3D& A, const Matrix3x3& B)
{
    return Matrix3x3::apply(B, A, std::multiplies<Vector3D>());
}
Matrix3x3 operator/(const Vector3D& A, const Matrix3x3& B)
{
    // (vector / matrix) element-wise: each row of matrix divides vector
    return Matrix3x3::apply(B, A, [](const Vector3D& row, const Vector3D& vec) { return vec / row; });
}

// ===== Matrix3x3 op decimal =====
Matrix3x3 operator+(const Matrix3x3& A, decimal s) { return Matrix3x3::apply(A, s, std::plus<decimal>()); }
Matrix3x3 operator-(const Matrix3x3& A, decimal s) { return Matrix3x3::apply(A, s, std::minus<decimal>()); }
Matrix3x3 operator*(const Matrix3x3& A, decimal s)
{
    return Matrix3x3::apply(A, s, std::multiplies<decimal>());
}
Matrix3x3 operator/(const Matrix3x3& A, decimal s) { return Matrix3x3::apply(A, s, std::divides<decimal>()); }

// ===== decimal op Matrix3x3 =====
Matrix3x3 operator+(decimal s, const Matrix3x3& A) { return Matrix3x3::apply(A, s, std::plus<decimal>()); }
Matrix3x3 operator-(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::apply(A, s, [](decimal a, decimal b) { return b - a; });
}
Matrix3x3 operator*(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::apply(A, s, std::multiplies<decimal>());
}
Matrix3x3 operator/(decimal s, const Matrix3x3& A)
{
    return Matrix3x3::apply(A, s, [](decimal a, decimal b) { return b / a; });
}

// ===== Matrix-vector product (true matrix-vector multiplication) =====
Vector3D operator*(const Matrix3x3& M, const Vector3D& v)
{
    // Each row dot v
    return Vector3D(M.getRow(0).dotProduct(v), M.getRow(1).dotProduct(v), M.getRow(2).dotProduct(v));
}

// ===== Vector-matrix product (row vector times matrix) =====
Vector3D operator*(const Vector3D& v, const Matrix3x3& M)
{
    // Each column dot v
    return Vector3D(v.dotProduct(M.getColumn(0)), v.dotProduct(M.getColumn(1)), v.dotProduct(M.getColumn(2)));
}
