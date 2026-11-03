#pragma once

#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>

struct BroadCollisionSweepPrune
{
    // Project each object onto an interval along a given axis, and test if intervals are overlapping.
    // O(NlogN), but issue if objects are mainly align with the chosen axis (need to find a quick way to
    // estimate the best one).

private:
    std::vector<std::pair<size_t, size_t>> pairs;
    // need variable: intervals

public:
    /// Rebuild the grid from the current positions (call once per step)
    void update(const std::vector<Object*>& objects);

    /// Candidate pairs (i < j) found by the last update
    const std::vector<std::pair<size_t, size_t>>& getPairs() const { return pairs; }
};