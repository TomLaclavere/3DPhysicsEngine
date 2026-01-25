#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <cmath>
#include <gtest/gtest.h>

// ——————————————————————————————————————————————————————————————————————————
//  X vs X Collisions
// ——————————————————————————————————————————————————————————————————————————
// Sphere vs Sphere
TEST(SphereTest, BroadSphereCollision)
{
    Sphere  sphere(Vector3D(0_d, 0_d, 0_d), 2_d);
    Sphere  sphere_inside(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere  sphere_outside(Vector3D(5_d, 5_d, 5_d), 2_d);
    Sphere  sphereTouching(Vector3D(2_d, 0_d, 0_d), 2_d);
    Contact contact;

    EXPECT_TRUE(sphere.checkCollision(sphere_inside));   // Overlapping
    EXPECT_FALSE(sphere.checkCollision(sphere_outside)); // Not overlapping
    EXPECT_TRUE(sphere.checkCollision(sphereTouching));  // Just touching
    EXPECT_TRUE(sphere_inside.checkCollision(sphere));   // Overlapping
    EXPECT_FALSE(sphere_outside.checkCollision(sphere)); // Not overlapping
    EXPECT_TRUE(sphereTouching.checkCollision(sphere));  // Just touching
}
// AABB vs AABB
TEST(AABBTest, BroadAABBCollision)
{
    AABB aabb1(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb2(Vector3D(1_d, 1_d, 1_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabb3(Vector3D(3_d, 3_d, 3_d), Vector3D(2_d, 2_d, 2_d));

    EXPECT_TRUE(aabb1.checkAABBCollision(aabb2));  // Overlapping
    EXPECT_FALSE(aabb1.checkAABBCollision(aabb3)); // Not overlapping
    EXPECT_TRUE(aabb2.checkAABBCollision(aabb1));  // Overlapping
    EXPECT_FALSE(aabb3.checkAABBCollision(aabb1)); // Not overlapping

    // Test over others dimensions
    AABB aabbY(Vector3D(0_d, 10_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabbZ(Vector3D(0_d, 0_d, -10_d), Vector3D(2_d, 2_d, 2_d));
    EXPECT_FALSE(aabb1.checkAABBCollision(aabbY));
    EXPECT_FALSE(aabb1.checkAABBCollision(aabbZ));
    EXPECT_FALSE(aabbY.checkAABBCollision(aabb1));
    EXPECT_FALSE(aabbZ.checkAABBCollision(aabb1));
}
// Plane vs Plane
TEST(PlaneTest, BroadPlaneCollision)
{
    // Very long function to test: decomposition of the different paths

    Plane   plane(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d));
    Contact contact;

    // Coplanar planes intersecting
    EXPECT_TRUE(
        plane.checkCollision(Plane(Vector3D(0_d, -1_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d))));

    // Non-parallel check
    // Intersection at center
    EXPECT_TRUE(plane.checkCollision(Plane(Vector3D(0_d), Vector3D(2_d), Vector3D(1_d, 0_d, 0_d))));
    // Intersecting at border
    EXPECT_TRUE(plane.checkCollision(Plane(Vector3D(2_d, 0_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d))));

    // Test collisions which would be false in narrow phase
    Plane planeCoplanar(Vector3D(0_d, 0_d, 0.1_d));
    EXPECT_TRUE(planeCoplanar.checkCollision(plane));
    EXPECT_FALSE(planeCoplanar.computeCollision(plane, contact));
    EXPECT_TRUE(plane.checkCollision(planeCoplanar));
    EXPECT_FALSE(plane.computeCollision(planeCoplanar, contact));
}

// ——————————————————————————————————————————————————————————————————————————
//  X vs Y Collisions
// ——————————————————————————————————————————————————————————————————————————
// Sphere vs Plane
TEST(PlaneTest, BroadSpherePlaneCollision)
{
    Plane   plane(Vector3D(0_d, 0_d, 0_d), Vector3D(4_d, 4_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    Contact contact;

    // Sphere intersecting the plane from the front
    Sphere sphereIntersecting(Vector3D(0_d, 0_d, 0.5_d), 1_d); // Center at z=0.5, radius=1
    EXPECT_TRUE(plane.checkCollision(sphereIntersecting));
    EXPECT_TRUE(sphereIntersecting.checkCollision(plane));

    // Sphere touching the edge from front side
    Sphere sphereTouchingEdge(Vector3D(2_d, 0_d, 0.5_d), 1_d); // At right edge, intersecting in Z
    EXPECT_TRUE(plane.checkCollision(sphereTouchingEdge));
    EXPECT_TRUE(sphereTouchingEdge.checkCollision(plane));

    // Sphere touching the corner from front side
    Sphere sphereTouchingCorner(Vector3D(2_d, 2_d, 0.5_d), 1_d);
    EXPECT_TRUE(plane.checkCollision(sphereTouchingCorner));
    EXPECT_TRUE(sphereTouchingCorner.checkCollision(plane));

    // Large sphere containing the entire plane
    Sphere sphereContainingPlane(Vector3D(0_d, 0_d, 0_d), 5_d);
    EXPECT_TRUE(plane.checkCollision(sphereContainingPlane));
    EXPECT_TRUE(sphereContainingPlane.checkCollision(plane));

    // Zero-radius sphere exactly on plane surface within bounds
    Sphere sphereZeroRadiusOnPlane(Vector3D(0_d, 0_d, 0_d), 0_d);
    EXPECT_TRUE(plane.checkCollision(sphereZeroRadiusOnPlane));
    EXPECT_TRUE(sphereZeroRadiusOnPlane.checkCollision(plane));

    // Test collisions which would be false in narrow phase
    Sphere sphereSamePlan(Vector3D(3_d, 0_d, 0_d));
    EXPECT_TRUE(plane.checkCollision(sphereSamePlan));
    EXPECT_FALSE(plane.computeCollision(sphereSamePlan, contact));
    EXPECT_TRUE(sphereSamePlan.checkCollision(plane));
    EXPECT_FALSE(sphereSamePlan.computeCollision(plane, contact));
}
// Sphere vs AABB
TEST(AABBTest, BroadSphereAABBCollision)
{
    AABB aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));

    // Basic cases
    Sphere  sphereInside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere  sphereOutside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere  sphereTouchingFace(Vector3D(3_d, 1_d, 1_d), 4_d);
    Contact contact;

    EXPECT_TRUE(aabb.checkCollision(sphereInside));
    EXPECT_FALSE(aabb.checkCollision(sphereOutside));
    EXPECT_TRUE(aabb.checkCollision(sphereTouchingFace));
    EXPECT_TRUE(sphereInside.checkCollision(aabb));
    EXPECT_FALSE(sphereOutside.checkCollision(aabb));
    EXPECT_TRUE(sphereTouchingFace.checkCollision(aabb));

    // Edge cases
    Sphere sphereTouchingEdge(Vector3D(1_d + 1_d / std::sqrt(2.0_d), 1_d + 1_d / std::sqrt(2.0_d), 1_d), 2_d);
    Sphere sphereTouchingCorner(
        Vector3D(1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d)),
        2_d);

    EXPECT_TRUE(aabb.checkCollision(sphereTouchingEdge));
    EXPECT_TRUE(aabb.checkCollision(sphereTouchingCorner));
    EXPECT_TRUE(sphereTouchingEdge.checkCollision(aabb));
    EXPECT_TRUE(sphereTouchingCorner.checkCollision(aabb));

    // Special cases
    Sphere sphereContainingAabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphereZeroRadius(Vector3D(1_d, 1_d, 1_d), 0_d);

    decimal r      = 0.05_d;
    decimal offset = r / std::sqrt(3.0_d);
    Sphere  sphereAtCorner(Vector3D(1_d + offset, 1_d + offset, 1_d + offset), 0.1_d);

    EXPECT_TRUE(aabb.checkCollision(sphereContainingAabb));
    EXPECT_TRUE(aabb.checkCollision(sphereZeroRadius));
    EXPECT_TRUE(aabb.checkCollision(sphereAtCorner));
    EXPECT_TRUE(sphereContainingAabb.checkCollision(aabb));
    EXPECT_TRUE(sphereZeroRadius.checkCollision(aabb));
    EXPECT_TRUE(sphereAtCorner.checkCollision(aabb));

    // Negative coordinate cases
    Sphere sphereNegativeOutside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphereNegativeTouching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.checkCollision(sphereNegativeOutside));
    EXPECT_TRUE(aabb.checkCollision(sphereNegativeTouching));
    EXPECT_FALSE(sphereNegativeOutside.checkCollision(aabb));
    EXPECT_TRUE(sphereNegativeTouching.checkCollision(aabb));

    // Test collisions which would be false in narrow phase
    Sphere sphereClose(Vector3D(0_d, 0_d, 2_d), 0.95_d);
    EXPECT_TRUE(aabb.checkCollision(sphereClose));
    EXPECT_FALSE(aabb.computeCollision(sphereClose, contact));
    EXPECT_TRUE(sphereClose.checkCollision(aabb));
    EXPECT_FALSE(sphereClose.computeCollision(aabb, contact));
}
// AABB vs Plane
TEST(AABBTest, BroadAABBPlaneCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Plane planeAbove(Vector3D(0_d, 0_d, 3_d));
    Plane planeBelow(Vector3D(0_d, 0_d, -3_d));
    Plane planeCenter(Vector3D(0_d, 0_d, 0_d));
    Plane planeLeftIn(Vector3D(-1.5_d, 0_d, 0_d));
    Plane planeRightIn(Vector3D(1.5_d, 0_d, 0_d));

    EXPECT_FALSE(aabb.checkCollision(planeAbove));
    EXPECT_FALSE(aabb.checkCollision(planeBelow));
    EXPECT_TRUE(aabb.checkCollision(planeCenter));
    EXPECT_TRUE(aabb.checkCollision(planeLeftIn));
    EXPECT_TRUE(aabb.checkCollision(planeRightIn));
    EXPECT_FALSE(planeAbove.checkCollision(aabb));
    EXPECT_FALSE(planeBelow.checkCollision(aabb));
    EXPECT_TRUE(planeCenter.checkCollision(aabb));
    EXPECT_TRUE(planeLeftIn.checkCollision(aabb));
    EXPECT_TRUE(planeRightIn.checkCollision(aabb));

    // Edge cases
    Plane planeUp(Vector3D(0_d, 0_d, 1_d));
    Plane planeDown(Vector3D(0_d, 0_d, -1_d));
    Plane planeLeft(Vector3D(-2_d, 0_d, 0_d));
    Plane planeRight(Vector3D(2_d, 0_d, 0_d));

    EXPECT_TRUE(aabb.checkCollision(planeUp));
    EXPECT_TRUE(aabb.checkCollision(planeDown));
    EXPECT_TRUE(aabb.checkCollision(planeLeft));
    EXPECT_TRUE(aabb.checkCollision(planeRight));
    EXPECT_TRUE(planeUp.checkCollision(aabb));
    EXPECT_TRUE(planeDown.checkCollision(aabb));
    EXPECT_TRUE(planeLeft.checkCollision(aabb));
    EXPECT_TRUE(planeRight.checkCollision(aabb));

    // Test collisions which would be false in narrow phase
    Plane planeClose(Vector3D(3_d, 0_d, 0_d));
    EXPECT_TRUE(aabb.checkCollision(planeClose));
    EXPECT_FALSE(aabb.computeCollision(planeClose, contact));
    EXPECT_TRUE(planeClose.checkCollision(aabb));
    EXPECT_FALSE(planeClose.computeCollision(aabb, contact));
}

// ——————————————————————————————————————————————————————————————————————————
//  X vs Unknown Collisions
// ——————————————————————————————————————————————————————————————————————————
// Dummy class to simulate an unknown object type
struct DummyObject : public Object
{
    ObjectType getType() const override { return ObjectType::Generic; }
    bool       computeCollision(const Object&, Contact& contact) override { return false; }
};

// Sphere vs Unknown
TEST(SphereTest, BroadCollisionDefaultCase)
{
    Sphere      sphere(Vector3D(0_d, 0_d, 0_d));
    DummyObject dummy;

    EXPECT_FALSE(sphere.checkCollision(dummy)); // Default case should return false
    EXPECT_FALSE(dummy.checkCollision(sphere));
}
// AABB vs Unknown
TEST(AABB, BroadCollisionDefaultCase)
{
    AABB        box(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 1_d));
    DummyObject dummy;

    EXPECT_FALSE(box.checkCollision(dummy)); // Default case should return false
    EXPECT_FALSE(dummy.checkCollision(box));
}
// Plane vs Unknown
TEST(PlaneTest, BroadCollisionDefaultCase)
{
    Plane       plane(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 1_d));
    DummyObject dummy;

    EXPECT_FALSE(plane.checkCollision(dummy)); // Default case should return false
    EXPECT_FALSE(dummy.checkCollision(plane));
}
