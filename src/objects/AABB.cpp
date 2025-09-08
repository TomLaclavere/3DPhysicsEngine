#include "objects/AABB.hpp"

#include "objects/sphere.hpp"

// ============================================================================
// ============================================================================
//  Collision
// ============================================================================
// ============================================================================
bool AABB::aabb_collision(const AABB& a, const AABB& b)
{
    // Check for overlap along each axis
    return (a.get_min()[0] <= b.get_max()[0] && a.get_max()[0] >= b.get_min()[0]) &&
           (a.get_min()[1] <= b.get_max()[1] && a.get_max()[1] >= b.get_min()[1]) &&
           (a.get_min()[2] <= b.get_max()[2] && a.get_max()[2] >= b.get_min()[2]);
}

bool AABB::AABB_sphere_collision(const AABB& aabb, const Sphere& sphere)
{
    // Use the static function from Sphere for consistency
    return Sphere::AABB_sphere_collision(sphere, aabb);
}

bool AABB::check_collision(const Object& other)
{
    switch (other.get_type())
    {
    case ObjectType::AABB:
        return aabb_collision(*this, static_cast<const AABB&>(other));
    case ObjectType::Sphere:
        return AABB_sphere_collision(*this, static_cast<const Sphere&>(other));
    default:
        return false;
    }
}
