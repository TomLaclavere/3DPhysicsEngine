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
    EXPECT_DECIMAL_EQ(m(0, 1), 2);
    EXPECT_DECIMAL_EQ(m(0, 2), 3);
    EXPECT_DECIMAL_EQ(m(1, 0), 4);
    EXPECT_DECIMAL_EQ(m(1, 1), 5);
    EXPECT_DECIMAL_EQ(m(1, 2), 6);
    EXPECT_DECIMAL_EQ(m(2, 0), 7);
    EXPECT_DECIMAL_EQ(m(2, 1), 8);
    EXPECT_DECIMAL_EQ(m(2, 2), 9);
}

TEST(Matrix3x3_Test, RowConstructor)
{
    Vector3D  r1(1, 2, 3), r2(4, 5, 6), r3(7, 8, 9);
    Matrix3x3 m(r1, r2, r3);
    EXPECT_EQ(m.getRow(0), r1);
    EXPECT_EQ(m.getRow(1), r2);
    EXPECT_EQ(m.getRow(2), r3);
}

TEST(Matrix3x3_Test, CopyConstructor)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 m2(m);
    EXPECT_EQ(m, m2);
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
    EXPECT_DECIMAL_EQ(m(0, 1), 2);
    EXPECT_DECIMAL_EQ(m(0, 2), 3);
    EXPECT_DECIMAL_EQ(m(1, 0), 4);
    EXPECT_DECIMAL_EQ(m(1, 1), 5);
    EXPECT_DECIMAL_EQ(m(1, 2), 6);
    EXPECT_DECIMAL_EQ(m(2, 0), 7);
    EXPECT_DECIMAL_EQ(m(2, 1), 8);
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
// 4) Matrix Operations
// ——————————————————————————————————————————————————————————————————————————

TEST(Matrix3x3_Test, MatrixProduct)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3x3 prod     = a.matrixProduct(b);
    Matrix3x3 prod_alt = matrixProduct(a, b);
    EXPECT_DECIMAL_EQ(prod(0, 0), 30);
    EXPECT_DECIMAL_EQ(prod_alt(0, 0), 30);
    EXPECT_DECIMAL_EQ(prod(0, 1), 24);
    EXPECT_DECIMAL_EQ(prod_alt(0, 1), 24);
    EXPECT_DECIMAL_EQ(prod(0, 2), 18);
    EXPECT_DECIMAL_EQ(prod_alt(0, 2), 18);
    EXPECT_DECIMAL_EQ(prod(1, 0), 84);
    EXPECT_DECIMAL_EQ(prod_alt(1, 0), 84);
    EXPECT_DECIMAL_EQ(prod(1, 1), 69);
    EXPECT_DECIMAL_EQ(prod_alt(1, 1), 69);
    EXPECT_DECIMAL_EQ(prod(1, 2), 54);
    EXPECT_DECIMAL_EQ(prod_alt(1, 2), 54);
    EXPECT_DECIMAL_EQ(prod(2, 0), 138);
    EXPECT_DECIMAL_EQ(prod_alt(2, 0), 138);
    EXPECT_DECIMAL_EQ(prod(2, 1), 114);
    EXPECT_DECIMAL_EQ(prod_alt(2, 1), 114);
    EXPECT_DECIMAL_EQ(prod(2, 2), 90);
    EXPECT_DECIMAL_EQ(prod_alt(2, 2), 90);
}

TEST(Matrix3x3_Test, MatrixVectorProduct)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 2, 3);
    Vector3D  prod     = a.matrixVectorProduct(v);
    Vector3D  prod_alt = matrixVectorProduct(a, v);
    EXPECT_DECIMAL_EQ(prod[0], 14);
    EXPECT_DECIMAL_EQ(prod_alt[0], 14);
    EXPECT_DECIMAL_EQ(prod[1], 32);
    EXPECT_DECIMAL_EQ(prod_alt[1], 32);
    EXPECT_DECIMAL_EQ(prod[2], 50);
    EXPECT_DECIMAL_EQ(prod_alt[2], 50);
}

TEST(Matrix3x3_Test, VectorMatrixProduct)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 2, 3);
    Vector3D  prod     = a.vectorMatrixProduct(v);
    Vector3D  prod_alt = vectorMatrixProduct(v, a);
    EXPECT_DECIMAL_EQ(prod[0], 30);
    EXPECT_DECIMAL_EQ(prod_alt[0], 30);
    EXPECT_DECIMAL_EQ(prod[1], 36);
    EXPECT_DECIMAL_EQ(prod_alt[1], 36);
    EXPECT_DECIMAL_EQ(prod[2], 42);
    EXPECT_DECIMAL_EQ(prod_alt[2], 42);
}

// ——————————————————————————————————————————————————————————————————————————
// 5) Comparison Operators
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

TEST(Matrix3x3_Test, LessThan)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3x3 c(10, 10, 10, 10, 10, 10, 10, 10, 10);
    EXPECT_FALSE(a < b);
    EXPECT_FALSE(b < a);
    EXPECT_FALSE(a < a);
    EXPECT_TRUE(a < c);
}

TEST(Matrix3x3_Test, LessThanOrEqual)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3x3 c(10, 10, 10, 10, 10, 10, 10, 10, 10);
    EXPECT_FALSE(a <= b);
    EXPECT_FALSE(b <= a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(a <= c);
}

TEST(Matrix3x3_Test, MoreThan)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3x3 c(10, 10, 10, 10, 10, 10, 10, 10, 10);
    EXPECT_FALSE(b > a);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a > a);
    EXPECT_TRUE(c > a);
}

TEST(Matrix3x3_Test, MoreThanOrEqual)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3x3 c(10, 10, 10, 10, 10, 10, 10, 10, 10);
    EXPECT_FALSE(b >= a);
    EXPECT_FALSE(a >= b);
    EXPECT_TRUE(a >= a);
    EXPECT_TRUE(c >= a);
}

// ——————————————————————————————————————————————————————————————————————————
// 6) Eleement Access
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, ElementAccessParentheses)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);

    // Test const and non-const access
    EXPECT_DECIMAL_EQ(m(0, 0), 1);
    EXPECT_DECIMAL_EQ(m(1, 2), 6);

    m(2, 1) = 42.0;
    EXPECT_DECIMAL_EQ(m(2, 1), 42.0);

    // Test row access
    Vector3D row = m(1);
    EXPECT_EQ(row, Vector3D(4, 5, 6));
    m(1) = Vector3D(10, 11, 12);
    EXPECT_EQ(m.getRow(1), Vector3D(10, 11, 12));
}

TEST(Matrix3x3_Test, ElementAccessBrackets)
{
    Matrix3x3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);

    // Test const and non-const access
    EXPECT_EQ(m[0], Vector3D(1, 2, 3));
    EXPECT_EQ(m[2], Vector3D(7, 8, 9));

    m[1] = Vector3D(20, 21, 22);
    EXPECT_EQ(m.getRow(1), Vector3D(20, 21, 22));
}

// ——————————————————————————————————————————————————————————————————————————
// 7) In-Place Arithmetic Operators (element-wise and scalar)
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, InPlaceArithmeticOperators_Matrix)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);

    Matrix3x3 c = a;
    c += b;
    EXPECT_EQ(c, Matrix3x3(10, 10, 10, 10, 10, 10, 10, 10, 10));

    c = a;
    c -= b;
    EXPECT_EQ(c, Matrix3x3(-8, -6, -4, -2, 0, 2, 4, 6, 8));

    c = a;
    c *= b;
    EXPECT_EQ(c, Matrix3x3(9, 16, 21, 24, 25, 24, 21, 16, 9));

    c = a;
    c /= b;
    EXPECT_EQ(c, Matrix3x3(1.0 / 9, 2.0 / 8, 3.0 / 7, 4.0 / 6, 1, 6.0 / 4, 7.0 / 3, 8.0 / 2, 9.0 / 1));
}

TEST(Matrix3x3_Test, InPlaceArithmeticOperators_Vector)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 2, 3);

    Matrix3x3 c = a;
    c += v;
    EXPECT_EQ(c, Matrix3x3(2, 4, 6, 5, 7, 9, 8, 10, 12));

    c = a;
    c -= v;
    EXPECT_EQ(c, Matrix3x3(0, 0, 0, 3, 3, 3, 6, 6, 6));

    c = a;
    c *= v;
    EXPECT_EQ(c, Matrix3x3(1, 4, 9, 4, 10, 18, 7, 16, 27));

    c = a;
    c /= v;
    EXPECT_EQ(c, Matrix3x3(1, 1, 1, 4, 2.5, 2, 7, 4, 3));
}

TEST(Matrix3x3_Test, InPlaceArithmeticOperators_Scalar)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);

    Matrix3x3 c = a;
    c += 2.0;
    EXPECT_EQ(c, Matrix3x3(3, 4, 5, 6, 7, 8, 9, 10, 11));

    c = a;
    c -= 1.0;
    EXPECT_EQ(c, Matrix3x3(0, 1, 2, 3, 4, 5, 6, 7, 8));

    c = a;
    c *= 2.0;
    EXPECT_EQ(c, Matrix3x3(2, 4, 6, 8, 10, 12, 14, 16, 18));

    c = a;
    c /= 2.0;
    EXPECT_EQ(c, Matrix3x3(0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5));
}

// ——————————————————————————————————————————————————————————————————————————
// 8) Free Arithmetic Operators (element-wise and scalar)
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, FreeArithmeticOperators_Matrix)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 b(9, 8, 7, 6, 5, 4, 3, 2, 1);

    EXPECT_EQ(a + b, Matrix3x3(10, 10, 10, 10, 10, 10, 10, 10, 10));
    EXPECT_EQ(a - b, Matrix3x3(-8, -6, -4, -2, 0, 2, 4, 6, 8));
    EXPECT_EQ(a * b, Matrix3x3(9, 16, 21, 24, 25, 24, 21, 16, 9));
    EXPECT_EQ(a / b, Matrix3x3(1.0 / 9, 2.0 / 8, 3.0 / 7, 4.0 / 6, 1, 6.0 / 4, 7.0 / 3, 8.0 / 2, 9.0 / 1));
}

TEST(Matrix3x3_Test, FreeArithmeticOperators_MatrixVector)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 2, 3);

    EXPECT_EQ(a + v, Matrix3x3(2, 4, 6, 5, 7, 9, 8, 10, 12));
    EXPECT_EQ(a - v, Matrix3x3(0, 0, 0, 3, 3, 3, 6, 6, 6));
    EXPECT_EQ(a * v, Matrix3x3(1, 4, 9, 4, 10, 18, 7, 16, 27));
    EXPECT_EQ(a / v, Matrix3x3(1, 1, 1, 4, 2.5, 2, 7, 4, 3));
}

TEST(Matrix3x3_Test, FreeArithmeticOperators_VectorMatrix)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3D  v(1, 2, 3);

    EXPECT_EQ(v + a, Matrix3x3(2, 4, 6, 5, 7, 9, 8, 10, 12));
    EXPECT_EQ(v - a, Matrix3x3(0, 0, 0, -3, -3, -3, -6, -6, -6));
    EXPECT_EQ(v * a, Matrix3x3(1, 4, 9, 4, 10, 18, 7, 16, 27));
    EXPECT_EQ(v / a,
              Matrix3x3(1.0 / 1, 2.0 / 2, 3.0 / 3, 1.0 / 4, 2.0 / 5, 3.0 / 6, 1.0 / 7, 2.0 / 8, 3.0 / 9));
}

TEST(Matrix3x3_Test, FreeArithmeticOperators_MatrixScalar)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);

    EXPECT_EQ(a + 2.0, Matrix3x3(3, 4, 5, 6, 7, 8, 9, 10, 11));
    EXPECT_EQ(a - 1.0, Matrix3x3(0, 1, 2, 3, 4, 5, 6, 7, 8));
    EXPECT_EQ(a * 2.0, Matrix3x3(2, 4, 6, 8, 10, 12, 14, 16, 18));
    EXPECT_EQ(a / 2.0, Matrix3x3(0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5));
}

TEST(Matrix3x3_Test, FreeArithmeticOperators_ScalarMatrix)
{
    Matrix3x3 a(1, 2, 3, 4, 5, 6, 7, 8, 9);

    EXPECT_EQ(2.0 + a, Matrix3x3(3, 4, 5, 6, 7, 8, 9, 10, 11));
    EXPECT_EQ(1.0 - a, Matrix3x3(0, -1, -2, -3, -4, -5, -6, -7, -8));
    EXPECT_EQ(2.0 * a, Matrix3x3(2, 4, 6, 8, 10, 12, 14, 16, 18));
    EXPECT_EQ(2.0 / a,
              Matrix3x3(2.0 / 1, 2.0 / 2, 2.0 / 3, 2.0 / 4, 2.0 / 5, 2.0 / 6, 2.0 / 7, 2.0 / 8, 2.0 / 9));
}

// ——————————————————————————————————————————————————————————————————————————
// 9) Stream Output
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, StreamOutput)
{
    Matrix3x3         m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    std::stringstream ss;
    ss << m[0] << " " << m[1] << " " << m[2];
    EXPECT_EQ(ss.str(), "(1,2,3) (4,5,6) (7,8,9)");
}
