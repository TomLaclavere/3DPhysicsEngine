#pragma once
#include "precision.hpp"
#include "vector.hpp"

class Matrix3x3
{
private:
    decimal m11, m12, m13;
    decimal m21, m22, m23;
    decimal m31, m32, m33;

public:
    // ===== Constructors =====
    Matrix3x3(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23, decimal m31,
              decimal m32, decimal m33);
    Matrix3x3();
    Matrix3x3(decimal value);
    Matrix3x3(const Vector3D& row1, const Vector3D& row2, const Vector3D& row3);

    // ===== Accessors =====
    Vector3D  getRow(int index) const;
    Vector3D  getColumn(int index) const;
    Vector3D  getDiagonal() const;
    decimal   getMinValue() const;
    decimal   getMaxValue() const;
    decimal   getDeterminant() const;
    Matrix3x3 getInverse() const;
    Matrix3x3 getTranspose() const;
    decimal   getTrace() const;
    Matrix3x3 getAbsolute() const;

    // ===== Mutators =====
    void setRow(int index, const Vector3D& row);
    void setColumn(int index, const Vector3D& column);
    void setDiagonal(const Vector3D& diagonal);
    void setToIdentity();
    void setToZero();
    void setAllValues(decimal value);
    void setAllValues(decimal m11, decimal m12, decimal m13, decimal m21, decimal m22, decimal m23,
                      decimal m31, decimal m32, decimal m33);

    // ===== Property Checks =====
    bool isIdentity() const;
    bool isZero() const;
    bool isFinite() const;
    bool isInvertible() const;
    bool isOrthogonal() const;
    bool isDiagonal() const;
    bool isSymmetric() const;

    // ===== Comparison Operators =====
    bool operator==(const Matrix3x3& matrix) const;
    bool operator!=(const Matrix3x3& matrix) const;
    bool operator<(const Matrix3x3& matrix) const;
    bool approxEqual(const Matrix3x3& matrix, decimal precision = PRECISION_MACHINE) const;

    // ===== Element Access =====
    decimal& operator()(int row, int column);
    decimal  operator()(int row, int column) const;
    Vector3D operator[](int index) const;

    // ===== Compound Assignment Operators =====
    Matrix3x3& operator+=(const decimal& value);
    Matrix3x3& operator+=(const Vector3D& vector);
    Matrix3x3& operator+=(const Matrix3x3& matrix);
    Matrix3x3& operator-=(const decimal& value);
    Matrix3x3& operator-=(const Vector3D& vector);
    Matrix3x3& operator-=(const Matrix3x3& matrix);
    Matrix3x3& operator*=(const decimal& value);
    Matrix3x3& operator*=(const Vector3D& vector);
    Matrix3x3& operator*=(const Matrix3x3& matrix);
    Matrix3x3& operator/=(const decimal& value);
    Matrix3x3& operator/=(const Vector3D& vector);

    // ===== Min/Max =====
    Matrix3x3 min(const Matrix3x3& matrix1, const Matrix3x3& matrix2) const;
    Matrix3x3 max(const Matrix3x3& matrix1, const Matrix3x3& matrix2) const;

    // ===== Non-member Arithmetic Operators =====
    friend Matrix3x3 operator+(const Matrix3x3& matrix, const decimal& value);
    friend Matrix3x3 operator+(const decimal& value, const Matrix3x3& matrix);
    friend Matrix3x3 operator+(const Matrix3x3& matrix, const Vector3D& vector);
    friend Matrix3x3 operator+(const Vector3D& vector, const Matrix3x3& matrix);
    friend Matrix3x3 operator+(const Matrix3x3& lhs, const Matrix3x3& rhs);
    friend Matrix3x3 operator-(const Matrix3x3& matrix);
    friend Matrix3x3 operator-(const Matrix3x3& matrix, const decimal& value);
    friend Matrix3x3 operator-(const decimal& value, const Matrix3x3& matrix);
    friend Matrix3x3 operator-(const Matrix3x3& lhs, const Matrix3x3& rhs);
    friend Matrix3x3 operator-(const Matrix3x3& matrix, const Vector3D& vector);
    friend Matrix3x3 operator-(const Vector3D& vector, const Matrix3x3& matrix);
    friend Matrix3x3 operator*(const Matrix3x3& matrix, const decimal& value);
    friend Matrix3x3 operator*(const decimal& value, const Matrix3x3& matrix);
    friend Matrix3x3 operator*(const Vector3D& vector, const Matrix3x3& matrix);
    friend Vector3D  operator*(const Matrix3x3& matrix, const Vector3D& vector);
    friend Matrix3x3 operator*(const Matrix3x3& lhs, const Matrix3x3& rhs);
    friend Matrix3x3 operator/(const decimal& value, const Matrix3x3& matrix);
    friend Matrix3x3 operator/(const Matrix3x3& matrix, const decimal& value);
    friend Matrix3x3 operator/(const Vector3D& vector, const Matrix3x3& matrix);
    friend Vector3D  operator/(const Matrix3x3& matrix, const Vector3D& vector);

    // ===== Printing function =====
    friend std::ostream& operator<<(std::ostream& os, const Matrix3x3& matrix);
};
