#pragma once

#include "config.hpp"
#include "precision.hpp"

#include <cmath>

namespace commonMaths {
bool approxEqual(decimal lhs, decimal rhs, decimal precision = PRECISION_MACHINE)
{
    return (std::abs(lhs - rhs) < precision);
}
} // namespace commonMaths
