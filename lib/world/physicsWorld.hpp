#pragma once

#include "common.hpp"
#include "config.hpp"
#include "object.hpp"

#include <vector>

struct PhysicsWorld
{
private:
    Config&             config = Config::get();
    std::vector<Object> objects;
    decimal             deltaTime       = 1_d;
    decimal             gravityConstant = config.gravity;

public:
    PhysicsWorld()  = default;
    ~PhysicsWorld() = default;

    void initialize();
    void update(decimal deltaTime);
    void reset();
};
