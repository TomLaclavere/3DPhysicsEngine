#include "objects/sphere.hpp"

#include "objects/AABB.hpp"
#include "precision.hpp"

bool Sphere::sphere_collision(const Sphere& a, const Sphere& b)
{
    // Calculate squared distance between centers to avoid sqrt
    Vector3D center_diff      = b.get_center() - a.get_center();
    decimal  squared_distance = center_diff.getNorm();

    // Calculate sum of radii
    decimal sum_radii         = a.get_radius() + b.get_radius();
    decimal squared_sum_radii = sum_radii * sum_radii;

    // Check collision with tolerance
    return (squared_distance <= squared_sum_radii + PRECISION_MACHINE);
}

bool Sphere::AABB_sphere_collision(const Sphere& sphere, const AABB& aabb)
{
    Vector3D closest_point = sphere.get_center();
    Vector3D aabb_min      = aabb.get_min();
    Vector3D aabb_max      = aabb.get_max();

    // Find the closest point on the AABB to the sphere center
    closest_point[0] = std::max(aabb_min[0], std::min(closest_point[0], aabb_max[0]));
    closest_point[1] = std::max(aabb_min[1], std::min(closest_point[1], aabb_max[1]));
    closest_point[2] = std::max(aabb_min[2], std::min(closest_point[2], aabb_max[2]));

    // Calculate squared distance to avoid sqrt
    Vector3D diff             = closest_point - sphere.get_center();
    decimal  squared_distance = diff.getNorm();
    decimal  radius           = sphere.get_radius();
    decimal  squared_radius   = radius * radius;

    // Check collision with tolerance
    return (squared_distance <= squared_radius + PRECISION_MACHINE);
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
