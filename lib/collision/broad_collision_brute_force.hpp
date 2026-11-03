#pragma once

#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <cstddef>
#include <unordered_map>
#include <vector>

struct BroadCollisionBrute
{
    // Test each pairs of objects, considering them as sphere based on their largest size, to see if they can
    // overlap : O(N²).
private:
    std::vector<std::pair<size_t, size_t>> pairs;

public:
    /// Rebuild the grid from the current positions (call once per step)
    void update(const std::vector<Object*>& objects);

    /// Candidate pairs (i < j) found by the last update
    const std::vector<std::pair<size_t, size_t>>& getPairs() const { return pairs; }
};