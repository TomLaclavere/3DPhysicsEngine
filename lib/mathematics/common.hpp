/**
 * @file common.hpp
 * @brief Utility functions for common maths and approximate floating-point comparison.
 *
 * Provides helpers to compare two `decimal` values (float or double, depending
 * on `precision.hpp`) with a tolerance. It includes constexpr function to test if numbers are finite and not
 * Nan, allowing each comparison to be constexpr.
 *
 */

#pragma once

#include "precision.hpp"

#include <limits>

namespace commonMaths {

/// @brief constexpr std::fabs function
constexpr decimal absVal(decimal d) { return d < 0 ? -d : d; }

/**
 * @brief consexpr check for isFinite
 *
 * @param d
 * @return true
 * @return false if NaN or infinite
 */
constexpr bool isFinite(decimal d) noexcept
{
    return d == d && d != std::numeric_limits<decimal>::infinity();
}

/**
 * @brief Compare two floating-point numbers for approximate equality.
 *
 * Uses an absolute difference check: `|lhs - rhs| <= precision`.
 *
 * @param lhs Left value.
 * @param rhs Right value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if the two values are approximately equal.
 */
constexpr bool approxEqual(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE) noexcept
{
    return isFinite(lhs) && isFinite(rhs) && isFinite(precision) && ((absVal(lhs - rhs) <= precision));
}
/**
 * @brief True if lhs is (significantly) greater than rhs.
 *
 * Checks `lhs > rhs + precision`.
 */
constexpr bool approxGreaterThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE) noexcept
{
    return isFinite(lhs) && isFinite(rhs) && isFinite(precision) && (lhs > rhs + precision);
}

/**
 * @brief True if lhs is (significantly) smaller than rhs.
 *
 * Checks `lhs < rhs - precision`.
 */
constexpr bool approxSmallerThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE) noexcept
{
    return isFinite(lhs) && isFinite(rhs) && isFinite(precision) && (lhs < rhs - precision);
}

/**
 * @brief True if lhs is greater than or approximately equal to rhs.
 *
 * Checks `lhs >= rhs - precision`.
 */
constexpr bool approxGreaterOrEqualThan(decimal lhs, decimal rhs,
                                        decimal precision = PRECISION_MACHINE) noexcept
{
    return isFinite(lhs) && isFinite(rhs) && isFinite(precision) && (lhs >= rhs - precision);
}

/**
 * @brief True if lhs is smaller than or approximately equal to rhs.
 *
 * Checks `lhs <= rhs + precision`.
 */
constexpr bool approxSmallerOrEqualThan(decimal lhs, decimal rhs,
                                        decimal precision = PRECISION_MACHINE) noexcept
{
    return isFinite(lhs) && isFinite(rhs) && isFinite(precision) && (lhs <= rhs + precision);
}

} // namespace commonMaths
