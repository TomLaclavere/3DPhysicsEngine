#pragma once
#include "mathematics/common.hpp"
#include "mathematics/matrix.hpp"
#include "mathematics/quaternion.hpp"
#include "mathematics/vector.hpp"

#include <gtest/gtest.h>

// Compare two decimals
#define EXPECT_DECIMAL_EQ(a, b)                                                              \
    do                                                                                       \
    {                                                                                        \
        decimal _a = a;                                                                      \
        decimal _b = b;                                                                      \
        if (!commonMaths::approxEqual(_a, _b, PRECISION_MACHINE))                            \
        {                                                                                    \
            ADD_FAILURE_AT(__FILE__, __LINE__)                                               \
                << "Expected " << _a << " ≈ " << _b << "\nDifference: " << std::abs(_a - _b) \
                << "\nPrecision: " << PRECISION_MACHINE;                                     \
        }                                                                                    \
    }                                                                                        \
    while (0)

// Compare two Vector3D
#define EXPECT_VECTOR_EQ(a, b)             \
    do                                     \
    {                                      \
        const Vector3D& _va = (a);         \
        const Vector3D& _vb = (b);         \
        EXPECT_DECIMAL_EQ(_va[0], _vb[0]); \
        EXPECT_DECIMAL_EQ(_va[1], _vb[1]); \
        EXPECT_DECIMAL_EQ(_va[2], _vb[2]); \
    }                                      \
    while (0)

// Compare two Matrix3x3
#define EXPECT_MATRIX_EQ(a, b)                               \
    do                                                       \
    {                                                        \
        const Matrix3x3& _ma = (a);                          \
        const Matrix3x3& _mb = (b);                          \
        for (int _i = 0; _i < 3; ++_i)                       \
            for (int _j = 0; _j < 3; ++_j)                   \
                EXPECT_DECIMAL_EQ(_ma(_i, _j), _mb(_i, _j)); \
    }                                                        \
    while (0)

// Compare two Quaternion3D
#define EXPECT_QUATERNION_EQ(a, b)                                        \
    do                                                                    \
    {                                                                     \
        const Quaternion3D& _qa = (a);                                    \
        const Quaternion3D& _qb = (b);                                    \
        EXPECT_VECTOR_EQ(_qa.getImaginaryPart(), _qb.getImaginaryPart()); \
        EXPECT_DECIMAL_EQ(_qa.getRealPart(), _qb.getRealPart());          \
    }                                                                     \
    while (0)
