#pragma once

#include "common.hpp"
#include "config.hpp"
#include "object.hpp"

#include <vector>

struct PhysicsWorld
{
private:
    std::vector<Object> objects;
    decimal             deltaTime       = 1_d;
    decimal             gravityConstant = gravityEarth;

public:
    PhysicsWorld()  = default;
    ~PhysicsWorld() = default;

    void initialize();
    void update(decimal deltaTime);
    void reset();
};
