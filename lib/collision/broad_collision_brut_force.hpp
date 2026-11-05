#pragma once

#include "objects/object.hpp"

#include <cstddef>
#include <vector>

struct BroadCollisionBrut
{
    // Test each pairs of objects, considering them as sphere based on their largest size, to see if they can
    // overlap : O(N²).
private:
    std::vector<std::pair<size_t, size_t>> pairs;

public:
    void update(const std::vector<Object*>& objects);

    /// Candidate pairs (i < j) found by the last update
    const std::vector<std::pair<size_t, size_t>>& getPairs() const { return pairs; }
};