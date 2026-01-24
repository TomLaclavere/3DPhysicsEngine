#pragma once

#include <cstdint>
#include <ostream>

enum class Solver : std::uint8_t
{
    Euler,
    Verlet,
    RK4,
    Unknown
};

inline std::ostream& operator<<(std::ostream& os, Solver s) noexcept
{
    switch (s)
    {
    case Solver::Euler:
        return os << "Euler";
    case Solver::Verlet:
        return os << "Verlet";
    case Solver::RK4:
        return os << "RK4";
    case Solver::Unknown:
        return os << "Unknown";
    }
    // Defensive fallback (shouldn't normally be reached)
    return os << "Solver(<invalid>)";
}
