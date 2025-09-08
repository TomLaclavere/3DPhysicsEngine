#include "objects/sphere.hpp"

#include "objects/AABB.hpp"
#include "precision.hpp"

#include <algorithm>

bool Sphere::sphere_collision(const Sphere& a, const Sphere& b)
{
    Vector3D center_diff       = b.get_center() - a.get_center();
    decimal  squared_distance  = center_diff.getNormSquare();
    decimal  squared_sum_radii = (a.get_radius() + b.get_radius()) * (a.get_radius() + b.get_radius());
    return squared_distance <= squared_sum_radii;
}

bool Sphere::AABB_sphere_collision(const Sphere& sphere, const AABB& aabb)
{
    const Vector3D c    = sphere.get_center();
    const Vector3D amin = aabb.get_min();
    const Vector3D amax = aabb.get_max();

    Vector3D closest;
    closest[0] = std::clamp(c[0], amin[0], amax[0]);
    closest[1] = std::clamp(c[1], amin[1], amax[1]);
    closest[2] = std::clamp(c[2], amin[2], amax[2]);

    decimal dist = (closest - c).getNormSquare();
    decimal r2   = sphere.get_radius() * sphere.get_radius();
    return dist <= r2 + PRECISION_MACHINE;
}

bool Sphere::check_collision(const Object& other)
{
    switch (other.get_type())
    {
    case ObjectType::Sphere:
        return sphere_collision(*this, static_cast<const Sphere&>(other));
    case ObjectType::AABB:
        return AABB_sphere_collision(*this, static_cast<const AABB&>(other));
    default:
        return false;
    }
}
