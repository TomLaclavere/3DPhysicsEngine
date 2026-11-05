#include "collision/broad_collision_brut_force.hpp"

/// @brief Test every pair (i < j) and keep those whose bounding spheres overlap: O(N²).
void BroadCollisionBrut::update(const std::vector<Object*>& objects)
{
    pairs.clear();

    const size_t n = objects.size();
    for (size_t i = 0; i < n; ++i)
    {
        Object* obj1 = objects[i];
        if (!obj1)
            continue;

        for (size_t j = i + 1; j < n; ++j)
        {
            Object* obj2 = objects[j];
            if (obj2 && obj1->checkCollision(*obj2))
                pairs.emplace_back(i, j);
        }
    }
}
