#include "mathematics/common.hpp"
#include "mathematics/quaternion.hpp"
#include "test_functions.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 1) Constructors and Getters
// ——————————————————————————————————————————————————————————————————————————
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
    // Vector scalar
    Vector3D   v(decimal(1), decimal(2), decimal(3));
    Quaternion q(v, decimal(4));
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), decimal(4));
    // Scalar vector
    Vector3D   v_bis(decimal(1), decimal(2), decimal(3));
    Quaternion q_bis(decimal(4), v_bis);
    EXPECT_EQ(q_bis.getImaginaryPart(), v_bis);
    EXPECT_DECIMAL_EQ(q_bis.getRealPart(), decimal(4));
}
TEST(Quaternion_Test, FromEulerAngles_ProducesExpectedRotation)
{
    // 90 degrees (pi/2) rotation about X axis
    decimal    angleX = decimal(M_PI) / 2;
    decimal    angleY = 0;
    decimal    angleZ = 0;
    Quaternion q(angleX, angleY, angleZ);

    // The expected quaternion for 90deg about X is (sqrt(0.5), 0, 0, sqrt(0.5))
    decimal s = std::sin(angleX / 2);
    decimal c = std::cos(angleX / 2);
    EXPECT_TRUE(q.approxEqual(Quaternion(s, 0, 0, c), PRECISION_MACHINE));

    // Check normalization
    EXPECT_NEAR(q.getNorm(), decimal(1), PRECISION_MACHINE);

    // Check rotation matrix matches expected
    Matrix3x3 expected(1, 0, 0, 0, 0, -1, 0, 1, 0);
    Matrix3x3 rot = q.getRotationMatrix();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_NEAR(rot(i, j), expected(i, j), PRECISION_MACHINE);
}
TEST(Quaternion_Test, FromMatrix_ProducesExpectedQuaternion)
{
    // 90 degrees (pi/2) rotation about X axis
    decimal    angle = decimal(M_PI) / 2;
    Matrix3x3  m(1, 0, 0, 0, std::cos(angle), -std::sin(angle), 0, std::sin(angle), std::cos(angle));
    Quaternion q(m);

    // The expected quaternion for 90deg about X is (sqrt(0.5), 0, 0, sqrt(0.5))
    decimal    s = std::sin(angle / 2);
    decimal    c = std::cos(angle / 2);
    Quaternion expected(s, 0, 0, c);

    EXPECT_TRUE(q.approxEqual(expected, PRECISION_MACHINE));

    // Check normalization
    EXPECT_NEAR(q.getNorm(), decimal(1), PRECISION_MACHINE);

    // Check that converting back to matrix gives the original
    Matrix3x3 rot = q.getRotationMatrix();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_NEAR(rot(i, j), m(i, j), PRECISION_MACHINE);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 2) Utilities
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ConjugateAndNormalize)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    Quaternion conjugateQ = q.getConjugate();
    Quaternion normalizeQ = conjugateQ.getNormalize();
    q.conjugate();
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(decimal(-1), decimal(-2), decimal(-3)));
    EXPECT_EQ(conjugateQ.getImaginaryPart(), Vector3D(decimal(-1), decimal(-2), decimal(-3)));
    q.normalize();
    EXPECT_TRUE(q.isUnit());
    EXPECT_TRUE(normalizeQ.isUnit());
}
TEST(Quaternion_Test, Inverse)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    q.normalize();
    Quaternion orig = q;

    // In-place inverse twice should return to original
    q.inverse();
    q.inverse();
    EXPECT_TRUE(q.approxEqual(orig, PRECISION_MACHINE));

    Quaternion inv = orig.getInverse();
    q.inverse();
    EXPECT_EQ(inv, q);
}
TEST(Quaternion_Test, Getters)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_NEAR(q.getNormSquare(),
                decimal(1) * decimal(1) + decimal(2) * decimal(2) + decimal(3) * decimal(3) +
                    decimal(4) * decimal(4),
                PRECISION_MACHINE);
    EXPECT_NEAR(q.getNorm(),
                std::sqrt(decimal(1) * decimal(1) + decimal(2) * decimal(2) + decimal(3) * decimal(3) +
                          decimal(4) * decimal(4)),
                PRECISION_MACHINE);
    Quaternion idenity = q.getIdentity();
    EXPECT_DECIMAL_EQ(idenity.getRealPart(), decimal(1));
    EXPECT_TRUE(idenity.getImaginaryPart() == Vector3D());
    Quaternion zero = q.getZero();
    EXPECT_DECIMAL_EQ(zero.getRealPart(), decimal(0));
    EXPECT_TRUE(zero.getImaginaryPart() == Vector3D());
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 3) Setters
// ——————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————
// 4) Property Checks
// ——————————————————————————————————————————————————————————————————————————
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
    EXPECT_TRUE(q.isInvertible()); // isInvertible returns true if norm==0
}
TEST(Quaternion_Test, IsUnit)
{
    Quaternion q(decimal(1), decimal(0), decimal(0), decimal(0));
    EXPECT_TRUE(q.isUnit());

    q.setAllValues(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_FALSE(q.isUnit());
}
TEST(Quaternion_Test, IsFinite)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_TRUE(q.isFinite());

    // Test with NaN
    Quaternion q_nan(decimal(NAN), decimal(2), decimal(3), decimal(4));
    EXPECT_FALSE(q_nan.isFinite());

    // Test with Inf
    Quaternion q_inf(decimal(INFINITY), decimal(2), decimal(3), decimal(4));
    EXPECT_FALSE(q_inf.isFinite());
}
TEST(Quaternion_Test, isInvertible)
{
    Quaternion q(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_TRUE(q.isInvertible()); // Non-zero norm means it's invertible

    q.setToZero();
    EXPECT_FALSE(q.isInvertible()); // Zero norm means it's not invertible
}
TEST(Quaternion_Test, IsOrthogonal)
{
    Quaternion q(decimal(1), decimal(0), decimal(0), decimal(0));
    EXPECT_TRUE(q.isOrthogonal()); // Identity quaternion is orthogonal

    // Non-identity quaternion is not orthogonal
    q.setAllValues(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_FALSE(q.isOrthogonal());
    // Check with a known orthogonal quaternion
    Quaternion orthogonalQ(decimal(0), decimal(1), decimal(0), decimal(0));
    EXPECT_TRUE(orthogonalQ.isOrthogonal());
}
TEST(Quaternion_Test, IsNormalized)
{
    Quaternion q(decimal(1), decimal(0), decimal(0), decimal(0));
    EXPECT_TRUE(q.isNormalized()); // Identity quaternion is normalized

    // Non-identity quaternion is not normalized
    q.setAllValues(decimal(1), decimal(2), decimal(3), decimal(4));
    EXPECT_FALSE(q.isNormalized());

    // Check with a normalized quaternion
    Quaternion normalizedQ = q.getNormalize();
    EXPECT_TRUE(normalizedQ.isNormalized());
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 5) Quaternion Operations
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, DotAndCrossProduct)
{
    Quaternion q1(decimal(1), decimal(0), decimal(0), decimal(1));
    Quaternion q2(decimal(0), decimal(1), decimal(0), decimal(1));
    decimal    dot = q1.dotProduct(q2);
    EXPECT_DECIMAL_EQ(dot, decimal(1)); // 1*0 + 0*1 + 0*0 + 1*1 = 1
    decimal dot_alt = dotProduct(q1, q2);
    EXPECT_EQ(dotProduct(q1, q2), dot);

    Quaternion cross = q1.crossProduct(q2);
    EXPECT_EQ(cross, Quaternion(decimal(1), decimal(1), decimal(1), decimal(1)));
    EXPECT_EQ(crossProduct(q1, q2), cross);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 6) Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
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

    EXPECT_TRUE(a.approxEqual(b, PRECISION_MACHINE));
    EXPECT_FALSE(a.approxEqual(c, PRECISION_MACHINE));
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 7) Element Access
// ——————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————
// 8) In-place Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————
// 9) Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————
// 10) Stream Output
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, StreamOutput)
{
    Quaternion        q(decimal(1), decimal(2), decimal(3), decimal(4));
    std::stringstream ss;
    ss << q;
    EXPECT_TRUE(ss.str().find("(") != std::string::npos);
    EXPECT_TRUE(ss.str().find(")") != std::string::npos);
}
