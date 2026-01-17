#include "narrow_collision.hpp"

#include "mathematics/common.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

// ============================================================================
//  Sphere vs Sphere
// ============================================================================
/**
 * @brief Checks collision between two spheres.
 *
 * Collision occurs if the distance between centers is less than or equal
 * to the sum of their radii.
 *
 * @param s1 Sphere instance.
 * @param s2 Sphere instance.
 * @return true if the spheres intersect, false otherwise.
 */
bool NarrowCollision::computeContact(const Sphere& s1, const Sphere& s2, Contact& contact)
{
    const Vector3D diff  = s2.getCenter() - s1.getCenter();
    const decimal  dist2 = diff.getNormSquare();
    const decimal  rSum  = s1.getRadius() + s2.getRadius();

    if (commonMaths::approxGreaterOrEqualThan(dist2, rSum * rSum))
    {
        return false;
    }

    const decimal dist = std::sqrt(dist2);

    // Compute normal and penetration
    Vector3D normal     = (commonMaths::approxGreaterThan(dist, 0_d)) ? diff / dist : Vector3D(1, 0, 0);
    contact.normal      = normal;
    contact.penetration = rSum - dist;

    // Compute contact point
    contact.position = s1.getCenter() + normal * s1.getRadius();

    contact.A = &s1;
    contact.B = &s2;

    return true;
}

// ============================================================================
//  Sphere vs AABB
// ============================================================================
/**
 * @brief Checks collision between a sphere and an axis-aligned bounding box (AABB).
 *
 * Collision occurs if the closest point on the AABB to the sphere center is smalled than the sphere radius.
 *
 * @param sphere Sphere instance.
 * @param aabb AABB instance.
 * @return true if the sphere and AABB intersect, false otherwise.
 */
bool NarrowCollision::computeContact(const Sphere& sphere, const AABB& aabb, Contact& contact)
{
    const Vector3D center = sphere.getCenter();
    const Vector3D min    = aabb.getMin();
    const Vector3D max    = aabb.getMax();
    const decimal  radius = sphere.getRadius();

    // Find the closest point on AABB to sphere center
    Vector3D closestPoint;
    closestPoint[0] = std::max(min[0], std::min(center[0], max[0]));
    closestPoint[1] = std::max(min[1], std::min(center[1], max[1]));
    closestPoint[2] = std::max(min[2], std::min(center[2], max[2]));

    Vector3D delta = center - closestPoint;
    decimal  dist2 = delta.getNormSquare();

    // Standard case : sphere inside AABB
    if (dist2 > 0_d)
    {
        if (commonMaths::approxGreaterThan(dist2, radius * radius))
            return false;

        const decimal dist = std::sqrt(dist2);

        contact.normal      = delta / dist;
        contact.penetration = radius - dist;
        contact.position    = closestPoint;
    }
    // Special case : sphere's center inside AABB
    else
    {
        // Find closest face
        const decimal dxMin = center[0] - min[0];
        const decimal dxMax = max[0] - center[0];
        const decimal dyMin = center[1] - min[1];
        const decimal dyMax = max[1] - center[1];
        const decimal dzMin = center[2] - min[2];
        const decimal dzMax = max[2] - center[2];

        decimal  minDist = dxMin;
        Vector3D normal(-1, 0, 0);

        if (dxMax < minDist)
        {
            minDist = dxMax;
            normal  = Vector3D(1, 0, 0);
        }
        if (dyMin < minDist)
        {
            minDist = dyMin;
            normal  = Vector3D(0, -1, 0);
        }
        if (dyMax < minDist)
        {
            minDist = dyMax;
            normal  = Vector3D(0, 1, 0);
        }
        if (dzMin < minDist)
        {
            minDist = dzMin;
            normal  = Vector3D(0, 0, -1);
        }
        if (dzMax < minDist)
        {
            minDist = dzMax;
            normal  = Vector3D(0, 0, 1);
        }

        contact.normal      = normal;
        contact.penetration = radius + minDist;
        contact.position    = center - normal * minDist;
    }

    contact.A = &sphere;
    contact.B = &aabb;

    return true;
}

// ============================================================================
//  Sphere vs Plane
// ============================================================================
/**
 * @brief Checks collision between a sphere and a finite plane.
 *
 * Collision occurs if the sphere's center-to-plane distance is smaller than the sphere radius, and if the
 * projection of the sphere center onto the plane lies within its rectangle bounds.
 *
 * @param sphere Sphere instance.
 * @param plane Plane instance.
 * @return true if the sphere and plane intersect, false otherwise.
 */
bool NarrowCollision::computeContact(const Sphere& sphere, const Plane& plane, Contact& contact)
{
    Vector3D        n            = plane.getNormal();
    const Vector3D& sphereCenter = sphere.getCenter();
    const decimal   sphereRadius = sphere.getRadius();

    // 1) Check distance using plane equation
    const Vector3D planeToSphere = sphereCenter - plane.getPosition();
    const decimal  signedDist    = planeToSphere.dotProduct(n);

    // Early exit: sphere completely behind or too far in front
    if (signedDist < -sphereRadius || signedDist > sphereRadius)
    {
        return false;
    }

    // 2) Project onto plane and check bounds with radius padding
    const Vector3D proj  = sphereCenter - signedDist * plane.getNormal();
    const Vector3D local = proj - plane.getPosition();

    const decimal s = local.dotProduct(plane.getU());
    const decimal t = local.dotProduct(plane.getV());

    // Check bounds with radius consideration
    const decimal effectiveHalfWidth  = plane.getHalfWidth() + sphereRadius;
    const decimal effectiveHalfHeight = plane.getHalfHeight() + sphereRadius;

    if (commonMaths::absVal(s) > effectiveHalfWidth || commonMaths::absVal(t) > effectiveHalfHeight)
    {
        return false;
    }

    // 3) Exact distance to clamped point
    const decimal clampedS = std::clamp(s, -plane.getHalfWidth(), plane.getHalfWidth());
    const decimal clampedT = std::clamp(t, -plane.getHalfHeight(), plane.getHalfHeight());

    const Vector3D closestPoint = plane.getPosition() + clampedS * plane.getU() + clampedT * plane.getV();
    const Vector3D delta        = closestPoint - sphereCenter;
    const decimal  dist2        = delta.dotProduct(delta);

    // Collision if within radius
    if (commonMaths::approxGreaterOrEqualThan(dist2, sphere.getRadius() * sphere.getRadius()))
        return false;

    decimal dist        = delta.getNorm();
    contact.normal      = (commonMaths::approxGreaterThan(dist, 0_d)) ? delta / dist : -n;
    contact.penetration = sphere.getRadius() - dist;
    contact.position    = closestPoint;
    contact.A           = &sphere;
    contact.B           = &plane;
    return true;
}

// ============================================================================
//  AABB vs AABB
// ============================================================================
/**
 * @brief Checks collision between two aabb.
 *
 * Collision occurs if they overlap on each axis simultaneously.
 *
 * @param a1 AABB instance.
 * @param a2 AABB instance.
 * @return true if the AABBs intersect, false otherwise.
 */
bool NarrowCollision::computeContact(const AABB& a1, const AABB& a2, Contact& contact)
{
    const Vector3D a1Min = a1.getMin();
    const Vector3D a1Max = a1.getMax();
    const Vector3D a2Min = a2.getMin();
    const Vector3D a2Max = a2.getMax();

    // Check overlap on each axis
    const decimal overlapX = std::min(a1Max[0], a2Max[0]) - std::max(a1Min[0], a2Min[0]);
    if (commonMaths::approxSmallerThan(overlapX, 0_d))
        ;
    return false;

    const decimal overlapY = std::min(a1Max[1], a2Max[1]) - std::max(a1Min[1], a2Min[1]);
    if (commonMaths::approxSmallerThan(overlapY, 0_d))
        ;
    return false;

    const decimal overlapZ = std::min(a1Max[2], a2Max[2]) - std::max(a1Min[2], a2Min[2]);
    if (commonMaths::approxSmallerThan(overlapZ, 0_d))
        ;
    return false;

    // Penetration value and axis
    decimal  penetration = overlapX;
    Vector3D normal(1, 0, 0);

    const Vector3D centerDelta = a2.getPosition() - a1.getPosition();

    if (overlapY < penetration)
    {
        penetration = overlapY;
        normal      = Vector3D(0, 1, 0);
    }
    if (overlapZ < penetration)
    {
        penetration = overlapZ;
        normal      = Vector3D(0, 0, 1);
    }

    // Normal orientation
    if (commonMaths::approxSmallerThan(normal.dotProduct(centerDelta), 0_d))
        normal = -normal;

    // Contact point
    const Vector3D contactMin(std::max(a1Min[0], a2Min[0]), std::max(a1Min[1], a2Min[1]),
                              std::max(a1Min[2], a2Min[2]));

    const Vector3D contactMax(std::min(a1Max[0], a2Max[0]), std::min(a1Max[1], a2Max[1]),
                              std::min(a1Max[2], a2Max[2]));

    contact.position    = (contactMin + contactMax) * 0.5_d;
    contact.normal      = normal;
    contact.penetration = penetration;

    contact.A = &a1;
    contact.B = &a2;

    return true;
}

// ============================================================================
//  AABB vs Plane
// ============================================================================
/**
 * @brief Checks collision between an aabb and a finite plane.
 *
 * Collision occurs if the signed distance from the AABB to the plane is less than or equal to the projection
 * radius of the AABB onto the plane normal.
 *
 * @param aabb AABB instance.
 * @param plane Plane instance.
 * @return true if the AABB and the Plane intersect, false otherwise.
 */
bool NarrowCollision::computeContact(const AABB& aabb, const Plane& plane, Contact& contact)
{
    const Vector3D center      = aabb.getPosition();
    const Vector3D halfExtents = aabb.getHalfExtents();
    const Vector3D n           = plane.getNormal();

    // Projection
    const decimal r = halfExtents[0] * commonMaths::absVal(n[0]) +
                      halfExtents[1] * commonMaths::absVal(n[1]) + halfExtents[2] * commonMaths::absVal(n[2]);

    // Distance
    const decimal distance = (center - plane.getPosition()).dotProduct(n);

    if (commonMaths::absVal(distance) > r)
        return false;

    // AABB closest pont to plane
    Vector3D support = center;
    support[0] -= halfExtents[0] * commonMaths::absVal(n[0]);
    support[1] -= halfExtents[1] * commonMaths::absVal(n[1]);
    support[2] -= halfExtents[2] * commonMaths::absVal(n[2]);

    // Projection on Plane
    const decimal supportDist = (support - plane.getPosition()).dotProduct(n);

    const Vector3D projected = support - supportDist * n;

    // Test collision
    const Vector3D local = projected - plane.getPosition();
    const decimal  s     = local.dotProduct(plane.getU());
    const decimal  t     = local.dotProduct(plane.getV());

    if (commonMaths::approxGreaterThan(commonMaths::absVal(s), plane.getHalfWidth()) ||
        commonMaths::approxGreaterThan(commonMaths::absVal(t), plane.getHalfHeight()))
        return false;

    // Contact
    contact.normal      = commonMaths::approxSmallerThan(distance, 0_d) ? -n : n;
    contact.penetration = r - commonMaths::absVal(distance);
    contact.position    = projected;
    contact.A           = &aabb;
    contact.B           = &plane;

    return true;
}

// ============================================================================
//  Plane vs Plane
// ============================================================================
/**
 * @brief Checks collision between two finite planes.
 *
 * Collision occurs if the associated infinite planes intersects (they are not parallel), and if the
 * intersection line passes through both rectangles' bounds.
 *
 * @param p1 Plane instance.
 * @param p2 Plane instance.
 * @return true if the planes intersect, false otherwise.
 */
bool NarrowCollision::computeContact(const Plane& p1, const Plane& p2, Contact& contact)
{
    const Vector3D n1 = p1.getNormal().getNormalised();
    const Vector3D n2 = p2.getNormal().getNormalised();

    // Check parallel planes
    const Vector3D cross       = n1.crossProduct(n2);
    const decimal  crossNormSq = cross.getNormSquare();

    if (commonMaths::approxEqual(crossNormSq, 0_d))
    {
        // Parallel planes
        const decimal d1   = n1.dotProduct(p1.getPosition());
        const decimal dist = commonMaths::absVal(n1.dotProduct(p2.getPosition()) - d1);

        if (!commonMaths::approxEqual(dist, 0_d))
            return false;

        // Coplanar: approximate contact at center of overlap
        Vector3D center1    = p1.getPosition();
        Vector3D center2    = p2.getPosition();
        contact.position    = (center1 + center2) * 0.5_d;
        contact.normal      = n2;
        contact.penetration = 0_d;
        contact.A           = &p1;
        contact.B           = &p2;
        return true;
    }

    // Non-parallel planes: compute intersection line
    const Vector3D dir = cross;
    const decimal  d1  = n1.dotProduct(p1.getPosition());
    const decimal  d2  = n2.dotProduct(p2.getPosition());

    const Vector3D P0 = ((n2 * d1 - n1 * d2).crossProduct(cross)) / crossNormSq;

    // Compute intervals for both rectangles along the line
    auto interval_for_rect = [&](const Plane& P) -> std::optional<std::pair<decimal, decimal>>
    {
        const Vector3D C     = P.getPosition();
        const Vector3D u     = P.getU().getNormalised();
        const Vector3D v     = P.getV().getNormalised();
        const decimal  halfU = P.getHalfWidth();
        const decimal  halfV = P.getHalfHeight();

        decimal s0 = u.dotProduct(P0 - C);
        decimal su = u.dotProduct(dir);
        decimal t0 = v.dotProduct(P0 - C);
        decimal sv = v.dotProduct(dir);

        auto axis_interval = [&](decimal s0_, decimal su_,
                                 decimal half) -> std::optional<std::pair<decimal, decimal>>
        {
            if (commonMaths::approxEqual(su_, 0_d))
            {
                if (commonMaths::approxSmallerOrEqualThan(commonMaths::absVal(s0_), half))
                    return std::make_pair(-INFINITY, INFINITY);
                else
                    return std::nullopt;
            }
            decimal t1 = (-half - s0_) / su_;
            decimal t2 = (half - s0_) / su_;
            if (t1 > t2)
                std::swap(t1, t2);
            return std::make_pair(t1, t2);
        };

        auto Iu = axis_interval(s0, su, halfU);
        if (!Iu)
            return std::nullopt;
        auto Iv = axis_interval(t0, sv, halfV);
        if (!Iv)
            return std::nullopt;

        decimal tmin = std::max(Iu->first, Iv->first);
        decimal tmax = std::min(Iu->second, Iv->second);
        if (commonMaths::approxGreaterThan(tmin, tmax))
            return std::nullopt;
        return std::make_pair(tmin, tmax);
    };

    auto I1 = interval_for_rect(p1);
    auto I2 = interval_for_rect(p2);
    if (!I1 || !I2)
        return false;

    decimal tmin = std::max(I1->first, I2->first);
    decimal tmax = std::min(I1->second, I2->second);
    if (commonMaths::approxGreaterThan(tmin, tmax))
        return false;

    // Contact: midpoint of intersection segment
    contact.position    = P0 + dir * ((tmin + tmax) * 0.5_d);
    contact.normal      = n2;
    contact.penetration = 0_d;
    contact.A           = &p1;
    contact.B           = &p2;

    return true;
}
