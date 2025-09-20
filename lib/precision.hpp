#pragma once

/**
 * @file precision.hpp
 * @brief Definition of the decimal type and user-defined literals for floating-point precision.
 *
 * Provides a precision-agnostic type `decimal` used throughout the project for mathematical computations.
 * The actual type can be `float` or `double` depending on the build configuration.
 *
 * Example usage:
 * @code
 * decimal a = 1.5_d;   // a is decimal, automatically float or double
 * decimal b = 2.0_d;
 * decimal c = a + b;
 * @endcode
 */

#ifdef IS_USE_DOUBLE_PRECISION
/// @brief Underlying floating-point type used when double precision is enabled.
using decimal = double;
#else
/// @brief Underlying floating-point type used when single precision is enabled.
using decimal = float;
#endif

/**
 * @brief User-defined literal for decimal type from floating-point literal.
 * @param val The floating-point literal value.
 * @return Converted value as `decimal`.
 *
 * Example:
 * @code
 * decimal x = 1.23_d;
 * @endcode
 */
constexpr decimal operator""_d(long double val) { return static_cast<decimal>(val); }

/**
 * @brief User-defined literal for decimal type from integer literal.
 * @param val The integer literal value.
 * @return Converted value as `decimal`.
 *
 * Example:
 * @code
 * decimal y = 42_d;
 * @endcode
 */
constexpr decimal operator""_d(unsigned long long val) { return static_cast<decimal>(val); }
