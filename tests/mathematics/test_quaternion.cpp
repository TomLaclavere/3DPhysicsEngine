#include "mathematics/common.hpp"
#include "mathematics/quaternion.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
// 1) Constructors and Getters
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, DefaultConstructorZeroes)
{
    Quaternion q;
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(0));
    EXPECT_TRUE(q.getImaginaryPart() == Vector3D());
}

TEST(Quaternion_Test, ValueConstructor)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(4));
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(decimal(1), decimal(2), decimal(3)));
}

TEST(Quaternion_Test, VectorAndScalarConstructor)
{
    Vector3D   v(decimal(1), decimal(2), decimal(3));
    Quaternion q(v, decimal(4));
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(4));
}

TEST(Quaternion_Test, ScalarAndVectorConstructor)
{
    Vector3D   v(decimal(1), decimal(2), decimal(3));
    Quaternion q(decimal(4), v);
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(4));
}

TEST(Quaternion_Test, FromEulerAngles)
{
    Quaternion q(decimal(0.1), decimal(0.2), decimal(0.3));
    // Just check it's finite and not zero
    EXPECT_TRUE(q.isFinite());
    EXPECT_FALSE(q.isZero());
}

TEST(Quaternion_Test, FromMatrix)
{
    Matrix3x3  m(decimal(1), decimal(0), decimal(0), decimal(0), decimal(1), decimal(0), decimal(0),
                 decimal(0), decimal(1));
    Quaternion q(m);
    EXPECT_TRUE(q.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// 2) Setters
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, Setters)
{
    Quaternion q;
    q.setAllValues(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(decimal(1), decimal(2), decimal(3)));
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(4));

    Vector3D v(decimal(5), decimal(6), decimal(7));
    q.setAllValues(v, decimal(8));
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(8));

    q.setAllValues(decimal(9), v);
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(9));

    q.setToZero();
    EXPECT_TRUE(q.isZero());

    q.setToIdentity();
    EXPECT_TRUE(q.isIdentity());
}

// ——————————————————————————————————————————————————————————————————————————
// 3) Utilities
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ConjugateAndNormalize)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    q.conjugate();
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(decimal(-1), decimal(-2), decimal(-3)));
    q.normalize();
    EXPECT_TRUE(q.isUnit());
}

TEST(Quaternion_Test, Inverse)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    q.normalize();
    Quaternion orig = q;
    q.inverse();
    q.inverse();
    EXPECT_TRUE(q.approxEqual(orig, decimal(1e-5)));
}

TEST(Quaternion_Test, Getters)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_NEAR(q.getNormSquare(),
                decimal(1) * decimal(1) + decimal(2) * decimal(2) + decimal(3) * decimal(3) +
                    decimal(4) * decimal(4),
                decimal(1e-6));
    EXPECT_NEAR(q.getNorm(),
                std::sqrt(decimal(1) * decimal(1) + decimal(2) * decimal(2) + decimal(3) * decimal(3) +
                          decimal(4) * decimal(4)),
                decimal(1e-6));
    Matrix3x3 m = q.getRotationMatrix();
    EXPECT_TRUE(m.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// 4) Property Checks
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, PropertyChecks)
{
    Quaternion q;
    q.setToZero();
    EXPECT_TRUE(q.isZero());
    q.setToIdentity();
    EXPECT_TRUE(q.isIdentity());
    EXPECT_TRUE(q.isUnit());
    EXPECT_TRUE(q.isNormalized());
    EXPECT_TRUE(q.isFinite());
    EXPECT_TRUE(q.isOrthogonal());
    EXPECT_FALSE(q.isInvertible()); // isInvertible returns true if norm==0
}

// ——————————————————————————————————————————————————————————————————————————
// 5) Quaternion Operations
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, DotAndCrossProduct)
{
    Quaternion q1(decimal(1), decimal(0), decimal(0), decimal(1));
    Quaternion q2(decimal(0), decimal(1), decimal(0), decimal(1));
    decimal    dot = q1.dotProduct(q2);
    EXPECT_DECIMAL_EQ(dot, decimal(1)); // 1*0 + 0*1 + 0*0 + 1*1 = 1

    Quaternion cross = q1.crossProduct(q2);
    EXPECT_TRUE(cross.isFinite());
}

// ——————————————————————————————————————————————————————————————————————————
// 6) Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ArithmeticOperators)
{
    Quaternion q1(decimal(1), decimal(2), decimal(3), decimal(4));
    Quaternion q2(decimal(4), decimal(5), decimal(6), decimal(7));

    Quaternion sum = q1 + q2;
    EXPECT_EQ(sum, Quaternion(decimal(1) + decimal(4), decimal(2) + decimal(5), decimal(3) + decimal(6),
                              decimal(4) + decimal(7)));

    Quaternion diff = q1 - q2;
    EXPECT_EQ(diff, Quaternion(decimal(1) - decimal(4), decimal(2) - decimal(5), decimal(3) - decimal(6),
                               decimal(4) - decimal(7)));

    Quaternion prod = q1 * q2;
    EXPECT_EQ(prod, Quaternion(decimal(1) * decimal(4), decimal(2) * decimal(5), decimal(3) * decimal(6),
                               decimal(4) * decimal(7)));

    Quaternion quot = q2 / q1;
    EXPECT_EQ(quot, Quaternion(decimal(4) / decimal(1), decimal(5) / decimal(2), decimal(6) / decimal(3),
                               decimal(7) / decimal(4)));

    Quaternion scalarSum = q1 + decimal(2);
    EXPECT_EQ(scalarSum, Quaternion(decimal(1) + decimal(2), decimal(2) + decimal(2), decimal(3) + decimal(2),
                                    decimal(4) + decimal(2)));

    Quaternion scalarProd = q1 * decimal(2);
    EXPECT_EQ(scalarProd, Quaternion(decimal(1) * decimal(2), decimal(2) * decimal(2),
                                     decimal(3) * decimal(2), decimal(4) * decimal(2)));
}

// ——————————————————————————————————————————————————————————————————————————
// 7) In-place Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, InPlaceArithmeticOperators)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    Quaternion orig = q;

    q += Quaternion(decimal(1), decimal(1), decimal(1), decimal(1));
    EXPECT_EQ(q, Quaternion(decimal(2), decimal(3), decimal(4), decimal(5)));

    q -= Quaternion(decimal(1), decimal(1), decimal(1), decimal(1));
    EXPECT_EQ(q, orig);

    q *= Quaternion(decimal(2), decimal(2), decimal(2), decimal(2));
    EXPECT_EQ(q, Quaternion(decimal(2), decimal(4), decimal(6), decimal(8)));

    q /= Quaternion(decimal(2), decimal(2), decimal(2), decimal(2));
    EXPECT_EQ(q, orig);

    q += decimal(1);
    EXPECT_EQ(q, Quaternion(decimal(2), decimal(3), decimal(4), decimal(5)));

    q -= decimal(1);
    EXPECT_EQ(q, orig);

    q *= decimal(2);
    EXPECT_EQ(q, Quaternion(decimal(2), decimal(4), decimal(6), decimal(8)));

    q /= decimal(2);
    EXPECT_EQ(q, orig);
}

// ——————————————————————————————————————————————————————————————————————————
// 8) Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ComparisonOperators)
{
    Quaternion a(decimal(1), decimal(2), decimal(3), decimal(4)),
        b(decimal(1), decimal(2), decimal(3), decimal(4)), c(decimal(2), decimal(3), decimal(4), decimal(5));

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);

    EXPECT_TRUE(a < c);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(c > a);
    EXPECT_TRUE(c >= a);

    EXPECT_TRUE(a.approxEqual(b, decimal(1e-6)));
    EXPECT_FALSE(a.approxEqual(c, decimal(1e-6)));
}

// ——————————————————————————————————————————————————————————————————————————
// 9) Element Access
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ElementAccess)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));

    EXPECT_DECIMAL_EQ(q(decimal(0)), decimal(1));
    EXPECT_DECIMAL_EQ(q(decimal(1)), decimal(2));
    EXPECT_DECIMAL_EQ(q(decimal(2)), decimal(3));

    q(decimal(0)) = decimal(10);
    EXPECT_DECIMAL_EQ(q(decimal(0)), decimal(10));

    EXPECT_DECIMAL_EQ(q[decimal(0)], decimal(10));
    q[decimal(1)] = decimal(20);
    EXPECT_DECIMAL_EQ(q[decimal(1)], decimal(20));
}

// ——————————————————————————————————————————————————————————————————————————
// 10) Stream Output
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, StreamOutput)
{
    Quaternion        q(decimal(1), decimal(2), decimal(3), decimal(4));
    std::stringstream ss;
    ss << q;
    EXPECT_TRUE(ss.str().find("(") != std::string::npos);
    EXPECT_TRUE(ss.str().find(")") != std::string::npos);
}
