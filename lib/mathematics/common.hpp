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
 * @param lhs Left value.
 * @param rhs Right value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if the two values are approximately equal.
 */
inline bool approxEqual(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return (std::abs(lhs - rhs) <= precision);
}

/**
 * @brief Compare two floating-point numbers for approximate greater than.
 *
 * Uses an absolute difference check: `lhs < rhs + precision`.
 *
 * @param lhs Left value.
 * @param rhs Right value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if lhs is approximatevely greater than rhs, false otherwise.
 */
inline bool approxGreaterThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return lhs > rhs + precision;
}

/**
 * @brief Compare two floating-point numbers for approximate smaller than.
 *
 * Uses an absolute difference check: `lhs > rhs + precision`.
 *
 * @param lhs Left value.
 * @param rhs Right value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if lhs is approximatevely smaller than rhs, false otherwise.
 */
inline bool approxSmallerThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return lhs < rhs + precision;
}

/**
 * @brief Compare two floating-point numbers for approximate greater or equal than.
 *
 * Uses an absolute difference check: `lhs <= rhs + precision`.
 *
 * @param lhs Left value.
 * @param rhs Right value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if lhs is approximatevely greater or equal than rhs, false otherwise.
 */
inline bool approxGreaterOrEqualThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return lhs >= rhs + precision;
}

/**
 * @brief Compare two floating-point numbers for approximate smaller or equal than.
 *
 * Uses an absolute difference check: `lhs >= rhs + precision`.
 *
 * @param lhs Left value.
 * @param rhs Right value.
 * @param precision Tolerance (defaults to `PRECISION_MACHINE`).
 * @return true if lhs is approximatevely qmaller or equal than rhs, false otherwise.
 */
inline bool approxSmallerOrEqualThan(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return lhs <= rhs + precision;
}

} // namespace commonMaths
