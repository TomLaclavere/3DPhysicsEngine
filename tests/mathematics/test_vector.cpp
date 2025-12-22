#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
//  Constructors and getters
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Constructors)
{
    // Zero
    Vector3D v0;
    EXPECT_DECIMAL_EQ(v0.getX(), 0_d);
    EXPECT_DECIMAL_EQ(v0.getY(), 0_d);
    EXPECT_DECIMAL_EQ(v0.getZ(), 0_d);

    // One Value
    Vector3D v1(1_d);
    EXPECT_DECIMAL_EQ(v1.getX(), 1_d);
    EXPECT_DECIMAL_EQ(v1.getY(), 1_d);
    EXPECT_DECIMAL_EQ(v1.getZ(), 1_d);

    // Three Values
    Vector3D v2(1.1_d, -2.2_d, 3.3_d);
    EXPECT_DECIMAL_EQ(v2.getX(), 1.1_d);
    EXPECT_DECIMAL_EQ(v2.getY(), -2.2_d);
    EXPECT_DECIMAL_EQ(v2.getZ(), 3.3_d);

    // Copy constructor
    Vector3D v3(v2);
    EXPECT_VECTOR_EQ(v3, v2);
}

// ——————————————————————————————————————————————————————————————————————————
//  Utilities
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Utilies)
{
    Vector3D v0(1_d, 2_d, -2_d);

    // Norm
    EXPECT_DECIMAL_EQ(v0.getNormSquare(), 9_d);
    EXPECT_DECIMAL_EQ(v0.getNorm(), 3_d);

    // Min & Max
    EXPECT_DECIMAL_EQ(v0.getMinValue(), -2_d);
    EXPECT_DECIMAL_EQ(v0.getMaxValue(), 2_d);

    // Absolute
    Vector3D absV = v0.getAbsolute();
    EXPECT_VECTOR_EQ(absV, Vector3D(1_d, 2_d, 2_d));
    v0.absolute();
    EXPECT_VECTOR_EQ(absV, v0);

    // Normalise
    Vector3D normV   = v0.getNormalised();
    decimal  invNorm = 1 / v0.getNorm();
    EXPECT_VECTOR_EQ(normV, invNorm * v0);
    v0.normalise();
    EXPECT_VECTOR_EQ(normV, v0);
    v0.setToNull();
    v0.normalise();
    EXPECT_VECTOR_EQ(v0, Vector3D(0_d, 0_d, 0_d));
}

TEST(Vector3D_Test, StaticMinMaxFunctions)
{
    Vector3D a(1_d, 5_d, 3_d), b(2_d, 4_d, 6_d);
    Vector3D mn = min(a, b);
    Vector3D mx = max(a, b);
    EXPECT_VECTOR_EQ(mn, Vector3D(1_d, 4_d, 3_d));
    EXPECT_VECTOR_EQ(mx, Vector3D(2_d, 5_d, 6_d));
}

// ——————————————————————————————————————————————————————————————————————————
//  Setters
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Setters)
{
    Vector3D v(1_d, 2_d, 3_d);
    v.setX(4_d);
    v.setY(5_d);
    v.setZ(6_d);
    EXPECT_VECTOR_EQ(v, Vector3D(4_d, 5_d, 6_d));

    v.setToNull();
    EXPECT_VECTOR_EQ(v, Vector3D());

    v.setAllValues(4);
    EXPECT_VECTOR_EQ(v, Vector3D(4_d));

    v.setAllValues(0_d, 5_d, -3_d);
    EXPECT_VECTOR_EQ(v, Vector3D(0_d, 5_d, -3_d));
}

// ——————————————————————————————————————————————————————————————————————————
//  Property Check
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3_Test, PropertyCheck)
{
    Vector3D v;
    EXPECT_TRUE(v.isNull());
    EXPECT_TRUE(v.isLengthEqual(0_d));
    EXPECT_FALSE(v.isNormalised());

    v.setAllValues(4_d, 0_d, -3_d);
    EXPECT_TRUE(v.isLengthEqual(25_d));
    EXPECT_FALSE(v.isNormalised());
    v.normalise();
    EXPECT_TRUE(v.isNormalised());
    EXPECT_TRUE(v.isFinite());

    decimal  inf = std::numeric_limits<decimal>::infinity();
    Vector3D infV(0, -3, inf);
    EXPECT_FALSE(infV.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
//  Vector Operations
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, VectorOperations)
{
    Vector3D x(3_d, -7_d, 5_d);
    Vector3D y(0_d, -2_d, -10_d);

    // Dot Product
    EXPECT_DECIMAL_EQ(y.dotProduct(y), 104_d);
    EXPECT_DECIMAL_EQ(x.dotProduct(y), -36_d);
    EXPECT_DECIMAL_EQ(x.dotProduct(y), y.dotProduct(x));
    EXPECT_DECIMAL_EQ(x.dotProduct(y), dotProduct(x, y));

    // Cross Product
    EXPECT_VECTOR_EQ(x.crossProduct(y), Vector3D(80_d, 30_d, -6_d));
    EXPECT_VECTOR_EQ(x.crossProduct(y), crossProduct(x, y));
}

// ——————————————————————————————————————————————————————————————————————————
//  Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ComparisonOperators)
{
    Vector3D u(-3_d, 0_d, 2_d);
    Vector3D v(5_d, 2_d, -1_d);
    Vector3D w(-3_d, 0_d, 2_d);

    // Equality & Inequality
    EXPECT_TRUE(u == w);
    EXPECT_TRUE(u.getV() == w.getV());
    EXPECT_FALSE(u != w);
    EXPECT_TRUE(u != v);

    // ApproxEqual
    u.setAllValues(1.001_d, 2.0001_d, 3.0001_d);
    v.setAllValues(1.002_d, 2.0002_d, 3.0002_d);
    EXPECT_TRUE(u.approxEqual(v, 0.01_d));
    EXPECT_FALSE(u.approxEqual(v, 1e-5_d));
    Vector3D u_(1.001_d, 1.0001_d, 3.0001_d);
    Vector3D u__(1.001_d, 2.0001_d, 1.0001_d);

    // Higher & Smaller
    EXPECT_TRUE(u < v);
    EXPECT_FALSE(u < u_);
    EXPECT_FALSE(u < u__);
    EXPECT_FALSE(v < u);

    EXPECT_TRUE(u <= v);
    EXPECT_TRUE(u <= u);
    EXPECT_FALSE(u <= u_);
    EXPECT_FALSE(u <= u__);
    EXPECT_FALSE(v <= u);

    EXPECT_TRUE(v > u);
    EXPECT_FALSE(u_ > u);
    EXPECT_FALSE(u > v);

    EXPECT_TRUE(v >= u);
    EXPECT_TRUE(u >= u);
    EXPECT_FALSE(u_ >= u);
    EXPECT_FALSE(u >= v);
}

// ——————————————————————————————————————————————————————————————————————————
//  Element Acess Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, ElementAccessChecked)
{
    Vector3D       v(1_d, 2_d, 3_d);
    const Vector3D u(4_d, 5_d, 6_d);

    // operator()(int) const and non-const
    EXPECT_DECIMAL_EQ(v(0_d), 1_d);
    EXPECT_DECIMAL_EQ(v(1_d), 2_d);
    EXPECT_DECIMAL_EQ(v(2_d), 3_d);

    EXPECT_DECIMAL_EQ(u(0_d), 4_d);
    EXPECT_DECIMAL_EQ(u(1_d), 5_d);
    EXPECT_DECIMAL_EQ(u(2_d), 6_d);

    // operator[] const and non-const

    EXPECT_DECIMAL_EQ(v[0_d], 1_d);
    EXPECT_DECIMAL_EQ(v[1_d], 2_d);
    EXPECT_DECIMAL_EQ(v[2_d], 3_d);

    EXPECT_DECIMAL_EQ(u[0_d], 4_d);
    EXPECT_DECIMAL_EQ(u[1_d], 5_d);
    EXPECT_DECIMAL_EQ(u[2_d], 6_d);

    v(1_d) = 42.0_d;
    EXPECT_DECIMAL_EQ(v(1_d), 42.0_d);

    // operator at const and non-const

    EXPECT_DECIMAL_EQ(v.at(0_d), 1_d);
    EXPECT_DECIMAL_EQ(v.at(1_d), 42_d);
    EXPECT_DECIMAL_EQ(v.at(2_d), 3_d);

    EXPECT_DECIMAL_EQ(u.at(0_d), 4_d);
    EXPECT_DECIMAL_EQ(u.at(1_d), 5_d);
    EXPECT_DECIMAL_EQ(u.at(2_d), 6_d);

    // Out-of-range checks
    EXPECT_THROW(v.at(-1_d), std::out_of_range);
    EXPECT_THROW(v.at(3_d), std::out_of_range);
    EXPECT_THROW(u.at(-1_d), std::out_of_range);
    EXPECT_THROW(u.at(3_d), std::out_of_range);

    // operator[] & () do not check the index, so I can't check out of range indices
}

// ——————————————————————————————————————————————————————————————————————————
//  Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, InPlace)
{
    Vector3D v(1_d, 2_d, 3_d);
    v += Vector3D(0_d, -1_d, 1_d);
    EXPECT_VECTOR_EQ(v, Vector3D(1_d, 1_d, 4_d));
    v -= Vector3D(2_d, -2_d, 3_d);
    EXPECT_VECTOR_EQ(v, Vector3D(-1_d, 3_d, 1_d));
    v *= Vector3D(2_d, -1_d, 0_d);
    EXPECT_VECTOR_EQ(v, Vector3D(-2_d, -3_d, 0_d));
    v /= Vector3D(-2_d, 1_d, 2_d);
    EXPECT_VECTOR_EQ(v, Vector3D(1_d, -3_d, 0_d));
    v = -v;
    EXPECT_VECTOR_EQ(v, Vector3D(-1_d, 3_d, 0_d));

    // Check division by zero
    EXPECT_THROW(v /= Vector3D(0_d), std::invalid_argument);
    EXPECT_THROW(v /= 0_d, std::invalid_argument);
}

TEST(Vector3D_Test, Free)
{
    Vector3D v(1_d, 2_d, 3_d);

    // scalar add/mul both ways
    EXPECT_VECTOR_EQ(5.0_d + v, v + 5.0_d);
    EXPECT_VECTOR_EQ(Vector3D(6_d, 7_d, 8_d), v + 5.0_d);

    EXPECT_VECTOR_EQ(2.0_d * v, v * 2.0_d);
    EXPECT_VECTOR_EQ(Vector3D(2_d, 4_d, 6_d), v * 2.0_d);

    // vector add/sub and negation
    Vector3D a(1_d, 1_d, 1_d), b(2_d, 3_d, 4_d);
    EXPECT_VECTOR_EQ(a + b, Vector3D(3_d, 4_d, 5_d));
    EXPECT_VECTOR_EQ(a - b, Vector3D(-1_d, -2_d, -3_d));
    EXPECT_VECTOR_EQ(b - 1_d, Vector3D(1_d, 2_d, 3_d));
    EXPECT_VECTOR_EQ(2_d - b, Vector3D(0_d, -1_d, -2_d));
    EXPECT_VECTOR_EQ(-b, Vector3D(-2_d, -3_d, -4_d));

    // scalar and vector division
    Vector3D w(8_d, 6_d, 4_d);
    EXPECT_VECTOR_EQ(w / 2_d, Vector3D(4_d, 3_d, 2_d));
    EXPECT_VECTOR_EQ(2_d / w, Vector3D(0.25_d, decimal(1_d / 3_d), 0.5_d));
    EXPECT_VECTOR_EQ(w / Vector3D(2_d, 2_d, 2_d), Vector3D(4_d, 3_d, 2_d));

    // vector multiplication
    EXPECT_VECTOR_EQ(v * w, Vector3D(8_d, 12_d, 12_d));

    // division by zero
    EXPECT_THROW(w / 0_d, std::invalid_argument);
    EXPECT_THROW(w / Vector3D(0_d), std::invalid_argument);
    EXPECT_THROW(2_d / Vector3D(0_d), std::invalid_argument);
}

// ——————————————————————————————————————————————————————————————————————————
//  Printing
// ——————————————————————————————————————————————————————————————————————————
TEST(Vector3D_Test, Printing)
{
    Vector3D          v(1_d, 2_d, 3_d);
    std::stringstream ss;
    ss << v;
    EXPECT_EQ(ss.str(), "(1.000e+00 , 2.000e+00 , 3.000e+00)");
    EXPECT_EQ(v.formatVector(), "( 1.000e+00,  2.000e+00,  3.000e+00)");
}
