#include "mathematics/common.hpp"

#include <cmath>
#include <gtest/gtest.h>

TEST(Common_Test, Approx_Equal)
{
    decimal lhs              = 2;
    decimal rhs              = 5;
    decimal rhs_equal        = 2;
    decimal rhs_equal_approx = 2 + 1e-3;
    decimal infinity         = INFINITY;

    EXPECT_FALSE(commonMaths::approxEqual(lhs, rhs));
    EXPECT_TRUE(commonMaths::approxEqual(lhs, rhs_equal));
    EXPECT_TRUE(commonMaths::approxEqual(lhs, rhs_equal_approx, 1e-2));
    EXPECT_FALSE(commonMaths::approxEqual(lhs, infinity));
}

TEST(Common_Test, Greater_Than)
{
    decimal lhs              = 4;
    decimal rhs              = 1;
    decimal rhs_wrong        = 20;
    decimal rhs_equal        = 4;
    decimal rhs_equal_approx = 4 + 1e-3;
    decimal infinity         = INFINITY;

    EXPECT_TRUE(commonMaths::approxGreaterThan(lhs, rhs));
    EXPECT_FALSE(commonMaths::approxGreaterThan(lhs, rhs_wrong));
    EXPECT_FALSE(commonMaths::approxGreaterThan(lhs, rhs_equal));
    EXPECT_FALSE(commonMaths::approxGreaterThan(lhs, infinity));

    EXPECT_TRUE(commonMaths::approxGreaterOrEqualThan(lhs, rhs));
    EXPECT_FALSE(commonMaths::approxGreaterOrEqualThan(lhs, rhs_wrong));
    EXPECT_TRUE(commonMaths::approxGreaterOrEqualThan(lhs, rhs_equal));
    EXPECT_TRUE(commonMaths::approxGreaterOrEqualThan(lhs, rhs_equal_approx, 1e-2));
    EXPECT_FALSE(commonMaths::approxGreaterOrEqualThan(lhs, infinity));
}

TEST(Common_Test, Smaller_Than)
{
    decimal lhs              = 3;
    decimal rhs              = 7;
    decimal rhs_wrong        = 1;
    decimal rhs_equal        = 3;
    decimal rhs_equal_approx = 3 + 1e-3;
    decimal infinity         = INFINITY;

    EXPECT_TRUE(commonMaths::approxSmallerThan(lhs, rhs));
    EXPECT_FALSE(commonMaths::approxSmallerThan(lhs, rhs_wrong));
    EXPECT_FALSE(commonMaths::approxSmallerThan(lhs, rhs_equal));
    EXPECT_FALSE(commonMaths::approxSmallerThan(lhs, infinity));

    EXPECT_TRUE(commonMaths::approxSmallerOrEqualThan(lhs, rhs));
    EXPECT_FALSE(commonMaths::approxSmallerOrEqualThan(lhs, rhs_wrong));
    EXPECT_TRUE(commonMaths::approxSmallerOrEqualThan(lhs, rhs_equal));
    EXPECT_TRUE(commonMaths::approxSmallerOrEqualThan(lhs, rhs_equal_approx, 1e-2));
    EXPECT_FALSE(commonMaths::approxSmallerOrEqualThan(lhs, infinity));
}
