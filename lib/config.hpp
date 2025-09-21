/**
 * @file precision_constants.hpp
 * @brief Defines numerical precision constants for the physics/math engine.
 *
 * This file centralizes tolerance values and machine precision
 * used in floating-point comparisons and stability checks.
 */
#pragma once

#include "precision.hpp"

#include <limits>

/**
 * @brief Machine epsilon for the current @ref decimal type.
 *
 * Defined as `std::numeric_limits<decimal>::epsilon()`.
 * This represents the smallest increment such that
 * `1.0 + PRECISION_MACHINE != 1.0`.
 *
 * Useful for:
 * - Approximate comparisons of floating-point numbers.
 * - Stability thresholds in numerical algorithms.
 */
const decimal PRECISION_MACHINE = std::numeric_limits<decimal>::epsilon();
