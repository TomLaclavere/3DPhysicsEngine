#include "mathematics/math_io.hpp"

#include <iomanip>
#include <sstream>

std::string formatVector(const Vector3D& v)
{
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(3) << "(" << std::setw(10) << v[0] << ", " << std::setw(10)
        << v[1] << ", " << std::setw(10) << v[2] << ")";
    return oss.str();
}
