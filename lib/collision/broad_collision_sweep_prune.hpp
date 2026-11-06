#pragma once

#include "objects/aabb.hpp"
#include "objects/object.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>

struct BroadCollisionSweepPrune
{
    // Project each object onto an interval along a given axis, and test if intervals are overlapping.
    // Sorting is O(NlogN), but overlaps can stay quadratic when objects fill the 3D space: only one axis
    // filters the pairs.

private:
    struct Interval
    {
        decimal min, max;
        size_t  index;
    };

    std::vector<std::pair<size_t, size_t>> pairs;
    std::vector<Interval>                  intervals;
    size_t                                 indexAxis = 0; // default: x-axis

    /// Choose the axis along which object positions are the most spread out (largest variance)
    void computeAxis(const std::vector<Object*>& objects);

public:
    /// Rebuild the grid from the current positions (call once per step)
    void update(const std::vector<Object*>& objects);

    /// Candidate pairs (i < j) found by the last update
    const std::vector<std::pair<size_t, size_t>>& getPairs() const { return pairs; }
};