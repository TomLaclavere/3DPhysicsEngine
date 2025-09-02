#pragma once

#ifdef IS_USE_DOUBLE_PRECISION
using decimal = double;
#else
using decimal = float;
#endif

// define literal for decimal type
constexpr decimal operator""_d(long double val) { return static_cast<decimal>(val); }
constexpr decimal operator""_d(unsigned long long val) { return static_cast<decimal>(val); }
