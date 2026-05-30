/**
 * @file solver.hpp
 * @brief Enumeration of numerical integration solvers available in the physics world.
 */
#pragma once

#include <cstdint>
#include <ostream>

/// Numerical integrator used by @ref PhysicsWorld to advance the simulation.
enum class Solver : std::uint8_t
{
    Euler,  /// Semi-implicit Euler (first-order).
    Verlet, /// Velocity Verlet (second-order, symplectic).
    RK4,    /// Runge-Kutta 4 (fourth-order).
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
