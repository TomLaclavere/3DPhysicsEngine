#include "mathematics/math_io.hpp"

#include <iomanip>
#include <sstream>

/// @brief Format a Vector3D as a fixed-width scientific string "(x,  y,  z)" for console output.
std::string formatVector(const Vector3D& v)
{
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(3) << "(" << std::setw(10) << v[0] << ", " << std::setw(10)
        << v[1] << ", " << std::setw(10) << v[2] << ")";
    return oss.str();
}
