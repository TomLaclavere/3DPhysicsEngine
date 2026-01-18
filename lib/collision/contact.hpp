#pragma once

#include "mathematics/vector.hpp"

// Forward declaration
class Object;

struct Contact
{
    Vector3D position;
    Vector3D normal;
    decimal  penetration;

    const Object* A;
    const Object* B;
};
