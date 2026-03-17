#include "mathematics/matrix.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
//  Constructors, getters and setters
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, Constructors)
{
    // Zero
    Matrix3x3 m;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), 0_d);

    // Nine Values
    Matrix3x3 m1(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m1(i, j), decimal(i * 3 + j + 1));

    // Three Vectors
    Vector3D  r1(1_d, 2_d, 3_d), r2(4_d, 5_d, 6_d), r3(7_d, 8_d, 9_d);
    Matrix3x3 m2(r1, r2, r3);
    EXPECT_VECTOR_EQ(m2.getRow(0), r1);
    EXPECT_VECTOR_EQ(m2.getRow(1), r2);
    EXPECT_VECTOR_EQ(m2.getRow(2), r3);
    EXPECT_VECTOR_EQ(m2.getColumn(0), Vector3D(1_d, 4_d, 7_d));
    EXPECT_VECTOR_EQ(m2.getColumn(1), Vector3D(2_d, 5_d, 8_d));
    EXPECT_VECTOR_EQ(m2.getColumn(2), Vector3D(3_d, 6_d, 9_d));

    // Copy constructor
    Matrix3x3 m3(m);
    EXPECT_MATRIX_EQ(m3, m);
}

TEST(Matrix3x3_Test, Setters)
{
    // Zero
    Matrix3x3 m;

    // Nine Values
    m.setAllValues(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), decimal(i * 3 + j + 1));

    // Diagonal from Vector
    m.setDiagonal(Vector3D(1_d, 2_d, 3_d));
    // Check diagonal elements are set correctly
    EXPECT_DECIMAL_EQ(m(0, 0), 1_d);
    EXPECT_DECIMAL_EQ(m(1, 1), 2_d);
    EXPECT_DECIMAL_EQ(m(2, 2), 3_d);
    // Check that off-diagonal elements remain unchanged
    EXPECT_DECIMAL_EQ(m(0, 1), 2_d);
    EXPECT_DECIMAL_EQ(m(0, 2), 3_d);
    EXPECT_DECIMAL_EQ(m(1, 0), 4_d);
    EXPECT_DECIMAL_EQ(m(1, 2), 6_d);
    EXPECT_DECIMAL_EQ(m(2, 0), 7_d);
    EXPECT_DECIMAL_EQ(m(2, 1), 8_d);

    // Three Vectors
    Vector3D r1(1_d, 2_d, 3_d), r2(4_d, 5_d, 6_d), r3(7_d, 8_d, 9_d);
    m.setAllValues(r1, r2, r3);
    EXPECT_VECTOR_EQ(m.getRow(0), r1);
    EXPECT_VECTOR_EQ(m.getRow(1), r2);
    EXPECT_VECTOR_EQ(m.getRow(2), r3);

    // Set from vectors
    Vector3D r(1_d, 2_d, 3_d), c(4_d, 5_d, 6_d), d(7_d, 8_d, 9_d);

    EXPECT_THROW(m.setRow(-1, r), std::out_of_range);
    EXPECT_THROW(m.setRow(3, r), std::out_of_range);
    m.setRow(0, r);
    EXPECT_VECTOR_EQ(m.getRow(0), r);

    EXPECT_THROW(m.setColumn(-1, r), std::out_of_range);
    EXPECT_THROW(m.setColumn(3, r), std::out_of_range);
    m.setColumn(1, c);
    EXPECT_VECTOR_EQ(m.getColumn(1), c);

    m.setDiagonal(d);
    EXPECT_VECTOR_EQ(m.getDiagonal(), d);

    // Identity
    m.setToIdentity();
    EXPECT_MATRIX_EQ(m, Matrix3x3(1_d, 0_d, 0_d, 0_d, 1_d, 0_d, 0_d, 0_d, 1_d));

    // Zero
    m.setToNull();
    EXPECT_MATRIX_EQ(m, Matrix3x3());

    // One Value
    m.setAllValues(-3.14_d);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), -3.14_d);

    // One Vector
    m.setAllValues(r);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), r[j]);

    // Matrix
    Matrix3x3 mTest(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    m.setAllValues(mTest);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_DECIMAL_EQ(m(i, j), decimal(i * 3 + j + 1));
}

// ——————————————————————————————————————————————————————————————————————————
//  Utilities
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, Utilities)
{
    Matrix3x3 mZero;
    mZero.setToNull();
    Matrix3x3 m(-1_d, 2_d, -3_d, 4_d, -5_d, 6_d, -7_d, 8_d, -9_d);

    // Determinant & Trace
    // det = -1*( -5* -9 - 6*8 ) - 2*( 4* -9 - 6* -7 ) + -3*( 4*8 - -5* -7 )
    //     = -1*(45 - 48) - 2*(-36 + 42) - 3*(32 - 35)
    //     = -1*(-3) - 2*(6) - 3*(-3)
    //     = 3 - 12 + 9 = 0
    EXPECT_DECIMAL_EQ(m.getDeterminant(), 0_d);
    EXPECT_DECIMAL_EQ(m.getTrace(), -1_d - 5_d - 9_d);

    // Identity
    Matrix3x3 idM = m.getIdentity();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (i == j)
                EXPECT_DECIMAL_EQ(idM(i, j), 1_d);
            else
                EXPECT_DECIMAL_EQ(idM(i, j), 0_d);

    // Absolute
    Matrix3x3 absM = m.getAbsolute();
    Matrix3x3 expectedAbs(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    EXPECT_MATRIX_EQ(absM, expectedAbs);

    m.absolute();
    EXPECT_MATRIX_EQ(absM, m);

    // Transpose
    Matrix3x3 transM = m.getTranspose();
    Matrix3x3 expectedTrans(1_d, 4_d, 7_d, 2_d, 5_d, 8_d, 3_d, 6_d, 9_d);
    EXPECT_MATRIX_EQ(transM, expectedTrans);

    m.transpose();
    EXPECT_MATRIX_EQ(transM, m);

    // Normalise
    Matrix3x3 normM = m.getNormalised();
    for (int i = 0; i < 3; ++i)
        EXPECT_NEAR(normM.getRow(i).getNorm(), 1_d, PRECISION_MACHINE);

    m.normalise();
    EXPECT_TRUE(normM.approxEqual(m, PRECISION_MACHINE));

    // Inverse
    Matrix3x3 invM(1_d, 2_d, 3_d, 0_d, 1_d, 4_d, 5_d, 6_d, 0_d);
    decimal   det = invM.getDeterminant();
    EXPECT_DECIMAL_EQ(det, 1_d); // Should be exactly 1

    Matrix3x3 inv = invM.getInverse();
    EXPECT_THROW(mZero.getInverse(), std::invalid_argument);
    // Exact inverse should be:
    // [-24, 18, 5]
    // [20, -15, -4]
    // [-5, 4, 1]
    Matrix3x3 expectedInv(-24_d, 18_d, 5_d, 20_d, -15_d, -4_d, -5_d, 4_d, 1_d);
    EXPECT_TRUE(inv.approxEqual(expectedInv, PRECISION_MACHINE));

    // Test that product is identity
    Matrix3x3 identity = invM.matrixProduct(inv);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (i == j)
            {
                EXPECT_NEAR(identity(i, j), 1_d, PRECISION_MACHINE);
            }
            else
            {
                EXPECT_NEAR(identity(i, j), 0_d, PRECISION_MACHINE);
            }
        }
    }

    invM.inverse();
    EXPECT_TRUE(invM.approxEqual(inv, PRECISION_MACHINE));
    EXPECT_FALSE(invM.approxEqual(transM, PRECISION_MACHINE));
}

// ——————————————————————————————————————————————————————————————————————————
//  Property Check
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, PropertyCheck)
{
    // Identity
    Matrix3x3 m;
    m.setToIdentity();
    EXPECT_TRUE(m.isIdentity());
    m(0, 1) = 1_d;
    EXPECT_FALSE(m.isIdentity());
    m.setToIdentity();
    m(1, 1) = 2_d;
    EXPECT_FALSE(m.isIdentity());

    // Zero
    m.setToNull();
    EXPECT_TRUE(m.isZero());
    m(1, 1) = 1_d;
    EXPECT_FALSE(m.isZero());

    // Finite
    m.setAllValues(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    EXPECT_TRUE(m.isFinite());
    m(1, 1) = std::numeric_limits<decimal>::infinity();
    EXPECT_FALSE(m.isFinite());

    // Diagonal
    m.setToNull();
    m(0, 0) = 1_d;
    m(1, 1) = 2_d;
    m(2, 2) = 3_d;
    EXPECT_TRUE(m.isDiagonal());
    m(0, 1) = 5_d;
    EXPECT_FALSE(m.isDiagonal());

    // Symmetric
    m.setAllValues(1_d, 2_d, 3_d, 2_d, 4_d, 5_d, 3_d, 5_d, 6_d);
    EXPECT_TRUE(m.isSymmetric());
    m(0, 2) = 7_d;
    EXPECT_FALSE(m.isSymmetric());

    // Invertible
    m.setAllValues(1_d, 2_d, 3_d, 0_d, 1_d, 4_d, 5_d, 6_d, 0_d);
    EXPECT_TRUE(m.isInvertible());
    m.setAllValues(1_d, 2_d, 3_d, 2_d, 4_d, 6_d, 3_d, 6_d, 9_d); // rank 1
    EXPECT_FALSE(m.isInvertible());

    // Orthogonal
    m.setToIdentity();
    EXPECT_TRUE(m.isOrthogonal());
    m(0, 1) = 1_d;
    EXPECT_FALSE(m.isOrthogonal());

    // Normalised
    m.setToIdentity();
    EXPECT_TRUE(m.isNormalised());
    m(0, 0) = 2_d;
    EXPECT_FALSE(m.isNormalised());
    // Norm = 0
    m.setToNull();
    EXPECT_FALSE(m.isNormalised());
}

// ——————————————————————————————————————————————————————————————————————————
//  Matrix Operations
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, MatrixProduct)
{
    // Matrix Product
    Matrix3x3 a(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    Matrix3x3 b(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 1_d);
    Matrix3x3 prod0     = a.matrixProduct(b);
    Matrix3x3 prod0_alt = matrixProduct(a, b);

    EXPECT_DECIMAL_EQ(prod0(0, 0), 30_d);
    EXPECT_DECIMAL_EQ(prod0_alt(0, 0), 30_d);
    EXPECT_DECIMAL_EQ(prod0(0, 1), 24_d);
    EXPECT_DECIMAL_EQ(prod0_alt(0, 1), 24_d);
    EXPECT_DECIMAL_EQ(prod0(0, 2), 18_d);
    EXPECT_DECIMAL_EQ(prod0_alt(0, 2), 18_d);
    EXPECT_DECIMAL_EQ(prod0(1, 0), 84_d);
    EXPECT_DECIMAL_EQ(prod0_alt(1, 0), 84_d);
    EXPECT_DECIMAL_EQ(prod0(1, 1), 69_d);
    EXPECT_DECIMAL_EQ(prod0_alt(1, 1), 69_d);
    EXPECT_DECIMAL_EQ(prod0(1, 2), 54_d);
    EXPECT_DECIMAL_EQ(prod0_alt(1, 2), 54_d);
    EXPECT_DECIMAL_EQ(prod0(2, 0), 138_d);
    EXPECT_DECIMAL_EQ(prod0_alt(2, 0), 138_d);
    EXPECT_DECIMAL_EQ(prod0(2, 1), 114_d);
    EXPECT_DECIMAL_EQ(prod0_alt(2, 1), 114_d);
    EXPECT_DECIMAL_EQ(prod0(2, 2), 90_d);
    EXPECT_DECIMAL_EQ(prod0_alt(2, 2), 90_d);

    // Matrix Vector Product
    Matrix3x3 m(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    Vector3D  v(1_d, 2_d, 3_d);
    Vector3D  prod1     = m.matrixVectorProduct(v);
    Vector3D  prod1_alt = matrixVectorProduct(m, v);

    EXPECT_DECIMAL_EQ(prod1[0], 14_d); // 1*1 + 2*2 + 3*3
    EXPECT_DECIMAL_EQ(prod1_alt[0], 14_d);
    EXPECT_DECIMAL_EQ(prod1[1], 32_d); // 4*1 + 5*2 + 6*3
    EXPECT_DECIMAL_EQ(prod1_alt[1], 32_d);
    EXPECT_DECIMAL_EQ(prod1[2], 50_d); // 7*1 + 8*2 + 9*3
    EXPECT_DECIMAL_EQ(prod1_alt[2], 50_d);

    // Vector Matrix Product
    Vector3D prod2     = m.vectorMatrixProduct(v);
    Vector3D prod2_alt = vectorMatrixProduct(v, m);

    EXPECT_DECIMAL_EQ(prod2[0], 30_d); // 1*1 + 2*4 + 3*7
    EXPECT_DECIMAL_EQ(prod2_alt[0], 30_d);
    EXPECT_DECIMAL_EQ(prod2[1], 36_d); // 1*2 + 2*5 + 3*8
    EXPECT_DECIMAL_EQ(prod2_alt[1], 36_d);
    EXPECT_DECIMAL_EQ(prod2[2], 42_d); // 1*3 + 2*6 + 3*9
    EXPECT_DECIMAL_EQ(prod2_alt[2], 42_d);
}

// ——————————————————————————————————————————————————————————————————————————
//  Comparison Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, ComparisonOperators)
{
    Matrix3x3 a(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    Matrix3x3 b(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 1_d);
    Matrix3x3 d(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 1_d);

    EXPECT_TRUE(b == d);
    EXPECT_FALSE(a == d);

    EXPECT_TRUE(a != d);
    EXPECT_FALSE(b != d);
}

// ——————————————————————————————————————————————————————————————————————————
//  Element Access Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, Mapping)
{
    Matrix3x3 m;
    EXPECT_DECIMAL_EQ(m.mapping(0, 0), 0);
    EXPECT_DECIMAL_EQ(m.mapping(1, 2), 5);
    EXPECT_DECIMAL_EQ(m.mapping(2, 2), 8);

    EXPECT_THROW(m.mapping(-1, 0), std::out_of_range);
    EXPECT_THROW(m.mapping(0, 4), std::out_of_range);
    EXPECT_THROW(m.mapping(3, 3), std::out_of_range);
}

TEST(Matrix3x3_Test, At)
{
    Matrix3x3       m(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    const Matrix3x3 cm(-5_d, 0_d, -3_d, 2_d, -1_d, 7_d, 6_d, -3_d, 10_d);

    // 2D at() access
    EXPECT_DECIMAL_EQ(m.at(0, 2), 3_d);
    EXPECT_DECIMAL_EQ(m.at(2, 2), 9_d);
    EXPECT_DECIMAL_EQ(cm.at(1, 1), -1_d);

    // 1D at() access
    EXPECT_DECIMAL_EQ(m.at(3), 4_d);
    EXPECT_DECIMAL_EQ(m.at(7), 8_d);
    EXPECT_DECIMAL_EQ(cm.at(2), -3_d);

    // Out-of-range checks
    EXPECT_THROW(m.at(3, 0), std::out_of_range);
    EXPECT_THROW(m.at(0, 3), std::out_of_range);
    EXPECT_THROW(m.at(-1, 0), std::out_of_range);
    EXPECT_THROW(m.at(0, -1), std::out_of_range);
    EXPECT_THROW(cm.at(3, 0), std::out_of_range);
    EXPECT_THROW(cm.at(0, 3), std::out_of_range);
    EXPECT_THROW(m.at(-1), std::out_of_range);
    EXPECT_THROW(m.at(9), std::out_of_range);
    EXPECT_THROW(cm.at(-1), std::out_of_range);
    EXPECT_THROW(cm.at(9), std::out_of_range);
}

TEST(Matrix3x3_Test, ElementAccess)
{
    Matrix3x3       m(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    const Matrix3x3 cm(-5_d, 0_d, -3_d, 2_d, -1_d, 7_d, 6_d, -3_d, 10_d);

    // operator() access
    EXPECT_DECIMAL_EQ(m(0, 0), 1_d);
    EXPECT_DECIMAL_EQ(m(2, 1), 8_d);
    EXPECT_DECIMAL_EQ(cm(0, 1), 0_d);

    m(1, 2) = 42.0_d;
    EXPECT_DECIMAL_EQ(m(1, 2), 42.0_d);

    // getRow() access
    Vector3D row = m.getRow(2);
    EXPECT_VECTOR_EQ(row, Vector3D(7_d, 8_d, 9_d));
    m.setRow(2, Vector3D(10_d, 11_d, 12_d));
    EXPECT_VECTOR_EQ(m.getRow(2), Vector3D(10_d, 11_d, 12_d));

    Vector3D row_cm = cm.getRow(1);
    EXPECT_VECTOR_EQ(row_cm, Vector3D(2_d, -1_d, 7_d));

    // Out-of-range checks
    EXPECT_THROW(m(3, 0), std::out_of_range);
    EXPECT_THROW(m(0, 3), std::out_of_range);
    EXPECT_THROW(m(-1, 0), std::out_of_range);
    EXPECT_THROW(m(0, -1), std::out_of_range);
    EXPECT_THROW(cm(3, 0), std::out_of_range);
    EXPECT_THROW(cm(0, 3), std::out_of_range);
}
// ——————————————————————————————————————————————————————————————————————————
//  Arithmetic Operators
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, InPlace)
{
    Matrix3x3 m(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    m += Matrix3x3(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 1_d);
    EXPECT_MATRIX_EQ(m, Matrix3x3(10_d, 10_d, 10_d, 10_d, 10_d, 10_d, 10_d, 10_d, 10_d));
    m -= Matrix3x3(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 1_d);
    EXPECT_MATRIX_EQ(m, Matrix3x3(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d));
    m *= Matrix3x3(2_d, 2_d, 2_d, 2_d, 2_d, 2_d, 2_d, 2_d, 2_d);
    EXPECT_MATRIX_EQ(m, Matrix3x3(2_d, 4_d, 6_d, 8_d, 10_d, 12_d, 14_d, 16_d, 18_d));
    m /= Matrix3x3(2_d, 2_d, 2_d, 2_d, 2_d, 2_d, 2_d, 2_d, 2_d);
    EXPECT_MATRIX_EQ(m, Matrix3x3(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d));
    m = -m;
    EXPECT_MATRIX_EQ(m, Matrix3x3(-1_d, -2_d, -3_d, -4_d, -5_d, -6_d, -7_d, -8_d, -9_d));

    // Check division by zero
    EXPECT_THROW(m /= Matrix3x3(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 0_d), std::invalid_argument);

    Vector3D v(1_d, 0_d, -3_d);
    m += v;
    EXPECT_MATRIX_EQ(m, Matrix3x3(0_d, -2_d, -6_d, -3_d, -5_d, -9_d, -6_d, -8_d, -12_d));
    m -= v;
    EXPECT_MATRIX_EQ(m, Matrix3x3(-1_d, -2_d, -3_d, -4_d, -5_d, -6_d, -7_d, -8_d, -9_d));
    m *= v;
    EXPECT_MATRIX_EQ(m, Matrix3x3(-1_d, -0_d, 9_d, -4_d, -0_d, 18_d, -7_d, -0_d, 27_d));
    EXPECT_THROW(m /= v, std::invalid_argument);
    m /= Vector3D(1_d, 2_d, -3_d);
    EXPECT_MATRIX_EQ(m, Matrix3x3(-1_d, -0_d, -3_d, -4_d, -0_d, -6_d, -7_d, -0_d, -9_d));

    EXPECT_THROW(m / 0_d, std::invalid_argument);
    m /= 2_d;
    EXPECT_MATRIX_EQ(m, Matrix3x3(-0.5_d, -0_d, -1.5_d, -2_d, -0_d, -3_d, -3.5_d, -0_d, -4.5_d));
}

TEST(Matrix3x3_Test, Free)
{
    Matrix3x3 m(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    Matrix3x3 n(9_d, 8_d, 7_d, 6_d, 5_d, 4_d, 3_d, 2_d, 1_d);

    // Matrix Arithmetic Operators
    EXPECT_MATRIX_EQ(m + n, Matrix3x3(10_d, 10_d, 10_d, 10_d, 10_d, 10_d, 10_d, 10_d, 10_d));
    EXPECT_MATRIX_EQ(m - n, Matrix3x3(-8_d, -6_d, -4_d, -2_d, 0_d, 2_d, 4_d, 6_d, 8_d));
    EXPECT_MATRIX_EQ(m * n, Matrix3x3(9_d, 16_d, 21_d, 24_d, 25_d, 24_d, 21_d, 16_d, 9_d));
    EXPECT_MATRIX_EQ(
        m / n, Matrix3x3(1_d / 9_d, 1_d / 4_d, 3_d / 7_d, 2_d / 3_d, 1_d, 3_d / 2_d, 7_d / 3_d, 4_d, 9_d));
    n(0, 2) = 0_d;
    EXPECT_THROW(m / n, std::invalid_argument);

    // Matrix Scalar Arithmetic Operators
    EXPECT_MATRIX_EQ(m + 5_d, Matrix3x3(6_d, 7_d, 8_d, 9_d, 10_d, 11_d, 12_d, 13_d, 14_d));
    EXPECT_MATRIX_EQ(m - 5_d, Matrix3x3(-4_d, -3_d, -2_d, -1_d, 0_d, 1_d, 2_d, 3_d, 4_d));
    EXPECT_MATRIX_EQ(m * 2_d, Matrix3x3(2_d, 4_d, 6_d, 8_d, 10_d, 12_d, 14_d, 16_d, 18_d));
    EXPECT_MATRIX_EQ(m / 2_d, Matrix3x3(0.5_d, 1.0_d, 1.5_d, 2.0_d, 2.5_d, 3.0_d, 3.5_d, 4.0_d, 4.5_d));
    EXPECT_MATRIX_EQ(5_d + m, Matrix3x3(6_d, 7_d, 8_d, 9_d, 10_d, 11_d, 12_d, 13_d, 14_d));
    EXPECT_MATRIX_EQ(5_d - m, Matrix3x3(4_d, 3_d, 2_d, 1_d, 0_d, -1_d, -2_d, -3_d, -4_d));
    EXPECT_MATRIX_EQ(2_d * m, Matrix3x3(2_d, 4_d, 6_d, 8_d, 10_d, 12_d, 14_d, 16_d, 18_d));
    EXPECT_MATRIX_EQ(2_d / m, Matrix3x3(2_d / 1_d, 1_d, 2_d / 3_d, 2_d / 4_d, 2_d / 5_d, 2_d / 6_d, 2_d / 7_d,
                                        2_d / 8_d, 2_d / 9_d));
    EXPECT_THROW(m / 0_d, std::invalid_argument);
    EXPECT_THROW(2_d / n, std::invalid_argument);

    // Matrix Vector Arithmetic Operators
    Vector3D v(1_d, 2_d, 3_d);
    EXPECT_VECTOR_EQ(m.matrixVectorProduct(v), Vector3D(14_d, 32_d, 50_d));
    EXPECT_VECTOR_EQ(vectorMatrixProduct(v, m), Vector3D(30_d, 36_d, 42_d));
}

// ——————————————————————————————————————————————————————————————————————————
//  Printing
// ——————————————————————————————————————————————————————————————————————————
TEST(Matrix3x3_Test, StreamOutput)
{
    Matrix3x3         m(1_d, 2_d, 3_d, 4_d, 5_d, 6_d, 7_d, 8_d, 9_d);
    std::stringstream ss;
    ss << m;

    std::string expected = "Matrix3x3:\n1, 2, 3\n4, 5, 6\n7, 8, 9";
    EXPECT_EQ(ss.str(), expected);
}
