// src/collision/broad_collision_grid.cpp
#include "collision/broad_collision_grid.hpp"

#include <stdexcept>

void BroadCollisionGrid::update([[maybe_unused]] const std::vector<Object*>& objects)
{
    throw std::logic_error("BroadCollisionGrid::update is not implemented yet");
}
