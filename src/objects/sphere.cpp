#include "lib/objects/sphere.hpp"

// ============================================================================
// ============================================================================
//  Collision
// ============================================================================
// ============================================================================
bool Sphere::sphere_collision(const Sphere& a, const Sphere& b)
{
    // Calculate the distance between the centers of the spheres
    decimal distance_between_spheres = (b.get_center() - a.get_center()).getNorm();

    // Check if the distance is less than the sum of their radii
    return (distance_between_spheres < (a.get_radius() + b.get_radius()));
}
bool Sphere::AABB_sphere_collision(const Sphere& sphere, const AABB& aabb)
{
    // Calculate the closest point on the AABB to the sphere center
    Vector3D closest_point = sphere.get_center();
    Vector3D aabb_min      = aabb.get_min();
    Vector3D aabb_max      = aabb.get_max();

    closest_point[0] = std::max(aabb_min[0], std::min(closest_point[0], aabb_max[0]));
    closest_point[1] = std::max(aabb_min[1], std::min(closest_point[1], aabb_max[1]));
    closest_point[2] = std::max(aabb_min[2], std::min(closest_point[2], aabb_max[2]));

    // Calculate the distance between the closest point and the sphere center
    decimal distance = (closest_point - sphere.get_center()).getNorm();
    return (distance < sphere.get_radius());
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
        false;
    }
}
