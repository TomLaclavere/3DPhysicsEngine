// src/collision/broad_collision_grid.cpp
#include "collision/broad_collision_grid.hpp"

#include "mathematics/vector.hpp"
#include "objects/object.hpp"

#include <cmath>

decimal BroadCollisionGrid::getCellSize(const std::vector<Object*>& objects) const
{
    decimal largest = 0_d;
    for (const auto* obj : objects)
    {
        if (obj)
            largest = std::max(largest, obj->getSize().getMaxValue() * 2);
    }
    return largest > 0_d ? largest : 1_d;
}

CellKey BroadCollisionGrid::computeCell(const Vector3D& p) const
{

    return { .x = static_cast<int>(std::floor(p.getX() / cellSize)),
             .y = static_cast<int>(std::floor(p.getY() / cellSize)),
             .z = static_cast<int>(std::floor(p.getZ() / cellSize)) };
}

void BroadCollisionGrid::computeGrid(const std::vector<Object*>& objects)
{
    cells.clear();
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i])
            cells[computeCell(objects[i]->getPosition())].push_back(i);
    }
}

void BroadCollisionGrid::computePairs(const std::vector<Object*>& objects)
{
    pairs.clear();

    for (const auto& [key, indices] : cells)
    {
        for (int dx = -1; dx < 2; ++dx)
        {
            for (int dy = -1; dy < 2; ++dy)
            {
                for (int dz = -1; dz < 2; ++dz)
                {
                    const auto neighbour = cells.find({ .x = key.x + dx, .y = key.y + dy, .z = key.z + dz });
                    if (neighbour == cells.end())
                        continue;

                    for (size_t i : indices)
                    {
                        for (size_t j : neighbour->second)
                        {
                            if (i < j && objects[i]->checkCollision(*objects[j]))
                                pairs.emplace_back(i, j);
                        }
                    }
                }
            }
        }
    }

    std::sort(pairs.begin(), pairs.end());
}

void BroadCollisionGrid::update(const std::vector<Object*>& objects)
{
    cellSize = getCellSize(objects);
    computeGrid(objects);
    computePairs(objects);
}
