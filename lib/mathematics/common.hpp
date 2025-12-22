/**
 * @file common.hpp
 * @brief Utility functions for approximate floating-point comparison.
 *
 * Provides helpers to compare two `decimal` values (float or double, depending
 * on `precision.hpp`) with a tolerance.
 *
 */

#pragma once

#include "precision.hpp"
#include "world/config.hpp"

#include <cmath>
#include <limits>

namespace commonMaths {

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
bool approxEqual(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE);

/**
 * @brief True if lhs is (significantly) greater than rhs.
 *
 * Checks `lhs > rhs + precision`.
 */
bool approxGreaterThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE);

/**
 * @brief True if lhs is (significantly) smaller than rhs.
 *
 * Checks `lhs < rhs - precision`.
 */
bool approxSmallerThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE);

/**
 * @brief True if lhs is greater than or approximately equal to rhs.
 *
 * Checks `lhs >= rhs - precision`.
 */
bool approxGreaterOrEqualThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE);

/**
 * @brief True if lhs is smaller than or approximately equal to rhs.
 *
 * Checks `lhs <= rhs + precision`.
 */
bool approxSmallerOrEqualThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE);

} // namespace commonMaths
