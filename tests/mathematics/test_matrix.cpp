#include "mathematics/common.hpp"
#include "mathematics/matrix.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

static void EXPECT_DECIMAL_EQ(decimal a, decimal b, decimal eps = PRECISION_MACHINE)
{
    EXPECT_TRUE(commonMaths::approxEqual(a, b, eps))
        << "Expected " << a << " ≈ " << b << "\nDifference: " << std::abs(a - b) << "\nPrecision: " << eps;
}

// ——————————————————————————————————————————————————————————————————————————
// 1) Constructors and getters
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, DefaultConstructorZeroes)
{
    Matrix3x3 m;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), 0.0);
}

TEST(Matrix3x3_Test, ValueConstructor)
{
    Matrix3x3 m(2.0);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), 2.0);
}

TEST(Matrix3x3_Test, FullValuesConstructor)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_DECIMAL_EQ(m(0, 0), 1);
    EXPECT_DECIMAL_EQ(m(1, 1), 5);
    EXPECT_DECIMAL_EQ(m(2, 2), 9);
    EXPECT_DECIMAL_EQ(m(2, 0), 7);
}

TEST(Matrix3x3_Test, RowConstructor)
{
    Vector3D  r1(1, 2, 3), r2(4, 5, 6), r3(7, 8, 9);
    Matrix3x3 m(r1, r2, r3);
    EXPECT_EQ(m.getRow(0), r1);
    EXPECT_EQ(m.getRow(1), r2);
    EXPECT_EQ(m.getRow(2), r3);
}

// ——————————————————————————————————————————————————————————————————————————
// 2) Accessors and Mutators
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, SetAndGetRowColumnDiagonal)
{
    Matrix3x3 m;
    Vector3D  r(1, 2, 3), c(4, 5, 6), d(7, 8, 9);
    m.setRow(0, r);
    EXPECT_EQ(m.getRow(0), r);
    m.setColumn(1, c);
    EXPECT_EQ(m.getColumn(1), c);
    m.setDiagonal(d);
    EXPECT_EQ(m.getDiagonal(), d);
}

TEST(Matrix3x3_Test, SetToIdentityAndZero)
{
    Matrix3x3 m;
    m.setToIdentity();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), (i == j) ? 1.0 : 0.0);

    m.setToZero();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), 0.0);
}

TEST(Matrix3x3_Test, SetAllValues)
{
    Matrix3x3 m;
    m.setAllValues(3.14);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), 3.14);

    m.setAllValues(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_DECIMAL_EQ(m(0, 0), 1);
    EXPECT_DECIMAL_EQ(m(2, 2), 9);
}

// ——————————————————————————————————————————————————————————————————————————
// 3) Property Checks
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, IsIdentity)
{
    Matrix3x3 m;
    m.setToIdentity();
    EXPECT_TRUE(m.isIdentity());
    m(0, 1) = 1.0;
    EXPECT_FALSE(m.isIdentity());
}

TEST(Matrix3x3_Test, IsZero)
{
    Matrix3x3 m;
    m.setToZero();
    EXPECT_TRUE(m.isZero());
    m(1, 1) = 1.0;
    EXPECT_FALSE(m.isZero());
}

TEST(Matrix3x3_Test, IsDiagonal)
{
    Matrix3x3 m;
    m.setToZero();
    m(0, 0) = 1.0;
    m(1, 1) = 2.0;
    m(2, 2) = 3.0;
    EXPECT_TRUE(m.isDiagonal());
    m(0, 1) = 5.0;
    EXPECT_FALSE(m.isDiagonal());
}

TEST(Matrix3x3_Test, IsSymmetric)
{
    Matrix3x3 m(1, 2, 3, 2, 4, 5, 3, 5, 6);
    EXPECT_TRUE(m.isSymmetric());
    m(0, 2) = 7.0;
    EXPECT_FALSE(m.isSymmetric());
}

TEST(Matrix3x3_Test, IsFinite)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_TRUE(m.isFinite());
    m(1, 1) = std::numeric_limits<decimal>::infinity();
    EXPECT_FALSE(m.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// 4) Arithmetic Operators (element-wise and scalar)
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, ElementWiseArithmetic)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3x3 sum = a + b;
    EXPECT_DECIMAL_EQ(sum(0, 0), 10);
    EXPECT_DECIMAL_EQ(sum(2, 2), 10);

    Matrix3x3 diff = a - b;
    EXPECT_DECIMAL_EQ(diff(0, 0), -8);
    EXPECT_DECIMAL_EQ(diff(2, 2), 8);

    Matrix3x3 prod = a * b;
    EXPECT_DECIMAL_EQ(prod(0, 0), 9);
    EXPECT_DECIMAL_EQ(prod(2, 2), 9);

    Matrix3x3 quot = a / b;
    EXPECT_DECIMAL_EQ(quot(0, 0), 1.0 / 9.0);
    EXPECT_DECIMAL_EQ(quot(2, 2), 9.0 / 1.0);
}

TEST(Matrix3x3_Test, ScalarArithmetic)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto      add = m + 1.0;
    EXPECT_DECIMAL_EQ(add(0, 0), 2.0);
    auto mul = 2.0 * m;
    EXPECT_DECIMAL_EQ(mul(1, 1), 10.0);
    auto sub = 10.0 - m;
    EXPECT_DECIMAL_EQ(sub(2, 2), 1.0);
    auto div = m / 2.0;
    EXPECT_DECIMAL_EQ(div(0, 1), 1.0);
}

// ——————————————————————————————————————————————————————————————————————————
// 5) Matrix-Vector and Vector-Matrix Product
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, MatrixVectorProduct)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 0, -1);
    Vector3D  result = m * v;
    EXPECT_DECIMAL_EQ(result.getX(), 1 * 1 + 2 * 0 + 3 * -1);
    EXPECT_DECIMAL_EQ(result.getY(), 4 * 1 + 5 * 0 + 6 * -1);
    EXPECT_DECIMAL_EQ(result.getZ(), 7 * 1 + 8 * 0 + 9 * -1);
}

TEST(Matrix3x3_Test, VectorMatrixProduct)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 2, 3);
    Vector3D  result = v * m;
    EXPECT_DECIMAL_EQ(result.getX(), 1 * 1 + 2 * 4 + 3 * 7);
    EXPECT_DECIMAL_EQ(result.getY(), 1 * 2 + 2 * 5 + 3 * 8);
    EXPECT_DECIMAL_EQ(result.getZ(), 1 * 3 + 2 * 6 + 3 * 9);
}

// ——————————————————————————————————————————————————————————————————————————
// 6) Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, EqualityAndApproxEqual)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 c(9, 8, 7, 6, 5, 4, 3, 2, 1);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a.approxEqual(b, 1e-10));
    EXPECT_FALSE(a.approxEqual(c, 1e-10));
}

// ——————————————————————————————————————————————————————————————————————————
// 7) Stream Output
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, StreamOutput)
{
    Matrix3x3         m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    std::stringstream ss;
    ss << m[0] << " " << m[1] << " " << m[2];
    EXPECT_EQ(ss.str(), "(1,2,3) (4,5,6) (7,8,9)");
}
