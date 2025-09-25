/**
 * @file common.hpp
 * @brief Utility function for approximate floating-point comparison.
 *
 * Provides a helper to compare two `decimal` values (float or double, depending
 * on `precision.hpp`) with a tolerance.
 *
 * Part of the math foundation utilities of the physics engine.
 *
 */

#pragma once

#include "config.hpp"
#include "precision.hpp"

#include <cmath>

namespace commonMaths {

/**
 * @brief Compare two floating-point numbers for approximate equality.
 *
 * Uses an absolute difference check: `|lhs - rhs| <= precision`.
 *
 * @param lhs First value.
 * @param rhs Second value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if the two values are approximately equal.
 *
 * @note Designed for cases where floating-point rounding errors must be ignored.
 */
inline bool approxEqual(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return (std::abs(lhs - rhs) <= precision);
}

} // namespace commonMaths
