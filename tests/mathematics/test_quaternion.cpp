#include "mathematics/common.hpp"
#include "mathematics/matrix.hpp"
#include "mathematics/quaternion.hpp"
#include "mathematics/vector.hpp"
#include "test_functions.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
//  Constructors and Getters
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, DefaultConstructorZeroes)
{
    Quaternion3D q;
    EXPECT_DECIMAL_EQ(q.getRealPart(), 0_d);
    EXPECT_TRUE(q.getImaginaryPart() == Vector3D());
}
TEST(QuaternionTest, ValueConstructor)
{
    Quaternion3D q(1_d, 2_d, 3_d, 4_d);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 4_d);
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(1_d, 2_d, 3_d));
}
TEST(QuaternionTest, VectorAndScalarConstructor)
{
    // Vector scalar
    Vector3D     v(1_d, 2_d, 3_d);
    Quaternion3D q(v, 4_d);
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 4_d);
    // Scalar vector
    Vector3D     v_bis(1_d, 2_d, 3_d);
    Quaternion3D q_bis(4_d, v_bis);
    EXPECT_VECTOR_EQ(q_bis.getImaginaryPart(), v_bis);
    EXPECT_DECIMAL_EQ(q_bis.getRealPart(), 4_d);
}
TEST(QuaternionTest, FromEulerAngles)
{
    // 90 degrees (pi/2) rotation about X axis
    decimal      angleX = decimal(M_PI) / 2_d;
    decimal      angleY = 0_d;
    decimal      angleZ = 0_d;
    Quaternion3D q(angleX, angleY, angleZ);
    Quaternion3D q_(Vector3D(angleX, angleY, angleZ));

    // The expected Quaternion3D for 90deg about X is (sqrt(0.5), 0, 0, sqrt(0.5))
    decimal s = std::sin(angleX / 2_d);
    decimal c = std::cos(angleX / 2_d);
    EXPECT_QUATERNION_EQ(q, Quaternion3D(s, 0_d, 0_d, c));

    // Check normalization
    EXPECT_DECIMAL_EQ(q.getNorm(), 1_d);
    EXPECT_DECIMAL_EQ(q_.getNorm(), 1_d);

    // Check rotation matrix matches expected
    Matrix3x3 expected(1_d, 0_d, 0_d, 0_d, 0_d, -1_d, 0_d, 1_d, 0_d);
    Matrix3x3 rot  = q.getRotationMatrix();
    Matrix3x3 rot_ = q_.getRotationMatrix();
    EXPECT_MATRIX_EQ(rot, expected);
    EXPECT_MATRIX_EQ(rot_, expected);
}
TEST(QuaternionTest, FromMatrix)
{
    // =====================
    // Branch 1: trace > 0 (small rotations)
    // =====================
    {
        decimal   angle = M_PI / 4_d; // 45° around Z
        Matrix3x3 R;
        R(0, 0) = std::cos(angle);
        R(0, 1) = -std::sin(angle);
        R(1, 0) = std::sin(angle);
        R(1, 1) = std::cos(angle);
        R(2, 2) = 1_d;

        Quaternion3D q(R);
        decimal      half = angle / 2_d;
        EXPECT_DECIMAL_EQ(q.getRealPart(), std::cos(half));
        EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(0_d, 0_d, std::sin(half)));
        EXPECT_MATRIX_EQ(q.getRotationMatrix(), R);
    }

    // ======================================================
    // Branch 2: trace <= 0, m(0,0) largest
    // ======================================================
    {
        // 120° rotation around X axis - this gives cleaner math
        decimal   angle = 2_d * M_PI / 3_d; // 120°
        Matrix3x3 R;
        R(0, 0) = 1_d;
        R(1, 1) = std::cos(angle);  // cos(120°) = -0.5
        R(1, 2) = -std::sin(angle); // sin(120°) = √3/2 ≈ 0.8660254
        R(2, 1) = std::sin(angle);
        R(2, 2) = std::cos(angle);

        Quaternion3D q(R);
        decimal      half = angle / 2_d;                                            // 60°
        EXPECT_DECIMAL_EQ(q.getRealPart(), std::cos(half));                         // cos(60°) = 0.5
        EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(std::sin(half), 0_d, 0_d)); // sin(60°) = √3/2
        EXPECT_MATRIX_EQ(q.getRotationMatrix(), R);
    }

    // ======================================================
    // Branch 3: trace <= 0, m(1,1) largest
    // ======================================================
    {
        // 120° rotation around Y axis
        decimal   angle = 2_d * M_PI / 3_d; // 120°
        Matrix3x3 R;
        R(1, 1) = 1_d;
        R(0, 0) = std::cos(angle);  // -0.5
        R(0, 2) = std::sin(angle);  // 0.8660254
        R(2, 0) = -std::sin(angle); // -0.8660254
        R(2, 2) = std::cos(angle);  // -0.5

        Quaternion3D q(R);
        decimal      half = angle / 2_d; // 60°
        EXPECT_DECIMAL_EQ(q.getRealPart(), std::cos(half));
        EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(0_d, std::sin(half), 0_d));
        EXPECT_MATRIX_EQ(q.getRotationMatrix(), R);
    }

    // ======================================================
    // Branch 4: trace <= 0, m(2,2) largest
    // ======================================================
    {
        // 120° rotation around Z axis
        decimal   angle = 2_d * M_PI / 3_d; // 120°
        Matrix3x3 R;
        R(2, 2) = 1_d;
        R(0, 0) = std::cos(angle);  // -0.5
        R(0, 1) = -std::sin(angle); // -0.8660254
        R(1, 0) = std::sin(angle);  // 0.8660254
        R(1, 1) = std::cos(angle);  // -0.5

        Quaternion3D q(R);
        decimal      half = angle / 2_d; // 60°
        EXPECT_DECIMAL_EQ(q.getRealPart(), std::cos(half));
        EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(0_d, 0_d, std::sin(half)));
        EXPECT_MATRIX_EQ(q.getRotationMatrix(), R);
    }

    // ======================================================
    // Edge case: trace = -1 (180° rotation)
    // ======================================================
    {
        decimal   angle = M_PI; // 180°
        Matrix3x3 R;
        R(0, 0) = 1_d;
        R(1, 1) = std::cos(angle);  // -1
        R(1, 2) = -std::sin(angle); // 0
        R(2, 1) = std::sin(angle);  // 0
        R(2, 2) = std::cos(angle);  // -1

        Quaternion3D q(R);
        // For 180° rotation around X, quaternion should be [0, 1, 0, 0]
        EXPECT_DECIMAL_EQ(q.getRealPart(), 0_d);
        EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(1_d, 0_d, 0_d));
        EXPECT_MATRIX_EQ(q.getRotationMatrix(), R);
    }
}

// ——————————————————————————————————————————————————————————————————————————
//  Utilities
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, ConjugateAndNormalize)
{
    Quaternion3D q(1_d, 2_d, 3_d, 4_d);
    Quaternion3D conjugateQ = q.getConjugate();
    Quaternion3D normalizeQ = conjugateQ.getNormalize();
    q.conjugate();
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(-1_d, -2_d, -3_d));
    EXPECT_VECTOR_EQ(conjugateQ.getImaginaryPart(), Vector3D(-1_d, -2_d, -3_d));
    q.normalize();
    EXPECT_TRUE(q.isUnit());
    EXPECT_TRUE(normalizeQ.isUnit());

    q.setToNull();
    EXPECT_QUATERNION_EQ(q, Quaternion3D());
}
TEST(QuaternionTest, Inverse)
{
    Quaternion3D q(1_d, 2_d, 3_d, 4_d);
    q.normalize();
    Quaternion3D orig = q;

    // In-place inverse twice should return to original
    q.inverse();
    q.inverse();
    EXPECT_QUATERNION_EQ(q, orig);

    Quaternion3D inv = orig.getInverse();
    q.inverse();
    EXPECT_QUATERNION_EQ(inv, q);
}
TEST(QuaternionTest, Getters)
{
    Quaternion3D q(1_d, 2_d, 3_d, 4_d);
    EXPECT_DECIMAL_EQ(q.getNormSquare(), 1_d * 1_d + 2_d * 2_d + 3_d * 3_d + 4_d * 4_d);
    EXPECT_DECIMAL_EQ(q.getNorm(), std::sqrt(1_d * 1_d + 2_d * 2_d + 3_d * 3_d + 4_d * 4_d));
    Quaternion3D idenity = q.getIdentity();
    EXPECT_DECIMAL_EQ(idenity.getRealPart(), 1_d);
    EXPECT_TRUE(idenity.getImaginaryPart() == Vector3D());
    Quaternion3D zero = q.getNull();
    EXPECT_DECIMAL_EQ(zero.getRealPart(), 0_d);
    EXPECT_TRUE(zero.getImaginaryPart() == Vector3D());
}

// ——————————————————————————————————————————————————————————————————————————
//  Setters
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, Setters)
{
    Quaternion3D q;
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(1_d, 2_d, 3_d));
    EXPECT_DECIMAL_EQ(q.getRealPart(), 4_d);

    Vector3D v(5_d, 6_d, 7_d);
    q.setAllValues(v, 8_d);
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 8_d);

    q.setAllValues(9_d, v);
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), v);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 9_d);

    q.setToNull();
    EXPECT_TRUE(q.isZero());

    q.setToIdentity();
    EXPECT_TRUE(q.isIdentity());
}

TEST(QuaternionTest, SetAllValues)
{
    Quaternion3D q;
    // From scalar ( = real part)
    q.setRealPart(0_d);
    EXPECT_DECIMAL_EQ(q.getRealPart(), 0_d);

    // From vector ( = imaginary part)
    q.setImaginaryPart(1_d, -4_d, 14_d);
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(1_d, -4_d, 14_d));

    q.setImaginaryPart(Vector3D(-2_d, 0_d, 3.14_d));
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(-2_d, 0_d, 3.14_d));

    // From rotation matrix
    decimal   angle = M_PI / 2_d;
    Matrix3x3 R;
    R(0, 0) = std::cos(angle);
    R(0, 1) = -std::sin(angle);
    R(1, 0) = std::sin(angle);
    R(1, 1) = std::cos(angle);
    R(2, 2) = 1_d;
    q.setAllValues(R);
    decimal half = angle / 2_d;
    EXPECT_DECIMAL_EQ(q.getRealPart(), std::cos(half));
    EXPECT_VECTOR_EQ(q.getImaginaryPart(), Vector3D(0_d, 0_d, std::sin(half)));
    EXPECT_MATRIX_EQ(q.getRotationMatrix(), R);

    // From Euler angles
    Quaternion3D q_;
    decimal      angleX = decimal(M_PI) / 2_d;
    decimal      angleY = 0_d;
    decimal      angleZ = 0_d;
    q.setAllValues(angleX, angleY, angleZ);
    q_.setAllValues(Vector3D(angleX, angleY, angleZ));

    // The expected Quaternion3D for 90deg about X is (sqrt(0.5), 0, 0, sqrt(0.5))
    decimal s = std::sin(angleX / 2_d);
    decimal c = std::cos(angleX / 2_d);
    EXPECT_QUATERNION_EQ(q, Quaternion3D(s, 0_d, 0_d, c));
    EXPECT_QUATERNION_EQ(q_, Quaternion3D(s, 0_d, 0_d, c));
}

// ——————————————————————————————————————————————————————————————————————————
//  Property Checks
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, PropertyChecks)
{
    Quaternion3D q;
    q.setToNull();
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
    Quaternion3D q_nan(NAN, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q_nan.isFinite());
    // Test with Inf
    Quaternion3D q_inf(INFINITY, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q_inf.isFinite());

    // Invertible
    EXPECT_TRUE(q.isInvertible()); // Non-zero norm means it's invertible
    q.setToNull();
    EXPECT_FALSE(q.isInvertible()); // Zero norm means it's not invertible

    // Orthogonal
    q.setAllValues(1_d, 0_d, 0_d, 0_d);
    EXPECT_TRUE(q.isOrthogonal()); // Identity Quaternion3D is orthogonal
    // Non-identity Quaternion3D is not orthogonal
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q.isOrthogonal());
    // Check with a known orthogonal Quaternion3D
    Quaternion3D orthogonalQ(0_d, 1_d, 0_d, 0_d);
    EXPECT_TRUE(orthogonalQ.isOrthogonal());

    // Normalized
    q.setAllValues(1_d, 0_d, 0_d, 0_d);
    EXPECT_TRUE(q.isNormalized()); // Identity Quaternion3D is normalized
    // Non-identity Quaternion3D is not normalized
    q.setAllValues(1_d, 2_d, 3_d, 4_d);
    EXPECT_FALSE(q.isNormalized());
    // Check with a normalized Quaternion3D
    Quaternion3D normalizedQ = q.getNormalize();
    EXPECT_TRUE(normalizedQ.isNormalized());
}

// ——————————————————————————————————————————————————————————————————————————
//  Quaternion3D Operations
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, DotAndCrossProduct)
{
    Quaternion3D q1(1_d, 0_d, 0_d, 1_d);
    Quaternion3D q2(0_d, 1_d, 0_d, 1_d);
    decimal      dot = q1.dotProduct(q2);
    EXPECT_DECIMAL_EQ(dot, 1_d); // 1*0 + 0*1 + 0*0 + 1*1 = 1
    decimal dot_alt = dotProduct(q1, q2);
    EXPECT_DECIMAL_EQ(dotProduct(q1, q2), dot);

    Quaternion3D cross = q1.crossProduct(q2);
    EXPECT_QUATERNION_EQ(cross, Quaternion3D(1_d, 1_d, 1_d, 1_d));
    EXPECT_QUATERNION_EQ(crossProduct(q1, q2), cross);
}

// ——————————————————————————————————————————————————————————————————————————
//  Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, ComparisonOperators)
{
    Quaternion3D a(1_d, 2_d, 3_d, 4_d), b(1_d, 2_d, 3_d, 4_d), c(2_d, 3_d, 4_d, 5_d);

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
//  Element Access
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, ElementAccess)
{
    Quaternion3D       q(1_d, 2_d, 3_d, 4_d);
    const Quaternion3D cq(5_d, 0_d, -5_d, 3.14_d);

    // operator()(int) const and non-const
    EXPECT_DECIMAL_EQ(q(0_d), 1_d);
    EXPECT_DECIMAL_EQ(q(1_d), 2_d);
    EXPECT_DECIMAL_EQ(q(2_d), 3_d);

    EXPECT_DECIMAL_EQ(cq(0_d), 5_d);
    EXPECT_DECIMAL_EQ(cq(1_d), 0_d);
    EXPECT_DECIMAL_EQ(cq(2_d), -5_d);

    // operator[] const and non-const
    EXPECT_DECIMAL_EQ(q[0_d], 1_d);
    EXPECT_DECIMAL_EQ(q[1_d], 2_d);
    EXPECT_DECIMAL_EQ(q[2_d], 3_d);

    EXPECT_DECIMAL_EQ(cq[0_d], 5_d);
    EXPECT_DECIMAL_EQ(cq[1_d], 0_d);
    EXPECT_DECIMAL_EQ(cq[2_d], -5_d);

    // operator at const and non-const
    EXPECT_DECIMAL_EQ(q.at(0_d), 1_d);
    EXPECT_DECIMAL_EQ(q.at(1_d), 2_d);
    EXPECT_DECIMAL_EQ(q.at(2_d), 3_d);

    EXPECT_DECIMAL_EQ(cq.at(0_d), 5_d);
    EXPECT_DECIMAL_EQ(cq.at(1_d), 0_d);
    EXPECT_DECIMAL_EQ(cq.at(2_d), -5_d);

    // Out-of-range checks
    EXPECT_THROW(q.at(-1_d), std::out_of_range);
    EXPECT_THROW(q.at(3_d), std::out_of_range);
    EXPECT_THROW(cq.at(-1_d), std::out_of_range);
    EXPECT_THROW(cq.at(3_d), std::out_of_range);

    // operator[] & () do not check the index, so I can't check out of range indices
}

// ——————————————————————————————————————————————————————————————————————————
//  In-place Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, InPlaceArithmeticOperators)
{
    Quaternion3D q(1_d, 2_d, 3_d, 4_d);
    Quaternion3D orig = q;

    q += Quaternion3D(1_d, 1_d, 1_d, 1_d);
    EXPECT_QUATERNION_EQ(q, Quaternion3D(2_d, 3_d, 4_d, 5_d));

    q -= Quaternion3D(1_d, 1_d, 1_d, 1_d);
    EXPECT_QUATERNION_EQ(q, orig);

    q *= Quaternion3D(2_d, 2_d, 2_d, 2_d);
    EXPECT_QUATERNION_EQ(q, Quaternion3D(2_d, 4_d, 6_d, 8_d));

    q /= Quaternion3D(2_d, 2_d, 2_d, 2_d);
    EXPECT_QUATERNION_EQ(q, orig);
    EXPECT_THROW(q /= Quaternion3D(0_d, 1_d, 2_d, -2_d), std::invalid_argument);
    EXPECT_THROW(q /= Quaternion3D(-2_d, 1_d, 2_d, 0_d), std::invalid_argument);

    q += 1_d;
    EXPECT_QUATERNION_EQ(q, Quaternion3D(2_d, 3_d, 4_d, 5_d));

    q -= 1_d;
    EXPECT_QUATERNION_EQ(q, orig);

    q *= 2_d;
    EXPECT_QUATERNION_EQ(q, Quaternion3D(2_d, 4_d, 6_d, 8_d));

    q /= 2_d;
    EXPECT_QUATERNION_EQ(q, orig);
    EXPECT_THROW(q /= 0_d, std::invalid_argument);

    EXPECT_QUATERNION_EQ(-q, Quaternion3D(-1_d, -2_d, -3_d, -4_d));
}

// ——————————————————————————————————————————————————————————————————————————
//  Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, ArithmeticOperators)
{
    Quaternion3D q1(1_d, 2_d, 3_d, 4_d);
    Quaternion3D q2(4_d, 5_d, 6_d, 7_d);

    // Quaternion x Quaternion
    Quaternion3D sum = q1 + q2;
    EXPECT_QUATERNION_EQ(sum, Quaternion3D(1_d + 4_d, 2_d + 5_d, 3_d + 6_d, 4_d + 7_d));

    Quaternion3D diff = q1 - q2;
    EXPECT_QUATERNION_EQ(diff, Quaternion3D(1_d - 4_d, 2_d - 5_d, 3_d - 6_d, 4_d - 7_d));

    Quaternion3D prod = q1 * q2;
    EXPECT_QUATERNION_EQ(prod, Quaternion3D(1_d * 4_d, 2_d * 5_d, 3_d * 6_d, 4_d * 7_d));

    Quaternion3D quot = q2 / q1;
    EXPECT_QUATERNION_EQ(quot, Quaternion3D(4_d / 1_d, 5_d / 2_d, 6_d / 3_d, 7_d / 4_d));
    EXPECT_THROW(q1 / Quaternion3D(0_d, 1_d, 2_d, -2_d), std::invalid_argument);
    EXPECT_THROW(q2 / Quaternion3D(-2_d, 1_d, 2_d, 0_d), std::invalid_argument);

    // Quaternion x Scalar
    Quaternion3D scalarSum = q1 + 2_d;
    EXPECT_QUATERNION_EQ(scalarSum, Quaternion3D(1_d + 2_d, 2_d + 2_d, 3_d + 2_d, 4_d + 2_d));

    Quaternion3D scalarSoustrac = q1 - 3_d;
    EXPECT_QUATERNION_EQ(scalarSoustrac, Quaternion3D(-2_d, -1_d, 0_d, 1_d));

    Quaternion3D scalarProd = q1 * 2_d;
    EXPECT_QUATERNION_EQ(scalarProd, Quaternion3D(1_d * 2_d, 2_d * 2_d, 3_d * 2_d, 4_d * 2_d));

    Quaternion3D scalarDiv = q1 / 2_d;
    EXPECT_QUATERNION_EQ(scalarDiv, Quaternion3D(0.5_d, 1_d, 1.5_d, 2_d));
    EXPECT_THROW(q1 / 0_d, std::invalid_argument);

    // Scalar x Quaternion
    EXPECT_QUATERNION_EQ(2_d + q1, Quaternion3D(3_d, 4_d, 5_d, 6_d));
    EXPECT_QUATERNION_EQ(2_d - q1, Quaternion3D(1_d, 0_d, -1_d, -2_d));
    EXPECT_QUATERNION_EQ(2_d * q1, Quaternion3D(2_d, 4_d, 6_d, 8_d));
    EXPECT_QUATERNION_EQ(2_d / q1, Quaternion3D(2_d, 1_d, 2_d / 3_d, 0.5_d));
    EXPECT_THROW(2_d / Quaternion3D(0_d, 1_d, 2_d, -2_d), std::invalid_argument);
    EXPECT_THROW(2_d / Quaternion3D(-2_d, 1_d, 2_d, 0_d), std::invalid_argument);
}

// ——————————————————————————————————————————————————————————————————————————
//  Stream Output
// ——————————————————————————————————————————————————————————————————————————
TEST(QuaternionTest, StreamOutput)
{
    Quaternion3D      q(1_d, 2_d, 3_d, 4_d);
    std::stringstream ss;
    ss << q;
    EXPECT_TRUE(ss.str().find("(") != std::string::npos);
    EXPECT_TRUE(ss.str().find(")") != std::string::npos);
}
