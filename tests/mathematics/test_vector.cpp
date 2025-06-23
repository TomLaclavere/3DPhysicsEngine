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
    EXPECT_DECIMAL_EQ(v.getX(), decimal(0));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(0));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(0));
}

TEST(Vector3D_Test, OneValueConstructor)
{
    Vector3D v(decimal(1));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(1));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(1));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(1));
}

TEST(Vector3D_Test, ValuesConstructor)
{
    Vector3D v(decimal(1.1), decimal(-2.2), decimal(3.3));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(1.1));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(-2.2));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(3.3));
}

// ——————————————————————————————————————————————————————————————————————————
// 2) Norm
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, NormAndSquare)
{
    Vector3D v(decimal(1), decimal(2), decimal(2));
    EXPECT_DECIMAL_EQ(v.getNormSquare(), decimal(9));
    EXPECT_DECIMAL_EQ(v.getNorm(), decimal(3)); // sqrt(9)
}

// ——————————————————————————————————————————————————————————————————————————
// 3) Min/Max
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, MinMaxValue)
{
    Vector3D v(decimal(-5), decimal(7), decimal(2));
    EXPECT_DECIMAL_EQ(v.getMinValue(), decimal(-5));
    EXPECT_DECIMAL_EQ(v.getMaxValue(), decimal(7));
}

// ——————————————————————————————————————————————————————————————————————————
// 4) Absolute and normalize
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, AbsoluteVector)
{
    Vector3D v(decimal(-1), decimal(-2), decimal(3));
    auto     absV = v.getAbsoluteVector();
    EXPECT_DECIMAL_EQ(absV.getX(), decimal(1));
    EXPECT_DECIMAL_EQ(absV.getY(), decimal(2));
    EXPECT_DECIMAL_EQ(absV.getZ(), decimal(3));
}

TEST(Vector3D_Test, NormalizeProducesNormalized)
{
    Vector3D v(decimal(0), decimal(3), decimal(4));
    v.normalize();
    EXPECT_DECIMAL_EQ(v.getNorm(), decimal(1));
    EXPECT_TRUE(v.isNormalized());
}

// ——————————————————————————————————————————————————————————————————————————
// 5) setX, setY, setZ, setAllValues, setToZero
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Mutators)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));
    v.setX(decimal(4));
    v.setY(decimal(5));
    v.setZ(decimal(6));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(4));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(5));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(6));
}

TEST(Vector3D_Test, SetAllValues)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));
    v.setAllValues(decimal(4));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(4));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(4));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(4));
    v.setAllValues(decimal(1), decimal(2), decimal(3));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(1));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(2));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(3));
}

TEST(Vector3D_Test, SetToZero)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));
    v.setToZero();
    EXPECT_DECIMAL_EQ(v.getX(), decimal(0));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(0));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(0));
}

// ——————————————————————————————————————————————————————————————————————————
// 6) isZero, isUnit, isLengthEqual, isFinite
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, IsZeroAndIsLengthEqual)
{
    Vector3D z;
    EXPECT_TRUE(z.isZero());
    EXPECT_TRUE(z.isLengthEqual(decimal(0)));
    EXPECT_FALSE(z.isNormalized());
}

TEST(Vector3D_Test, IsFiniteDetectsInfNaN)
{
    decimal  inf = std::numeric_limits<decimal>::infinity();
    Vector3D v(inf, decimal(0), decimal(0));
    EXPECT_FALSE(v.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// 7) dotProduct and crossProduct
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, DotProductOrthogonal)
{
    Vector3D x(decimal(1), decimal(0), decimal(0)), y(decimal(0), decimal(1), decimal(0));
    EXPECT_DECIMAL_EQ(x.dotProduct(y), decimal(0));
    EXPECT_DECIMAL_EQ(x.dotProduct(x), decimal(1));
}

TEST(Vector3D_Test, CrossProductRightHanded)
{
    Vector3D x(decimal(1), decimal(0), decimal(0)), y(decimal(0), decimal(1), decimal(0));
    auto     z = x.crossProduct(y);
    EXPECT_DECIMAL_EQ(z.getX(), decimal(0));
    EXPECT_DECIMAL_EQ(z.getY(), decimal(0));
    EXPECT_DECIMAL_EQ(z.getZ(), decimal(1));
}

// ——————————————————————————————————————————————————————————————————————————
// 8) operators ==, !=, <, approxEqual
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, EqualityAndInequality)
{
    Vector3D a(decimal(1), decimal(2), decimal(3)), b(decimal(1), decimal(2), decimal(3)),
        c(decimal(3), decimal(2), decimal(1));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(Vector3D_Test, ApproxEqualWithPrecision)
{
    Vector3D a(decimal(1.001), decimal(2.0001), decimal(3.0001));
    Vector3D b(decimal(1.002), decimal(2.0002), decimal(3.0002));
    EXPECT_TRUE(a.approxEqual(b, decimal(0.01)));
    EXPECT_FALSE(a.approxEqual(b, decimal(1e-5)));
}

TEST(Vector3D_Test, AllComparisonOperators)
{
    Vector3D a(1, 2, 3), b(1, 2, 4), c(2, 2, 3), d(1, 3, 3);

    // <
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);

    // <=
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= a);
    EXPECT_FALSE(b <= a);

    // >
    EXPECT_TRUE(b > a);
    EXPECT_FALSE(a > b);

    // >=
    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a >= a);
    EXPECT_FALSE(a >= b);
}

TEST(Vector3D_Test, UnaryMinus)
{
    Vector3D v(1, -2, 3);
    Vector3D neg = -v;
    EXPECT_DECIMAL_EQ(neg.getX(), -1);
    EXPECT_DECIMAL_EQ(neg.getY(), 2);
    EXPECT_DECIMAL_EQ(neg.getZ(), -3);
}

TEST(Vector3D_Test, DivideAssignScalar)
{
    Vector3D v(2, 4, 8);
    v /= 2;
    EXPECT_DECIMAL_EQ(v.getX(), 1);
    EXPECT_DECIMAL_EQ(v.getY(), 2);
    EXPECT_DECIMAL_EQ(v.getZ(), 4);
}

// ——————————————————————————————————————————————————————————————————————————
// 9) Element Access
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ElementAccessChecked)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));

    // operator()(int) const and non-const
    EXPECT_DECIMAL_EQ(v(decimal(0)), decimal(1));
    EXPECT_DECIMAL_EQ(v(decimal(1)), decimal(2));
    EXPECT_DECIMAL_EQ(v(decimal(2)), decimal(3));

    v(decimal(1)) = decimal(42.0);
    EXPECT_DECIMAL_EQ(v(decimal(1)), decimal(42.0));

    // Out-of-range checks
    EXPECT_THROW(v(decimal(-1)), std::out_of_range);
    EXPECT_THROW(v(decimal(3)), std::out_of_range);

    const Vector3D cv(decimal(4), decimal(5), decimal(6));
    EXPECT_DECIMAL_EQ(cv(decimal(0)), decimal(4));
    EXPECT_DECIMAL_EQ(cv(decimal(1)), decimal(5));
    EXPECT_DECIMAL_EQ(cv(decimal(2)), decimal(6));
    EXPECT_THROW(cv(decimal(-1)), std::out_of_range);
    EXPECT_THROW(cv(decimal(3)), std::out_of_range);
}

TEST(Vector3D_Test, ElementAccessUnchecked)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));

    // operator[](int) const and non-const
    EXPECT_DECIMAL_EQ(v[decimal(0)], decimal(1));
    EXPECT_DECIMAL_EQ(v[decimal(1)], decimal(2));
    EXPECT_DECIMAL_EQ(v[decimal(2)], decimal(3));

    v[decimal(2)] = decimal(99.0);
    EXPECT_DECIMAL_EQ(v[decimal(2)], decimal(99.0));

    const Vector3D cv(decimal(7), decimal(8), decimal(9));
    EXPECT_DECIMAL_EQ(cv[decimal(0)], decimal(7));
    EXPECT_DECIMAL_EQ(cv[decimal(1)], decimal(8));
    EXPECT_DECIMAL_EQ(cv[decimal(2)], decimal(9));
}

// ——————————————————————————————————————————————————————————————————————————
// 10) static min/max functions
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, StaticMinMaxFunctions)
{
    Vector3D a(decimal(1), decimal(5), decimal(3)), b(decimal(2), decimal(4), decimal(6));
    auto     mn = min(a, b);
    auto     mx = max(a, b);
    EXPECT_EQ(mn, Vector3D(decimal(1), decimal(4), decimal(3)));
    EXPECT_EQ(mx, Vector3D(decimal(2), decimal(5), decimal(6)));
}

// ——————————————————————————————————————————————————————————————————————————
// 11) scalar and vector arithmetic operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, CompoundOperators)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));
    v += Vector3D(decimal(1), decimal(1), decimal(1));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(2));
    v -= Vector3D(decimal(1), decimal(2), decimal(3));
    EXPECT_DECIMAL_EQ(v.getY(), decimal(1)); // 3-2
    v *= Vector3D(decimal(2), decimal(3), decimal(4));
    EXPECT_DECIMAL_EQ(v.getZ(), decimal(4)); // 1*4
    v /= Vector3D(decimal(2), decimal(2), decimal(2));
    EXPECT_DECIMAL_EQ(v.getX(), decimal(1)); // 2/2
}

TEST(Vector3D_Test, ScalarAndVectorOperators)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));

    // scalar add/mul both ways
    EXPECT_EQ(decimal(5.0) + v, v + decimal(5.0));
    EXPECT_EQ(Vector3D(decimal(6), decimal(7), decimal(8)), v + decimal(5.0));

    EXPECT_EQ(decimal(2.0) * v, v * decimal(2.0));
    EXPECT_EQ(Vector3D(decimal(2), decimal(4), decimal(6)), v * decimal(2.0));

    // vector add/sub and negation
    Vector3D a(decimal(1), decimal(1), decimal(1)), b(decimal(2), decimal(3), decimal(4));
    EXPECT_EQ(a + b, Vector3D(decimal(3), decimal(4), decimal(5)));
    EXPECT_EQ(-b, Vector3D(decimal(-2), decimal(-3), decimal(-4)));

    // scalar and vector division
    Vector3D w(decimal(8), decimal(6), decimal(4));
    EXPECT_EQ(w / decimal(2.0), Vector3D(decimal(4), decimal(3), decimal(2)));
    EXPECT_EQ(w / Vector3D(decimal(2), decimal(2), decimal(2)), Vector3D(decimal(4), decimal(3), decimal(2)));
}

// ——————————————————————————————————————————————————————————————————————————
// 12) stream output
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, StreamOutput)
{
    Vector3D          v(decimal(1), decimal(2), decimal(3));
    std::stringstream ss;
    ss << v;
    EXPECT_EQ(ss.str(), "(1,2,3)");
}
