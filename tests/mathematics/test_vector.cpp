#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"

#include <gtest/gtest.h>
#include <ostream>

static void EXPECT_DECIMAL_EQ(decimal a, decimal b, decimal eps = PRECISION_MACHINE)
{
    EXPECT_TRUE(commonMaths::approxEqual(a, b, eps))
        << "Expected " << a << " ≈ " << b << "\nDifference: " << std::abs(a - b) << "\nPrecision: " << eps;
}

// ——————————————————————————————————————————————————————————————————————————
// 1) Constructors and getters
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, DefaultConstructorZeroes)
{
    Vector3D v;
    EXPECT_DECIMAL_EQ(v.getX(), 1.0);
    EXPECT_DECIMAL_EQ(v.getY(), 0.0);
    EXPECT_DECIMAL_EQ(v.getZ(), 0.0);
}

TEST(Vector3D_Test, ValueConstructor)
{
    Vector3D v(1.1, -2.2, 3.3);
    EXPECT_DECIMAL_EQ(v.getX(), 1.1);
    EXPECT_DECIMAL_EQ(v.getY(), -2.2);
    EXPECT_DECIMAL_EQ(v.getZ(), 3.3);
}

// ——————————————————————————————————————————————————————————————————————————
// 2) Norm
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, NormAndSquare)
{
    Vector3D v(1, 2, 2);
    EXPECT_DECIMAL_EQ(v.getNormSquare(), 1 * 1 + 2 * 2 + 2 * 2);
    EXPECT_DECIMAL_EQ(v.getNorm(), 3.0); // sqrt(9)
}

// ——————————————————————————————————————————————————————————————————————————
// 3) Min/Max
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, MinMaxValue)
{
    Vector3D v(-5, 7, 2);
    EXPECT_DECIMAL_EQ(v.getMinValue(), -5);
    EXPECT_DECIMAL_EQ(v.getMaxValue(), 7);
}

// ——————————————————————————————————————————————————————————————————————————
// 4) Absolute and normalize
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, AbsoluteVector)
{
    Vector3D v(-1, -2, 3);
    auto     absV = v.getAbsoluteVector();
    EXPECT_DECIMAL_EQ(absV.getX(), 1);
    EXPECT_DECIMAL_EQ(absV.getY(), 2);
    EXPECT_DECIMAL_EQ(absV.getZ(), 3);
}

TEST(Vector3D_Test, NormalizeProducesUnit)
{
    Vector3D v(0, 3, 4);
    v.normalize();
    EXPECT_DECIMAL_EQ(v.getNorm(), 1.0);
    EXPECT_TRUE(v.isUnit());
}

// ——————————————————————————————————————————————————————————————————————————
// 5) isZero, isUnit, isLengthEqual, isFinite
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, IsZeroAndIsLengthEqual)
{
    Vector3D z;
    EXPECT_TRUE(z.isZero());
    EXPECT_TRUE(z.isLengthEqual(0.0));
    EXPECT_FALSE(z.isUnit());
}

TEST(Vector3D_Test, IsFiniteDetectsInfNaN)
{
    decimal  inf = std::numeric_limits<decimal>::infinity();
    Vector3D v(inf, 0, 0);
    EXPECT_FALSE(v.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// 6) dotProduct and crossProduct
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, DotProductOrthogonal)
{
    Vector3D x(1, 0, 0), y(0, 1, 0);
    EXPECT_DECIMAL_EQ(x.dotProduct(y), 0.0);
    EXPECT_DECIMAL_EQ(x.dotProduct(x), 1.0);
}

TEST(Vector3D_Test, CrossProductRightHanded)
{
    Vector3D x(1, 0, 0), y(0, 1, 0);
    auto     z = x.crossProduct(y);
    EXPECT_DECIMAL_EQ(z.getX(), 0.0);
    EXPECT_DECIMAL_EQ(z.getY(), 0.0);
    EXPECT_DECIMAL_EQ(z.getZ(), 1.0);
}

// ——————————————————————————————————————————————————————————————————————————
// 7) operators ==, !=, <, approxEqual
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, EqualityAndInequality)
{
    Vector3D a(1, 2, 3), b(1, 2, 3), c(3, 2, 1);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(Vector3D_Test, LessThanLexico)
{
    Vector3D a(1, 2, 3), b(1, 2, 4), c(0, 9, 9);
    EXPECT_TRUE(a < b); // same x,y but 3<4
    EXPECT_FALSE(b < a);
    EXPECT_FALSE(a < c); // 1<0 == false
}

TEST(Vector3D_Test, ApproxEqualWithPrecision)
{
    Vector3D a(1.0001, 2.0001, 3.0001);
    Vector3D b(1.0002, 2.0002, 3.0002);
    EXPECT_TRUE(a.approxEqual(b, 0.001));
    EXPECT_FALSE(a.approxEqual(b, 1e-5));
}

// ——————————————————————————————————————————————————————————————————————————
// 8) operator[] and exception on bad index
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, BracketOperatorReadWrite)
{
    Vector3D v(7, 8, 9);
    EXPECT_DECIMAL_EQ(v[0], 7);
    EXPECT_DECIMAL_EQ(v[1], 8);
    EXPECT_DECIMAL_EQ(v[2], 9);
    v[1] = 42;
    EXPECT_DECIMAL_EQ(v.getY(), 42);
}

TEST(Vector3D_Test, BracketOperatorThrowsOnInvalid)
{
    Vector3D v;
    EXPECT_THROW(v[3], std::invalid_argument);
    EXPECT_THROW(std::as_const(v)[-1], std::invalid_argument);
}

// ——————————————————————————————————————————————————————————————————————————
// 9) operators +=, -=, *=, /= and functions min/max
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, CompoundOperators)
{
    Vector3D v(1, 2, 3);
    v += Vector3D(1, 1, 1);
    EXPECT_DECIMAL_EQ(v.getX(), 2);
    v -= Vector3D(1, 2, 3);
    EXPECT_DECIMAL_EQ(v.getY(), 1); // 2-2
    v *= Vector3D(2, 3, 4);
    EXPECT_DECIMAL_EQ(v.getZ(), 4); // 1*4
    v /= Vector3D(2, 2, 2);
    EXPECT_DECIMAL_EQ(v.getX(), 1); // 2/2
}

TEST(Vector3D_Test, StaticMinMaxFunctions)
{
    Vector3D a(1, 5, 3), b(2, 4, 6);
    auto     mn = a.min(a, b);
    auto     mx = a.max(a, b);
    EXPECT_EQ(mn, Vector3D(1, 4, 3));
    EXPECT_EQ(mx, Vector3D(2, 5, 6));
}

// ——————————————————————————————————————————————————————————————————————————
// 10) scalar operators +, -, * and /
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ScalarAdditionMultiplication)
{
    Vector3D v(1, 2, 3);
    auto     a1 = 5.0 + v;
    auto     a2 = v + 5.0;
    EXPECT_EQ(a1, a2);
    EXPECT_EQ(a1, Vector3D(6, 7, 8));

    auto m1 = 2.0 * v;
    auto m2 = v * 2.0;
    EXPECT_EQ(m1, Vector3D(2, 4, 6));
}

TEST(Vector3D_Test, VectorAdditionSubtraction)
{
    Vector3D a(1, 1, 1), b(2, 3, 4);
    EXPECT_EQ(a + b, Vector3D(3, 4, 5));
    EXPECT_EQ(-b, Vector3D(-2, -3, -4));
}

TEST(Vector3D_Test, ScalarDivisionAndVectorDivision)
{
    Vector3D v(8, 6, 4);
    EXPECT_EQ(v / 2.0, Vector3D(4, 3, 2));
    EXPECT_EQ(v / Vector3D(2, 2, 2), Vector3D(4, 3, 2));
}
