#include "mathematics/common.hpp"

bool commonMaths::approxEqual(decimal lhs, decimal rhs, decimal precision)
{
    // If any value is NaN, treat as not equal.
    if (!std::isfinite(lhs) || !std::isfinite(rhs) || !std::isfinite(precision))
        return false;
    return std::abs(lhs - rhs) <= precision;
}

bool commonMaths::approxGreaterThan(decimal lhs, decimal rhs, decimal precision)
{
    if (!std::isfinite(lhs) || !std::isfinite(rhs) || !std::isfinite(precision))
        return false;
    return lhs > rhs + precision;
}

bool commonMaths::approxSmallerThan(decimal lhs, decimal rhs, decimal precision)
{
    if (!std::isfinite(lhs) || !std::isfinite(rhs) || !std::isfinite(precision))
        return false;
    return lhs < rhs - precision;
}

bool commonMaths::approxGreaterOrEqualThan(decimal lhs, decimal rhs, decimal precision)
{
    if (!std::isfinite(lhs) || !std::isfinite(rhs) || !std::isfinite(precision))
        return false;
    return lhs >= rhs - precision;
}

bool commonMaths::approxSmallerOrEqualThan(decimal lhs, decimal rhs, decimal precision)
{
    if (!std::isfinite(lhs) || !std::isfinite(rhs) || !std::isfinite(precision))
        return false;
    return lhs <= rhs + precision;
}
