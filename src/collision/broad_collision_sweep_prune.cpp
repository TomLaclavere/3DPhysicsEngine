// src/collision/broad_collision_sweep_prune.cpp
#include "collision/broad_collision_sweep_prune.hpp"

#include "mathematics/vector.hpp"

#include <algorithm>
#include <array>

void BroadCollisionSweepPrune::computeAxis(const std::vector<Object*>& objects)
{
    std::array<decimal, 3> sum {};
    std::array<decimal, 3> sumSquares {};
    size_t                 count = 0;

    for (const auto* obj : objects)
    {
        if (!obj)
            continue;
        ++count;
        for (size_t axis = 0; axis < 3; ++axis)
        {
            const decimal p = obj->getPosition()[axis];
            sum[axis] += p;
            sumSquares[axis] += p * p;
        }
    }

    decimal bestSpread = -1_d;
    for (size_t axis = 0; axis < 3 && count > 0; ++axis)
    {
        const decimal spread = sumSquares[axis] - sum[axis] * sum[axis] / static_cast<decimal>(count);
        if (spread > bestSpread)
        {
            bestSpread = spread;
            indexAxis  = axis;
        }
    }
}

void BroadCollisionSweepPrune::update(const std::vector<Object*>& objects)
{
    computeAxis(objects);

    intervals.clear();
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (!objects[i])
            continue;
        const decimal p      = objects[i]->getPosition()[indexAxis];
        const decimal extent = objects[i]->getSize().getNorm() * 0.5_d; // radius of the bounding sphere
        intervals.push_back({ .min = p - extent, .max = p + extent, .index = i });
    }
    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) { return a.min < b.min; });

    pairs.clear();
    for (size_t a = 0; a < intervals.size(); ++a)
    {
        for (size_t b = a + 1; b < intervals.size() && intervals[b].min <= intervals[a].max; ++b)
        {
            const size_t i = std::min(intervals[a].index, intervals[b].index);
            const size_t j = std::max(intervals[a].index, intervals[b].index);
            if (objects[i]->checkCollision(*objects[j]))
                pairs.emplace_back(i, j);
        }
    }

    std::sort(pairs.begin(), pairs.end());
}
