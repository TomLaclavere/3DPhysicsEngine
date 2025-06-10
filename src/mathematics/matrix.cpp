#include "mathematics/matrix.hpp"

#include <cmath>
#include <ostream>

Matrix3x3::Matrix3x3(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23,
                     decimal m31, decimal m32, decimal m33)
    : m11(m11)
    , m12(m12)
    , m13(m13)
    , m21(m21)
    , m22(m22)
    , m23(m23)
    , m31(m31)
    , m32(m32)
    , m33(m33)
{}
Matrix3x3::Matrix3x3()
    : Matrix3x3(0, 0, 0, 0, 0, 0, 0, 0, 0)
{}
Matrix3x3::Matrix3x3(decimal value)
    : Matrix3x3(value, value, value, value, value, value, value, value, value)
{}

Matrix3x3::Matrix3x3(const Vector3D& row1, const Vector3D& row2, const Vector3D& row3)
    : Matrix3x3(row1.getX(), row1.getY(), row1.getZ(), row2.getX(), row2.getY(), row2.getZ(), row3.getX(),
                row3.getY(), row3.getZ())
{}

Vector3D Matrix3x3::getRow(int index) const
{
    switch (index)
    {
    case 0:
        return Vector3D(m11, m12, m13);
    case 1:
        return Vector3D(m21, m22, m23);
    case 2:
        return Vector3D(m31, m32, m33);
    default:
        throw std::out_of_range("Row index must be 0, 1, or 2");
    }
}

Vector3D Matrix3x3::getColumn(int index) const
{
    switch (index)
    {
    case 0:
        return Vector3D(m11, m21, m31);
    case 1:
        return Vector3D(m12, m22, m32);
    case 2:
        return Vector3D(m13, m23, m33);
    default:
        throw std::out_of_range("Column index must be 0, 1, or 2");
    }
}

Vector3D Matrix3x3::getDiagonal() const { return Vector3D(m11, m22, m33); }

decimal Matrix3x3::getDeterminant() const
{
    return m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
}

Matrix3x3 Matrix3x3::getTranspose() const { return Matrix3x3(m11, m21, m31, m12, m22, m32, m13, m23, m33); }

Matrix3x3 Matrix3x3::getAbsolute() const
{
    return Matrix3x3(std::fabs(m11), std::fabs(m12), std::fabs(m13), std::fabs(m21), std::fabs(m22),
                     std::fabs(m23), std::fabs(m31), std::fabs(m32), std::fabs(m33));
}

bool Matrix3x3::isIdentity() const
{
    return m11 == 1 && m22 == 1 && m33 == 1 && m12 == 0 && m13 == 0 && m21 == 0 && m23 == 0 && m31 == 0 &&
           m32 == 0;
}

bool Matrix3x3::isZero() const
{
    return m11 == 0 && m12 == 0 && m13 == 0 && m21 == 0 && m22 == 0 && m23 == 0 && m31 == 0 && m32 == 0 &&
           m33 == 0;
}

bool Matrix3x3::isFinite() const
{
    return std::isfinite(m11) && std::isfinite(m12) && std::isfinite(m13) && std::isfinite(m21) &&
           std::isfinite(m22) && std::isfinite(m23) && std::isfinite(m31) && std::isfinite(m32) &&
           std::isfinite(m33);
}

bool Matrix3x3::isInvertible() const { return std::fabs(getDeterminant()) > PRECISION_MACHINE; }

bool Matrix3x3::isOrthogonal() const
{
    Matrix3x3 transposed = getTranspose();
    Matrix3x3 identity   = (*this) * transposed;
    return identity.isIdentity();
}

bool Matrix3x3::isDiagonal() const
{
    return m12 == 0 && m13 == 0 && m21 == 0 && m23 == 0 && m31 == 0 && m32 == 0;
}

bool Matrix3x3::isSymmetric() const { return m12 == m21 && m13 == m31 && m23 == m32; }

decimal& Matrix3x3::operator()(int row, int column) { return *(&m11 + row * 3 + column); }
decimal Matrix3x3::operator()(int row, int column) const { return *(&m11 + row * 3 + column); }
Vector3D Matrix3x3::operator[](int index) const
{
    switch (index)
    {
    case 0:
        return Vector3D(m11, m12, m13);
    case 1:
        return Vector3D(m21, m22, m23);
    case 2:
        return Vector3D(m31, m32, m33);
    default:
        throw std::out_of_range("Index must be 0, 1, or 2");
    }
}

Matrix3x3 Matrix3x3::getInverse() const
{
    decimal det = getDeterminant();
    if (std::fabs(det) < PRECISION_MACHINE)
        throw std::runtime_error("Matrix not invertible");

    return Matrix3x3(
        (m22 * m33 - m23 * m32) / det, (m13 * m32 - m12 * m33) / det, (m12 * m23 - m13 * m22) / det,

        (m23 * m31 - m21 * m33) / det, (m11 * m33 - m13 * m31) / det, (m13 * m21 - m11 * m23) / det,

        (m21 * m32 - m22 * m31) / det, (m12 * m31 - m11 * m32) / det, (m11 * m22 - m12 * m21) / det);
}

decimal Matrix3x3::getTrace() const { return m11 + m22 + m33; }

void Matrix3x3::setRow(int index, const Vector3D& row)
{
    switch (index)
    {
    case 0:
        m11 = row.getX();
        m12 = row.getY();
        m13 = row.getZ();
        break;
    case 1:
        m21 = row.getX();
        m22 = row.getY();
        m23 = row.getZ();
        break;
    case 2:
        m31 = row.getX();
        m32 = row.getY();
        m33 = row.getZ();
        break;
    default:
        throw std::out_of_range("Row index must be 0, 1, or 2");
    }
}

void Matrix3x3::setColumn(int index, const Vector3D& column)
{
    switch (index)
    {
    case 0:
        m11 = column.getX();
        m21 = column.getY();
        m31 = column.getZ();
        break;
    case 1:
        m12 = column.getX();
        m22 = column.getY();
        m32 = column.getZ();
        break;
    case 2:
        m13 = column.getX();
        m23 = column.getY();
        m33 = column.getZ();
        break;
    default:
        throw std::out_of_range("Column index must be 0, 1, or 2");
    }
}

void Matrix3x3::setDiagonal(const Vector3D& diagonal)
{
    m11 = diagonal.getX();
    m22 = diagonal.getY();
    m33 = diagonal.getZ();
}

void Matrix3x3::setToIdentity()
{
    m11 = m22 = m33 = 1;
    m12 = m13 = m21 = m23 = m31 = m32 = 0;
}

void Matrix3x3::setToZero() { m11 = m12 = m13 = m21 = m22 = m23 = m31 = m32 = m33 = 0; }

void Matrix3x3::setAllValues(decimal value) { m11 = m12 = m13 = m21 = m22 = m23 = m31 = m32 = m33 = value; }

void Matrix3x3::setAllValues(decimal m11_, decimal m12_, decimal m13_, decimal m21_, decimal m22_,
                             decimal m23_, decimal m31_, decimal m32_, decimal m33_)
{
    m11 = m11_;
    m12 = m12_;
    m13 = m13_;
    m21 = m21_;
    m22 = m22_;
    m23 = m23_;
    m31 = m31_;
    m32 = m32_;
    m33 = m33_;
}

Matrix3x3& Matrix3x3::operator+=(const decimal& value)
{
    m11 += value;
    m12 += value;
    m13 += value;
    m21 += value;
    m22 += value;
    m23 += value;
    m31 += value;
    m32 += value;
    m33 += value;
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(const Vector3D& vector)
{
    m11 += vector.getX();
    m12 += vector.getY();
    m13 += vector.getZ();
    m21 += vector.getX();
    m22 += vector.getY();
    m23 += vector.getZ();
    m31 += vector.getX();
    m32 += vector.getY();
    m33 += vector.getZ();
    return *this;
}
Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& matrix)
{
    m11 += matrix.m11;
    m12 += matrix.m12;
    m13 += matrix.m13;
    m21 += matrix.m21;
    m22 += matrix.m22;
    m23 += matrix.m23;
    m31 += matrix.m31;
    m32 += matrix.m32;
    m33 += matrix.m33;
    return *this;
}
Matrix3x3& Matrix3x3::operator-=(const decimal& value) { return *this += -value; }
Matrix3x3& Matrix3x3::operator-=(const Vector3D& vector) { return *this += -vector; }
Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& matrix) { return *this += -matrix; }
Matrix3x3& Matrix3x3::operator*=(const decimal& value)
{
    m11 *= value;
    m12 *= value;
    m13 *= value;
    m21 *= value;
    m22 *= value;
    m23 *= value;
    m31 *= value;
    m32 *= value;
    m33 *= value;
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(const Vector3D& vector)
{
    m11 *= vector.getX();
    m12 *= vector.getY();
    m13 *= vector.getZ();
    m21 *= vector.getX();
    m22 *= vector.getY();
    m23 *= vector.getZ();
    m31 *= vector.getX();
    m32 *= vector.getY();
    m33 *= vector.getZ();
    return *this;
}
Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& matrix)
{
    decimal m11_ = m11 * matrix.m11 + m12 * matrix.m21 + m13 * matrix.m31;
    decimal m12_ = m11 * matrix.m12 + m12 * matrix.m22 + m13 * matrix.m32;
    decimal m13_ = m11 * matrix.m13 + m12 * matrix.m23 + m13 * matrix.m33;
    decimal m21_ = m21 * matrix.m11 + m22 * matrix.m21 + m23 * matrix.m31;
    decimal m22_ = m21 * matrix.m12 + m22 * matrix.m22 + m23 * matrix.m32;
    decimal m23_ = m21 * matrix.m13 + m22 * matrix.m23 + m23 * matrix.m33;
    decimal m31_ = m31 * matrix.m11 + m32 * matrix.m21 + m33 * matrix.m31;
    decimal m32_ = m31 * matrix.m12 + m32 * matrix.m22 + m33 * matrix.m32;
    decimal m33_ = m31 * matrix.m13 + m32 * matrix.m23 + m33 * matrix.m33;
    m11          = m11_;
    m12          = m12_;
    m13          = m13_;
    m21          = m21_;
    m22          = m22_;
    m23          = m23_;
    m31          = m31_;
    m32          = m32_;
    m33          = m33_;
    return *this;
}
Matrix3x3& Matrix3x3::operator/=(const decimal& value)
{
    if (value == 0)
    {
        throw std::runtime_error("Division by zero");
    }
    return *this *= (1.0 / value);
}
Matrix3x3& Matrix3x3::operator/=(const Vector3D& vector)
{
    if (vector.getX() == 0 || vector.getY() == 0 || vector.getZ() == 0)
    {
        throw std::runtime_error("Division by zero");
    }
    return *this *= (1.0 / vector);
}

Matrix3x3 Matrix3x3::min(const Matrix3x3& matrix1, const Matrix3x3& matrix2) const
{
    return Matrix3x3(std::min(matrix1.m11, matrix2.m11), std::min(matrix1.m12, matrix2.m12),
                     std::min(matrix1.m13, matrix2.m13), std::min(matrix1.m21, matrix2.m21),
                     std::min(matrix1.m22, matrix2.m22), std::min(matrix1.m23, matrix2.m23),
                     std::min(matrix1.m31, matrix2.m31), std::min(matrix1.m32, matrix2.m32),
                     std::min(matrix1.m33, matrix2.m33));
}
Matrix3x3 Matrix3x3::max(const Matrix3x3& matrix1, const Matrix3x3& matrix2) const
{
    return Matrix3x3(std::max(matrix1.m11, matrix2.m11), std::max(matrix1.m12, matrix2.m12),
                     std::max(matrix1.m13, matrix2.m13), std::max(matrix1.m21, matrix2.m21),
                     std::max(matrix1.m22, matrix2.m22), std::max(matrix1.m23, matrix2.m23),
                     std::max(matrix1.m31, matrix2.m31), std::max(matrix1.m32, matrix2.m32),
                     std::max(matrix1.m33, matrix2.m33));
}

Matrix3x3 operator+(const Matrix3x3& matrix, const decimal& value)
{
    return Matrix3x3(matrix.m11 + value, matrix.m12 + value, matrix.m13 + value, matrix.m21 + value,
                     matrix.m22 + value, matrix.m23 + value, matrix.m31 + value, matrix.m32 + value,
                     matrix.m33 + value);
}
Matrix3x3 operator+(const Matrix3x3& value, const decimal matrix) { return matrix + value; }
Matrix3x3 operator+(const Matrix3x3& matrix, const Vector3D& vector)
{
    return Matrix3x3(matrix.m11 + vector.getX(), matrix.m12 + vector.getY(), matrix.m13 + vector.getZ(),
                     matrix.m21 + vector.getX(), matrix.m22 + vector.getY(), matrix.m23 + vector.getZ(),
                     matrix.m31 + vector.getX(), matrix.m32 + vector.getY(), matrix.m33 + vector.getZ());
}
Matrix3x3 operator+(const Vector3D& vector, const Matrix3x3& matrix) { return matrix + vector; }
Matrix3x3 operator+(const Matrix3x3& matrix1, const Matrix3x3& matrix2)
{
    return Matrix3x3(matrix1.m11 + matrix2.m11, matrix1.m12 + matrix2.m12, matrix1.m13 + matrix2.m13,
                     matrix1.m21 + matrix2.m21, matrix1.m22 + matrix2.m22, matrix1.m23 + matrix2.m23,
                     matrix1.m31 + matrix2.m31, matrix1.m32 + matrix2.m32, matrix1.m33 + matrix2.m33);
}
Matrix3x3 operator-(const Matrix3x3& matrix) { return matrix * -1; }
Matrix3x3 operator-(const decimal& value, const Matrix3x3& matrix)
{
    return Matrix3x3(value - matrix.m11, value - matrix.m12, value - matrix.m13, value - matrix.m21,
                     value - matrix.m22, value - matrix.m23, value - matrix.m31, value - matrix.m32,
                     value - matrix.m33);
}
Matrix3x3 operator-(const decimal& value, const Matrix3x3& matrix) { return matrix - value; }
Matrix3x3 operator-(const Matrix3x3& matrix, const Vector3D& vector) { return matrix + (-vector); }
Matrix3x3 operator-(const Vector3D& vector, const Matrix3x3& matrix) { return matrix - vector; }
Matrix3x3 operator-(const Matrix3x3& matrix1, const Matrix3x3& matrix2) { return matrix1 + (-matrix2); }
Matrix3x3 operator*(const Matrix3x3& matrix, const decimal& value)
{
    return Matrix3x3(matrix.m11 * value, matrix.m12 * value, matrix.m13 * value, matrix.m21 * value,
                     matrix.m22 * value, matrix.m23 * value, matrix.m31 * value, matrix.m32 * value,
                     matrix.m33 * value);
}
Matrix3x3 operator*(const decimal& value, const Matrix3x3& matrix) { return matrix * value; }
Vector3D  operator*(const Matrix3x3& matrix, const Vector3D& vector)
{
    return Vector3D(matrix.m11 * vector.getX() + matrix.m12 * vector.getY() + matrix.m13 * vector.getZ(),
                    matrix.m21 * vector.getX() + matrix.m22 * vector.getY() + matrix.m23 * vector.getZ(),
                    matrix.m31 * vector.getX() + matrix.m32 * vector.getY() + matrix.m33 * vector.getZ());
}
Vector3D  operator*(const Vector3D& vector, const Matrix3x3& matrix) { return matrix * vector; }
Matrix3x3 operator*(const Matrix3x3& matrix1, const Matrix3x3& matrix2)
{
    decimal m11_ = matrix1.m11 * matrix2.m11 + matrix1.m12 * matrix2.m21 + matrix1.m13 * matrix2.m31;
    decimal m12_ = matrix1.m11 * matrix2.m12 + matrix1.m12 * matrix2.m22 + matrix1.m13 * matrix2.m32;
    decimal m13_ = matrix1.m11 * matrix2.m13 + matrix1.m12 * matrix2.m23 + matrix1.m13 * matrix2.m33;
    decimal m21_ = matrix1.m21 * matrix2.m11 + matrix1.m22 * matrix2.m21 + matrix1.m23 * matrix2.m31;
    decimal m22_ = matrix1.m21 * matrix2.m12 + matrix1.m22 * matrix2.m22 + matrix1.m23 * matrix2.m32;
    decimal m23_ = matrix1.m21 * matrix2.m13 + matrix1.m22 * matrix2.m23 + matrix1.m23 * matrix2.m33;
    decimal m31_ = matrix1.m31 * matrix2.m11 + matrix1.m32 * matrix2.m21 + matrix1.m33 * matrix2.m31;
    decimal m32_ = matrix1.m31 * matrix2.m12 + matrix1.m32 * matrix2.m22 + matrix1.m33 * matrix2.m32;
    decimal m33_ = matrix1.m31 * matrix2.m13 + matrix1.m32 * matrix2.m23 + matrix1.m33 * matrix2.m33;
    return Matrix3x3(m11_, m12_, m13_, m21_, m22_, m23_, m31_, m32_, m33_);
}
Matrix3x3 operator/(const Matrix3x3& matrix, const decimal& value)
{
    if (value == 0)
    {
        throw std::invalid_argument("Division by zero");
    }
    return matrix * (1 / value);
}
Matrix3x3 operator/(const decimal& value, const Matrix3x3& matrix) { return matrix / value; }
Vector3D  operator/(const Matrix3x3& matrix, const Vector3D& vector) { return matrix * (1 / vector); }
Vector3D  operator/(const Vector3D& vector, const Matrix3x3& matrix) { return matrix / vector; }

std::ostream& operator<<(std::ostream& os, const Matrix3x3& matrix)
{
    os << "[" << matrix(0, 0) << ", " << matrix(0, 1) << ", " << matrix(0, 2) << "]\n"
       << "[" << matrix(1, 0) << ", " << matrix(1, 1) << ", " << matrix(1, 2) << "]\n"
       << "[" << matrix(2, 0) << ", " << matrix(2, 1) << ", " << matrix(2, 2) << "]";
    return os;
}
