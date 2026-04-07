#include "mathematics/vector.hpp"
#include "objects/object.hpp"

struct MotionCSV
{
    decimal  time;
    Object*  obj;
    Vector3D pos;
    Vector3D vel;
    Vector3D acc;
};