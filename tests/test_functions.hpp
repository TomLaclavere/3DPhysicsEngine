#include "mathematics/common.hpp"

#include <gtest/gtest.h>

static void EXPECT_DECIMAL_EQ(decimal a, decimal b, decimal eps = PRECISION_MACHINE)
{
    EXPECT_TRUE(commonMaths::approxEqual(a, b, eps))
        << "Expected " << a << " ≈ " << b << "\nDifference: " << std::abs(a - b) << "\nPrecision: " << eps;
}
