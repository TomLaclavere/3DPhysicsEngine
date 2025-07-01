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
    EXPECT_DECIMAL_EQ(q.getRealPart(), 0_d);
    EXPECT_TRUE(q.getImaginaryPart() == Vector3D());
}
TEST(Quaternion_Test, ValueConstructor)
{
    Quaternion q(1_d, 2_d, 3_d, 4_d);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 4_d);
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(1_d, 2_d, 3_d));
}
TEST(Quaternion_Test, VectorAndScalarConstructor)
{
    // Vector scalar
    Vector3D   v(1_d, 2_d, 3_d);
    Quaternion q(v, 4_d);
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 4_d);
    // Scalar vector
    Vector3D   v_bis(1_d, 2_d, 3_d);
    Quaternion q_bis(4_d, v_bis);
    EXPECT_EQ(q_bis.getImaginaryPart(), v_bis);
    EXPECT_DECIMAL_EQ(q_bis.getRealPart(), 4_d);
}
TEST(Quaternion_Test, FromEulerAngles_ProducesExpectedRotation)
{
    // 90 degrees (pi/2) rotation about X axis
    decimal    angleX = decimal(M_PI) / 2_d;
    decimal    angleY = 0_d;
    decimal    angleZ = 0_d;
    Quaternion q(angleX, angleY, angleZ);

    // The expected quaternion for 90deg about X is (sqrt(0.5), 0, 0, sqrt(0.5))
    decimal s = std::sin(angleX / 2_d);
    decimal c = std::cos(angleX / 2_d);
    EXPECT_TRUE(q.approxEqual(Quaternion(s, 0_d, 0_d, c), PRECISION_MACHINE));

    // Check normalization
    EXPECT_NEAR(q.getNorm(), 1_d, PRECISION_MACHINE);

    // Check rotation matrix matches expected
    Matrix3x3 expected(1_d, 0_d, 0_d, 0_d, 0_d, -1_d, 0_d, 1_d, 0_d);
    Matrix3x3 rot = q.getRotationMatrix();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_NEAR(rot(i, j), expected(i, j), PRECISION_MACHINE);
}
TEST(Quaternion_Test, FromMatrix_ProducesExpectedQuaternion)
{
    // 90 degrees (pi/2) rotation about X axis
    decimal   angle = decimal(M_PI) / 2;
    Matrix3x3 m(1_d, 0_d, 0_d, 0_d, std::cos(angle), -std::sin(angle), 0_d, std::sin(angle), std::cos(angle));
    Quaternion q(m);

    // The expected quaternion for 90deg about X is (sqrt(0.5), 0, 0, sqrt(0.5))
    decimal    s = std::sin(angle / 2);
    decimal    c = std::cos(angle / 2);
    Quaternion expected(s, 0_d, 0_d, c);

    EXPECT_TRUE(q.approxEqual(expected, PRECISION_MACHINE));

    // Check normalization
    EXPECT_NEAR(q.getNorm(), 1_d, PRECISION_MACHINE);

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
    Quaternion q(1_d, 2_d, 3_d, 4_d);
    Quaternion conjugateQ = q.getConjugate();
    Quaternion normalizeQ = conjugateQ.getNormalize();
    q.conjugate();
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(-1_d, -2_d, -3_d));
    EXPECT_EQ(conjugateQ.getImaginaryPart(), Vector3D(-1_d, -2_d, -3_d));
    q.normalize();
    EXPECT_TRUE(q.isUnit());
    EXPECT_TRUE(normalizeQ.isUnit());
}
TEST(Quaternion_Test, Inverse)
{
    Quaternion q(1_d, 2_d, 3_d, 4_d);
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
    Quaternion q(1_d, 2_d, 3_d, 4_d);
    EXPECT_NEAR(q.getNormSquare(), 1_d * 1_d + 2_d * 2_d + 3_d * 3_d + 4_d * 4_d, PRECISION_MACHINE);
    EXPECT_NEAR(q.getNorm(), std::sqrt(1_d * 1_d + 2_d * 2_d + 3_d * 3_d + 4_d * 4_d), PRECISION_MACHINE);
    Quaternion idenity = q.getIdentity();
    EXPECT_DECIMAL_EQ(idenity.getRealPart(), 1_d);
    EXPECT_TRUE(idenity.getImaginaryPart() == Vector3D());
    Quaternion zero = q.getZero();
    EXPECT_DECIMAL_EQ(zero.getRealPart(), 0_d);
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
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
    EXPECT_EQ(q.getImaginaryPart(), Vector3D(1_d, 2_d, 3_d));
    EXPECT_DECIMAL_EQ(q.getRealPart(), 4_d);

    Vector3D v(5_d, 6_d, 7_d);
    q.setAllValues(v, 8_d);
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 8_d);

    q.setAllValues(9_d, v);
    EXPECT_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 9_d);

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

    // Unitary
    q.setAllValues(1_d, 0_d, 0_d, 0_d);
    EXPECT_TRUE(q.isUnit());
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q.isUnit());

    // Finite
    EXPECT_TRUE(q.isFinite());
    // Test with NaN
    Quaternion q_nan(NAN, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q_nan.isFinite());
    // Test with Inf
    Quaternion q_inf(INFINITY, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q_inf.isFinite());

    // Invertible
    EXPECT_TRUE(q.isInvertible()); // Non-zero norm means it's invertible
    q.setToZero();
    EXPECT_FALSE(q.isInvertible()); // Zero norm means it's not invertible

    // Orthogonal
    q.setAllValues(1_d, 0_d, 0_d, 0_d);
    EXPECT_TRUE(q.isOrthogonal()); // Identity quaternion is orthogonal
    // Non-identity quaternion is not orthogonal
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q.isOrthogonal());
    // Check with a known orthogonal quaternion
    Quaternion orthogonalQ(0_d, 1_d, 0_d, 0_d);
    EXPECT_TRUE(orthogonalQ.isOrthogonal());

    // Normalized
    q.setAllValues(1_d, 0_d, 0_d, 0_d);
    EXPECT_TRUE(q.isNormalized()); // Identity quaternion is normalized
    // Non-identity quaternion is not normalized
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
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
    Quaternion q1(1_d, 0_d, 0_d, 1_d);
    Quaternion q2(0_d, 1_d, 0_d, 1_d);
    decimal    dot = q1.dotProduct(q2);
    EXPECT_DECIMAL_EQ(dot, 1_d); // 1*0 + 0*1 + 0*0 + 1*1 = 1
    decimal dot_alt = dotProduct(q1, q2);
    EXPECT_EQ(dotProduct(q1, q2), dot);

    Quaternion cross = q1.crossProduct(q2);
    EXPECT_EQ(cross, Quaternion(1_d, 1_d, 1_d, 1_d));
    EXPECT_EQ(crossProduct(q1, q2), cross);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 6) Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ComparisonOperators)
{
    Quaternion a(1_d, 2_d, 3_d, 4_d), b(1_d, 2_d, 3_d, 4_d), c(2_d, 3_d, 4_d, 5_d);

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
    Quaternion q(1_d, 2_d, 3_d, 4_d);

    EXPECT_DECIMAL_EQ(q(0_d), 1_d);
    EXPECT_DECIMAL_EQ(q(1_d), 2_d);
    EXPECT_DECIMAL_EQ(q(2_d), 3_d);

    q(0_d) = 10_d;
    EXPECT_DECIMAL_EQ(q(0_d), 10_d);

    EXPECT_DECIMAL_EQ(q[0_d], 10_d);
    q[1_d] = 20_d;
    EXPECT_DECIMAL_EQ(q[1_d], 20_d);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 8) In-place Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, InPlaceArithmeticOperators)
{
    Quaternion q(1_d, 2_d, 3_d, 4_d);
    Quaternion orig = q;

    q += Quaternion(1_d, 1_d, 1_d, 1_d);
    EXPECT_EQ(q, Quaternion(2_d, 3_d, 4_d, 5_d));

    q -= Quaternion(1_d, 1_d, 1_d, 1_d);
    EXPECT_EQ(q, orig);

    q *= Quaternion(2_d, 2_d, 2_d, 2_d);
    EXPECT_EQ(q, Quaternion(2_d, 4_d, 6_d, 8_d));

    q /= Quaternion(2_d, 2_d, 2_d, 2_d);
    EXPECT_EQ(q, orig);

    q += 1_d;
    EXPECT_EQ(q, Quaternion(2_d, 3_d, 4_d, 5_d));

    q -= 1_d;
    EXPECT_EQ(q, orig);

    q *= 2_d;
    EXPECT_EQ(q, Quaternion(2_d, 4_d, 6_d, 8_d));

    q /= 2_d;
    EXPECT_EQ(q, orig);
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 9) Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, ArithmeticOperators)
{
    Quaternion q1(1_d, 2_d, 3_d, 4_d);
    Quaternion q2(4_d, 5_d, 6_d, 7_d);

    Quaternion sum = q1 + q2;
    EXPECT_EQ(sum, Quaternion(1_d + 4_d, 2_d + 5_d, 3_d + 6_d, 4_d + 7_d));

    Quaternion diff = q1 - q2;
    EXPECT_EQ(diff, Quaternion(1_d - 4_d, 2_d - 5_d, 3_d - 6_d, 4_d - 7_d));

    Quaternion prod = q1 * q2;
    EXPECT_EQ(prod, Quaternion(1_d * 4_d, 2_d * 5_d, 3_d * 6_d, 4_d * 7_d));

    Quaternion quot = q2 / q1;
    EXPECT_EQ(quot, Quaternion(4_d / 1_d, 5_d / 2_d, 6_d / 3_d, 7_d / 4_d));

    Quaternion scalarSum = q1 + 2_d;
    EXPECT_EQ(scalarSum, Quaternion(1_d + 2_d, 2_d + 2_d, 3_d + 2_d, 4_d + 2_d));

    Quaternion scalarProd = q1 * 2_d;
    EXPECT_EQ(scalarProd, Quaternion(1_d * 2_d, 2_d * 2_d, 3_d * 2_d, 4_d * 2_d));
}

// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
// 10) Stream Output
// ——————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————
TEST(Quaternion_Test, StreamOutput)
{
    Quaternion        q(1_d, 2_d, 3_d, 4_d);
    std::stringstream ss;
    ss << q;
    EXPECT_TRUE(ss.str().find("(") != std::string::npos);
    EXPECT_TRUE(ss.str().find(")") != std::string::npos);
}
