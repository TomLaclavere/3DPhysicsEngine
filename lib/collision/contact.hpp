#pragma once

#include "object.hpp"

struct Contact
{
    Vector3D position;
    Vector3D normal;
    decimal  penetration;

    const Object* A;
    const Object* B;
};
