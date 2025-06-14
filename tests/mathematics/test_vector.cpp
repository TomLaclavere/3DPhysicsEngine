#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"

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
TEST(Vector3D_Test, DefaultConstructorZeroes)
{
    Vector3D v;
    EXPECT_DECIMAL_EQ(v.getX(), 0.0);
    EXPECT_DECIMAL_EQ(v.getY(), 0.0);
    EXPECT_DECIMAL_EQ(v.getZ(), 0.0);
}

TEST(Vector3D_Test, OneValueConstructor)
{
    Vector3D v(1.0);
    EXPECT_DECIMAL_EQ(v.getX(), 1.0);
    EXPECT_DECIMAL_EQ(v.getY(), 1.0);
    EXPECT_DECIMAL_EQ(v.getZ(), 1.0);
}

TEST(Vector3D_Test, ValuesConstructor)
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
// 5) setX, setY, setZ, setAllValues, setToZero
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Mutators)
{
    Vector3D v(1, 2, 3);
    v.setX(4);
    v.setY(5);
    v.setZ(6);
    EXPECT_DECIMAL_EQ(v.getX(), 4);
    EXPECT_DECIMAL_EQ(v.getY(), 5);
    EXPECT_DECIMAL_EQ(v.getZ(), 6);
}

TEST(Vector3D_Test, SetAllValues)
{
    Vector3D v(1, 2, 3);
    v.setAllValues(4);
    EXPECT_DECIMAL_EQ(v.getX(), 4);
    EXPECT_DECIMAL_EQ(v.getY(), 4);
    EXPECT_DECIMAL_EQ(v.getZ(), 4);
    v.setAllValues(1, 2, 3);
    EXPECT_DECIMAL_EQ(v.getX(), 1);
    EXPECT_DECIMAL_EQ(v.getY(), 2);
    EXPECT_DECIMAL_EQ(v.getZ(), 3);
}

TEST(Vector3D_Test, SetToZero)
{
    Vector3D v(1, 2, 3);
    v.setToZero();
    EXPECT_DECIMAL_EQ(v.getX(), 0);
    EXPECT_DECIMAL_EQ(v.getY(), 0);
    EXPECT_DECIMAL_EQ(v.getZ(), 0);
}

// ——————————————————————————————————————————————————————————————————————————
// 6) isZero, isUnit, isLengthEqual, isFinite
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
// 7) dotProduct and crossProduct
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
// 8) operators ==, !=, <, approxEqual
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
// 9) Element Access
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ElementAccessChecked)
{
    Vector3D v(1, 2, 3);

    // operator()(int) const and non-const
    EXPECT_DECIMAL_EQ(v(0), 1);
    EXPECT_DECIMAL_EQ(v(1), 2);
    EXPECT_DECIMAL_EQ(v(2), 3);

    v(1) = 42.0;
    EXPECT_DECIMAL_EQ(v(1), 42.0);

    // Out-of-range checks
    EXPECT_THROW(v(-1), std::out_of_range);
    EXPECT_THROW(v(3), std::out_of_range);

    const Vector3D cv(4, 5, 6);
    EXPECT_DECIMAL_EQ(cv(0), 4);
    EXPECT_DECIMAL_EQ(cv(1), 5);
    EXPECT_DECIMAL_EQ(cv(2), 6);
    EXPECT_THROW(cv(-1), std::out_of_range);
    EXPECT_THROW(cv(3), std::out_of_range);
}

TEST(Vector3D_Test, ElementAccessUnchecked)
{
    Vector3D v(1, 2, 3);

    // operator[](int) const and non-const
    EXPECT_DECIMAL_EQ(v[0], 1);
    EXPECT_DECIMAL_EQ(v[1], 2);
    EXPECT_DECIMAL_EQ(v[2], 3);

    v[2] = 99.0;
    EXPECT_DECIMAL_EQ(v[2], 99.0);

    const Vector3D cv(7, 8, 9);
    EXPECT_DECIMAL_EQ(cv[0], 7);
    EXPECT_DECIMAL_EQ(cv[1], 8);
    EXPECT_DECIMAL_EQ(cv[2], 9);
}

// ——————————————————————————————————————————————————————————————————————————
// 10) static min/max functions
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, StaticMinMaxFunctions)
{
    Vector3D a(1, 5, 3), b(2, 4, 6);
    auto     mn = min(a, b);
    auto     mx = max(a, b);
    EXPECT_EQ(mn, Vector3D(1, 4, 3));
    EXPECT_EQ(mx, Vector3D(2, 5, 6));
}

// ——————————————————————————————————————————————————————————————————————————
// 11) scalar and vector arithmetic operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, CompoundOperators)
{
    Vector3D v(1, 2, 3);
    v += Vector3D(1, 1, 1);
    EXPECT_DECIMAL_EQ(v.getX(), 2);
    v -= Vector3D(1, 2, 3);
    EXPECT_DECIMAL_EQ(v.getY(), 1); // 3-2
    v *= Vector3D(2, 3, 4);
    EXPECT_DECIMAL_EQ(v.getZ(), 4); // 1*4
    v /= Vector3D(2, 2, 2);
    EXPECT_DECIMAL_EQ(v.getX(), 1); // 2/2
}

TEST(Vector3D_Test, ScalarAndVectorOperators)
{
    Vector3D v(1, 2, 3);

    // scalar add/mul both ways
    EXPECT_EQ(5.0 + v, v + 5.0);
    EXPECT_EQ(Vector3D(6, 7, 8), v + 5.0);

    EXPECT_EQ(2.0 * v, v * 2.0);
    EXPECT_EQ(Vector3D(2, 4, 6), v * 2.0);

    // vector add/sub and negation
    Vector3D a(1, 1, 1), b(2, 3, 4);
    EXPECT_EQ(a + b, Vector3D(3, 4, 5));
    EXPECT_EQ(-b, Vector3D(-2, -3, -4));

    // scalar and vector division
    Vector3D w(8, 6, 4);
    EXPECT_EQ(w / 2.0, Vector3D(4, 3, 2));
    EXPECT_EQ(w / Vector3D(2, 2, 2), Vector3D(4, 3, 2));
}

// ——————————————————————————————————————————————————————————————————————————
// 12) stream output
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, StreamOutput)
{
    Vector3D          v(1, 2, 3);
    std::stringstream ss;
    ss << v;
    EXPECT_EQ(ss.str(), "(1,2,3)");
}
