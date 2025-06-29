#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Constructors and getters
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Constructor)
{
    // Zero
    Vector3D v0;
    EXPECT_DECIMAL_EQ(v0.getX(), decimal(0));
    EXPECT_DECIMAL_EQ(v0.getY(), decimal(0));
    EXPECT_DECIMAL_EQ(v0.getZ(), decimal(0));

    // One Value
    Vector3D v1(decimal(1));
    EXPECT_DECIMAL_EQ(v1.getX(), decimal(1));
    EXPECT_DECIMAL_EQ(v1.getY(), decimal(1));
    EXPECT_DECIMAL_EQ(v1.getZ(), decimal(1));

    // Three Values
    Vector3D v2(decimal(1.1), decimal(-2.2), decimal(3.3));
    EXPECT_DECIMAL_EQ(v2.getX(), decimal(1.1));
    EXPECT_DECIMAL_EQ(v2.getY(), decimal(-2.2));
    EXPECT_DECIMAL_EQ(v2.getZ(), decimal(3.3));
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Utilities
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Utilies)
{
    Vector3D v0(decimal(1), decimal(2), decimal(-2));

    // Norm
    EXPECT_DECIMAL_EQ(v0.getNormSquare(), decimal(9));
    EXPECT_DECIMAL_EQ(v0.getNorm(), decimal(3));

    // Min & Max
    EXPECT_DECIMAL_EQ(v0.getMinValue(), decimal(-2));
    EXPECT_DECIMAL_EQ(v0.getMaxValue(), decimal(2));

    // Absolute
    Vector3D absV = v0.getAbsoluteVector();
    EXPECT_EQ(absV, Vector3D(decimal(1), decimal(2), decimal(2)));
    v0.absolute();
    EXPECT_EQ(absV, v0);

    // Normalize
    Vector3D normV   = v0.getNormalized();
    decimal  invNorm = 1 / v0.getNorm();
    EXPECT_EQ(normV, invNorm * v0);
    v0.normalize();
    EXPECT_EQ(normV, v0);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Setters
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Setters)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));
    v.setX(decimal(4));
    v.setY(decimal(5));
    v.setZ(decimal(6));
    EXPECT_EQ(v, Vector3D(decimal(4), decimal(5), decimal(6)));

    v.setToZero();
    EXPECT_EQ(v, Vector3D());

    v.setAllValues(4);
    EXPECT_EQ(v, Vector3D(decimal(4)));

    v.setAllValues(decimal(0), decimal(5), decimal(-3));
    EXPECT_EQ(v, Vector3D(decimal(0), decimal(5), decimal(-3)));
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Property Checks
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3_Test, PropertyCheck)
{
    Vector3D v;
    EXPECT_TRUE(v.isZero());
    EXPECT_TRUE(v.isLengthEqual(decimal(0)));
    EXPECT_FALSE(v.isNormalized());

    v.setAllValues(decimal(4), decimal(0), decimal(-3));
    EXPECT_TRUE(v.isLengthEqual(decimal(25)));
    EXPECT_FALSE(v.isNormalized());
    v.normalize();
    EXPECT_TRUE(v.isNormalized());
    EXPECT_TRUE(v.isFinite());

    decimal  inf = std::numeric_limits<decimal>::infinity();
    Vector3D infV(0, -3, inf);
    EXPECT_FALSE(infV.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Vector Operations
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, VectorOperations)
{
    Vector3D x(decimal(3), decimal(-7), decimal(5));
    Vector3D y(decimal(0), decimal(-2), decimal(-10));

    // Dot Product
    EXPECT_DECIMAL_EQ(y.dotProduct(y), decimal(104));
    EXPECT_DECIMAL_EQ(x.dotProduct(y), decimal(-36));
    EXPECT_EQ(x.dotProduct(y), y.dotProduct(x));
    EXPECT_DECIMAL_EQ(x.dotProduct(y), dotProduct(x, y));

    // Cross Product
    EXPECT_EQ(x.crossProduct(y), Vector3D(decimal(80), decimal(30), decimal(-6)));
    EXPECT_EQ(x.crossProduct(y), crossProduct(x, y));
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Comparison Operations
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ComparisonOperators)
{
    Vector3D u(decimal(-3), decimal(0), decimal(2));
    Vector3D v(decimal(5), decimal(2), decimal(-1));
    Vector3D w(decimal(-3), decimal(0), decimal(2));

    // Equality & Inequality
    EXPECT_TRUE(u == w);
    EXPECT_FALSE(u != w);
    EXPECT_TRUE(u != v);

    // ApproxEqual
    u.setAllValues(decimal(1.001), decimal(2.0001), decimal(3.0001));
    v.setAllValues(decimal(1.002), decimal(2.0002), decimal(3.0002));
    EXPECT_TRUE(u.approxEqual(v, decimal(0.01)));
    EXPECT_FALSE(u.approxEqual(v, decimal(1e-5)));

    // Higher & Smaller
    EXPECT_TRUE(u < v);
    EXPECT_FALSE(v < u);

    EXPECT_TRUE(u <= v);
    EXPECT_TRUE(u <= u);
    EXPECT_FALSE(v <= u);

    EXPECT_TRUE(v > u);
    EXPECT_FALSE(u > v);

    EXPECT_TRUE(v >= u);
    EXPECT_TRUE(u >= u);
    EXPECT_FALSE(u >= v);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Element Access Operations
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ElementAccessChecked)
{
    Vector3D       v(decimal(1), decimal(2), decimal(3));
    const Vector3D u(decimal(4), decimal(5), decimal(6));

    // operator()(int) const and non-const
    EXPECT_DECIMAL_EQ(v(decimal(0)), decimal(1));
    EXPECT_DECIMAL_EQ(v(decimal(1)), decimal(2));
    EXPECT_DECIMAL_EQ(v(decimal(2)), decimal(3));

    EXPECT_DECIMAL_EQ(v[decimal(0)], decimal(1));
    EXPECT_DECIMAL_EQ(v[decimal(1)], decimal(2));
    EXPECT_DECIMAL_EQ(v[decimal(2)], decimal(3));

    EXPECT_DECIMAL_EQ(u(decimal(0)), decimal(4));
    EXPECT_DECIMAL_EQ(u(decimal(1)), decimal(5));
    EXPECT_DECIMAL_EQ(u(decimal(2)), decimal(6));

    EXPECT_DECIMAL_EQ(u[decimal(0)], decimal(4));
    EXPECT_DECIMAL_EQ(u[decimal(1)], decimal(5));
    EXPECT_DECIMAL_EQ(u[decimal(2)], decimal(6));

    v(decimal(1)) = decimal(42.0);
    EXPECT_DECIMAL_EQ(v(decimal(1)), decimal(42.0));

    // Out-of-range checks
    EXPECT_THROW(v(decimal(-1)), std::out_of_range);
    EXPECT_THROW(v(decimal(3)), std::out_of_range);
    EXPECT_THROW(u(decimal(-1)), std::out_of_range);
    EXPECT_THROW(u(decimal(3)), std::out_of_range);

    // operator[] does not check the index, so I can't check out of range indices
}
TEST(Vector3D_Test, StaticMinMaxFunctions)
{
    Vector3D a(decimal(1), decimal(5), decimal(3)), b(decimal(2), decimal(4), decimal(6));
    Vector3D mn = min(a, b);
    Vector3D mx = max(a, b);
    EXPECT_EQ(mn, Vector3D(decimal(1), decimal(4), decimal(3)));
    EXPECT_EQ(mx, Vector3D(decimal(2), decimal(5), decimal(6)));
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————

// ——————————————————————————————————————————————————————————————————————————
// 11) scalar and vector arithmetic operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, InPlace)
{
    Vector3D v(decimal(1), decimal(2), decimal(3));
    v += Vector3D(decimal(0), decimal(-1), decimal(1));
    EXPECT_EQ(v, Vector3D(decimal(1), decimal(1), decimal(4)));
    v -= Vector3D(decimal(2), decimal(-2), decimal(3));
    EXPECT_EQ(v, Vector3D(decimal(-1), decimal(3), decimal(1)));
    v *= Vector3D(decimal(2), decimal(-1), decimal(0));
    EXPECT_EQ(v, Vector3D(decimal(-2), decimal(-3), decimal(0)));
    v /= Vector3D(decimal(-2), decimal(1), decimal(2));
    EXPECT_EQ(v, Vector3D(decimal(1), decimal(-3), decimal(0)));
    v = -v;
    EXPECT_EQ(v, Vector3D(decimal(-1), decimal(3), decimal(0)));

    // Check division by zero
    EXPECT_THROW(v /= Vector3D(decimal(0)), std::invalid_argument);
}

TEST(Vector3D_Test, Free)
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
    EXPECT_EQ(w / decimal(2), Vector3D(decimal(4), decimal(3), decimal(2)));
    EXPECT_EQ(w / Vector3D(decimal(2), decimal(2), decimal(2)), Vector3D(decimal(4), decimal(3), decimal(2)));

    // division by zero
    EXPECT_THROW({ auto result = w / decimal(0); }, std::invalid_argument);
    EXPECT_THROW({ auto result = w / Vector3D(); }, std::invalid_argument);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
//  Printing
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Printing)
{
    Vector3D          v(decimal(1), decimal(2), decimal(3));
    std::stringstream ss;
    ss << v;
    EXPECT_EQ(ss.str(), "(1,2,3)");
}
