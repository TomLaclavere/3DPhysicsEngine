#pragma once
#include "mathematics/vector.hpp"
#include "precision.hpp"

struct Objet
{
private:
    Vector3D position;
    Vector3D rotation;
    Vector3D scale;

public:
    Objet()  = default;
    ~Objet() = default;
};
