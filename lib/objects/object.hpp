#pragma once
#include "mathematics/vector.hpp"
#include "precision.hpp"

struct Object
{
private:
    Vector3D position;
    Vector3D rotation;
    Vector3D scale;

public:
    Object()  = default;
    ~Object() = default;
};
