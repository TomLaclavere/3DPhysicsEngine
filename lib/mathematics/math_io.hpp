/**
 * @file math_io.hpp
 * @brief Formatting utilities for mathematical types.
 */
#pragma once

#include "mathematics/vector.hpp"

#include <string>

/// Format a Vector3D as a fixed-width string.
std::string formatVector(const Vector3D& v);
