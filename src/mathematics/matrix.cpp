#include "mathematics/matrix.hpp"

#include <cmath>
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
            if (m[i][j] != (i == j) ? 1 : 0)
            {
                return false;
            }
    return true;
}
