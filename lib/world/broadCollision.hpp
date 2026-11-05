/**
 * @file broadCollision.hpp
 * @brief Enumeration of broad collision phase available in the physics world.
 */
#pragma once

#include <cstdint>
#include <ostream>

/// Broad collision used by @ref PhysicsWorld.
enum class BroadCollision : std::uint8_t
{
    Brut,       /// Brut Force pairs testing (O(N²)).
    Grid,       /// Grid (O(N)).
    SweepPrune, /// Sweep & Prune (O(NlogN)).
    Unknown
};

inline std::ostream& operator<<(std::ostream& os, BroadCollision s) noexcept
{
    switch (s)
    {
    case BroadCollision::Brut:
        return os << "Brut";
    case BroadCollision::Grid:
        return os << "Grid";
    case BroadCollision::SweepPrune:
        return os << "SweepPrune";
    case BroadCollision::Unknown:
        return os << "Unknown";
    }
    // Defensive fallback (shouldn't normally be reached)
    return os << "BroadCollision(<invalid>)";
}
