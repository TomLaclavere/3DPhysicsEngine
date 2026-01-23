#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"

#include <cmath>
#include <gtest/gtest.h>

// ——————————————————————————————————————————————————————————————————————————
//  X vs X Collisions
// ——————————————————————————————————————————————————————————————————————————
// Sphere vs Sphere
TEST(SphereTest, NarrowSphereCollision)
{
    Sphere  sphere(Vector3D(0_d, 0_d, 0_d), 2_d);
    Sphere  sphere_inside(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere  sphere_outside(Vector3D(5_d, 5_d, 5_d), 2_d);
    Sphere  sphereTouching(Vector3D(2_d, 0_d, 0_d), 2_d);
    Contact contact;

    EXPECT_TRUE(sphere.computeCollision(sphere_inside, contact));   // Overlapping
    EXPECT_FALSE(sphere.computeCollision(sphere_outside, contact)); // Not overlapping
    EXPECT_TRUE(sphere.computeCollision(sphereTouching, contact));  // Just touching
    EXPECT_TRUE(sphere_inside.computeCollision(sphere, contact));   // Overlapping
    EXPECT_FALSE(sphere_outside.computeCollision(sphere, contact)); // Not overlapping
    EXPECT_TRUE(sphereTouching.computeCollision(sphere, contact));  // Just touching
}
// AABB vs AABB
TEST(AABBTest, NarrowAABBCollision)
{
    AABB    aabb1(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB    aabb2(Vector3D(1_d, 1_d, 1_d), Vector3D(2_d, 2_d, 2_d));
    AABB    aabb3(Vector3D(3_d, 3_d, 3_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    EXPECT_TRUE(aabb1.computeCollision(aabb2, contact));  // Overlapping
    EXPECT_FALSE(aabb1.computeCollision(aabb3, contact)); // Not overlapping
    EXPECT_TRUE(aabb2.computeCollision(aabb1, contact));  // Overlapping
    EXPECT_FALSE(aabb3.computeCollision(aabb1, contact)); // Not overlapping

    // Test over others dimensions
    AABB aabbY(Vector3D(0_d, 10_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    AABB aabbZ(Vector3D(0_d, 0_d, -10_d), Vector3D(2_d, 2_d, 2_d));
    EXPECT_FALSE(aabb1.computeCollision(aabbY, contact));
    EXPECT_FALSE(aabb1.computeCollision(aabbZ, contact));
    EXPECT_FALSE(aabbY.computeCollision(aabb1, contact));
    EXPECT_FALSE(aabbZ.computeCollision(aabb1, contact));
}
// Plane vs Plane
TEST(PlaneTest, NarrowPlaneCollision)
{
    // Very long function to test: decomposition of the different paths

    Plane   plane(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d));
    Contact contact;

    // 1) Parallel / coplanar check
    // Distinct parallel planes
    EXPECT_FALSE(plane.computeCollision(
        Plane(Vector3D(0_d, 0_d, 2_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d)), contact));

    // Coplanar planes too far
    EXPECT_FALSE(plane.computeCollision(
        Plane(Vector3D(5_d, 0_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d)), contact));

    // Coplanar planes intersecting
    EXPECT_TRUE(plane.computeCollision(
        Plane(Vector3D(0_d, -1_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d)), contact));

    // 2) Non-parallel check
    // Intersection at center
    EXPECT_TRUE(
        plane.computeCollision(Plane(Vector3D(0_d), Vector3D(2_d), Vector3D(1_d, 0_d, 0_d)), contact));

    // Not intersecting over X
    EXPECT_FALSE(plane.computeCollision(
        Plane(Vector3D(5_d, 0_d, 0_d), Vector3D(2_d), Vector3D(0_d, 1_d, 0_d)), contact));
    // Not intersecting over Y
    EXPECT_FALSE(plane.computeCollision(
        Plane(Vector3D(0_d, 4_d, 0_d), Vector3D(2_d), Vector3D(0_d, 1_d, 0_d)), contact));
    // Not intersecting over Z
    EXPECT_FALSE(plane.computeCollision(
        Plane(Vector3D(0_d, 0_d, -3_d), Vector3D(2_d), Vector3D(0_d, 1_d, 0_d)), contact));
    // Intersecting at border
    EXPECT_TRUE(plane.computeCollision(Plane(Vector3D(2_d, 0_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d)),
                                       contact));
    // Intersecting at angle
    EXPECT_TRUE(plane.computeCollision(Plane(Vector3D(2_d, 2_d, 0_d), Vector3D(2_d), Vector3D(0_d, 0_d, 1_d)),
                                       contact));
}

// ——————————————————————————————————————————————————————————————————————————
//  X vs Y Collisions
// ——————————————————————————————————————————————————————————————————————————
// Sphere vs Plane
TEST(SphereTest, NarrowSpherePlaneCollision)
{
    Plane   plane(Vector3D(0_d, 0_d, 0_d), Vector3D(4_d, 4_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    Contact contact;

    // Sphere intersecting the plane from the front
    Sphere sphereIntersecting(Vector3D(0_d, 0_d, 0.5_d), 1_d); // Center at z=0.5, radius=1
    EXPECT_TRUE(sphereIntersecting.computeCollision(plane, contact));
    EXPECT_TRUE(plane.computeCollision(sphereIntersecting, contact));

    // Sphere completely above plane (no intersection)
    Sphere sphereAbove(Vector3D(0_d, 0_d, 2.5_d), 1_d); // Center at z=2.5, radius=1 → distance > radius
    EXPECT_FALSE(sphereAbove.computeCollision(plane, contact));
    EXPECT_FALSE(plane.computeCollision(sphereAbove, contact));

    // Sphere completely behind plane (one-sided collision = no intersection)
    Sphere sphereBehind(Vector3D(0_d, 0_d, -2.5_d), 1_d);
    EXPECT_FALSE(sphereBehind.computeCollision(plane, contact));
    EXPECT_FALSE(plane.computeCollision(sphereBehind, contact));

    // Sphere touching the edge from front side
    Sphere sphereTouchingEdge(Vector3D(2_d, 0_d, 0.5_d), 1_d); // At right edge, intersecting in Z
    EXPECT_TRUE(sphereTouchingEdge.computeCollision(plane, contact));
    EXPECT_TRUE(plane.computeCollision(sphereTouchingEdge, contact));

    // Sphere touching the corner from front side
    Sphere sphereTouchingCorner(Vector3D(2_d, 2_d, 0.5_d), 1_d);
    EXPECT_TRUE(sphereTouchingCorner.computeCollision(plane, contact));
    EXPECT_TRUE(plane.computeCollision(sphereTouchingCorner, contact));

    // Sphere outside plane bounds in X direction
    Sphere sphereOutsideX(Vector3D(3_d, 0_d, 0.5_d), 1_d); // Too far right
    EXPECT_FALSE(plane.computeCollision(sphereOutsideX, contact));
    EXPECT_FALSE(sphereOutsideX.computeCollision(plane, contact));

    // Sphere outside plane bounds in Y direction
    Sphere sphereOutsideY(Vector3D(0_d, 3_d, 0.5_d), 1_d); // Too far up
    EXPECT_FALSE(sphereOutsideY.computeCollision(plane, contact));
    EXPECT_FALSE(plane.computeCollision(sphereOutsideY, contact));

    // Large sphere containing the entire plane
    Sphere sphereContainingPlane(Vector3D(0_d, 0_d, 0_d), 5_d);
    EXPECT_TRUE(sphereContainingPlane.computeCollision(plane, contact));
    EXPECT_TRUE(plane.computeCollision(sphereContainingPlane, contact));

    // Zero-radius sphere exactly on plane surface within bounds
    Sphere sphereZeroRadiusOnPlane(Vector3D(0_d, 0_d, 0_d), 0_d);
    EXPECT_TRUE(sphereZeroRadiusOnPlane.computeCollision(plane, contact));
    EXPECT_TRUE(plane.computeCollision(sphereZeroRadiusOnPlane, contact));

    // Zero-radius sphere outside plane bounds
    Sphere sphereZeroRadiusOutside(Vector3D(3_d, 0_d, 0_d), 0_d);
    EXPECT_FALSE(sphereZeroRadiusOutside.computeCollision(plane, contact));
    EXPECT_FALSE(plane.computeCollision(sphereZeroRadiusOutside, contact));
}
// Sphere vs AABB
TEST(AABBTest, NarrowSphereAABBCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Sphere sphereInside(Vector3D(1_d, 1_d, 1_d), 0.5_d);
    Sphere sphereOutside(Vector3D(4_d, 4_d, 4_d), 1_d);
    Sphere sphereTouchingFace(Vector3D(3_d, 1_d, 1_d), 4_d);

    EXPECT_TRUE(aabb.computeCollision(sphereInside, contact));
    EXPECT_FALSE(aabb.computeCollision(sphereOutside, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereTouchingFace, contact));
    EXPECT_TRUE(sphereInside.computeCollision(aabb, contact));
    EXPECT_FALSE(sphereOutside.computeCollision(aabb, contact));
    EXPECT_TRUE(sphereTouchingFace.computeCollision(aabb, contact));

    // Edge cases
    Sphere sphereTouchingEdge(Vector3D(1_d + 1_d / std::sqrt(2.0_d), 1_d + 1_d / std::sqrt(2.0_d), 1_d), 2_d);
    Sphere sphereTouchingCorner(
        Vector3D(1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d), 1_d + 1_d / std::sqrt(3.0_d)),
        2_d);

    EXPECT_TRUE(aabb.computeCollision(sphereTouchingEdge, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereTouchingCorner, contact));
    EXPECT_TRUE(sphereTouchingEdge.computeCollision(aabb, contact));
    EXPECT_TRUE(sphereTouchingCorner.computeCollision(aabb, contact));

    // Special cases
    Sphere sphereContainingAabb(Vector3D(1_d, 1_d, 1_d), 2_d);
    Sphere sphereZeroRadius(Vector3D(1_d, 1_d, 1_d), 0_d);

    decimal r      = 0.05_d;
    decimal offset = r / std::sqrt(3.0_d);
    Sphere  sphereAtCorner(Vector3D(1_d + offset, 1_d + offset, 1_d + offset), 0.1_d);

    EXPECT_TRUE(aabb.computeCollision(sphereContainingAabb, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereZeroRadius, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereAtCorner, contact));
    EXPECT_TRUE(sphereContainingAabb.computeCollision(aabb, contact));
    EXPECT_TRUE(sphereZeroRadius.computeCollision(aabb, contact));
    EXPECT_TRUE(sphereAtCorner.computeCollision(aabb, contact));

    // Negative coordinate cases
    Sphere sphereNegativeOutside(Vector3D(-2_d, -2_d, -2_d), 1_d);
    Sphere sphereNegativeTouching(Vector3D(-1_d, 1_d, 1_d), 1_d);

    EXPECT_FALSE(aabb.computeCollision(sphereNegativeOutside, contact));
    EXPECT_TRUE(aabb.computeCollision(sphereNegativeTouching, contact));
    EXPECT_FALSE(sphereNegativeOutside.computeCollision(aabb, contact));
    EXPECT_TRUE(sphereNegativeTouching.computeCollision(aabb, contact));
}
// AABB vs Plane
TEST(AABBTest, NarrowAABBPlaneCollision)
{
    AABB    aabb(Vector3D(0_d, 0_d, 0_d), Vector3D(2_d, 2_d, 2_d));
    Contact contact;

    // Basic cases
    Plane planeAbove(Vector3D(0_d, 0_d, 3_d));
    Plane planeBelow(Vector3D(0_d, 0_d, -3_d));
    Plane planeCenter(Vector3D(0_d, 0_d, 0_d));
    Plane planeLeftOut(Vector3D(-3_d, 0_d, 0_d));
    Plane planeLeftIn(Vector3D(-1.5_d, 0_d, 0_d));
    Plane planeRightOut(Vector3D(5_d, 0_d, 0_d));
    Plane planeRightIn(Vector3D(1.5_d, 0_d, 0_d));

    EXPECT_FALSE(aabb.computeCollision(planeAbove, contact));
    EXPECT_FALSE(aabb.computeCollision(planeBelow, contact));
    EXPECT_TRUE(aabb.computeCollision(planeCenter, contact));
    EXPECT_FALSE(aabb.computeCollision(planeLeftOut, contact));
    EXPECT_FALSE(aabb.computeCollision(planeRightOut, contact));
    EXPECT_TRUE(aabb.computeCollision(planeLeftIn, contact));
    EXPECT_TRUE(aabb.computeCollision(planeRightIn, contact));
    EXPECT_FALSE(planeAbove.computeCollision(aabb, contact));
    EXPECT_FALSE(planeBelow.computeCollision(aabb, contact));
    EXPECT_TRUE(planeCenter.computeCollision(aabb, contact));
    EXPECT_FALSE(planeLeftOut.computeCollision(aabb, contact));
    EXPECT_FALSE(planeRightOut.computeCollision(aabb, contact));
    EXPECT_TRUE(planeLeftIn.computeCollision(aabb, contact));
    EXPECT_TRUE(planeRightIn.computeCollision(aabb, contact));

    // Edge cases
    Plane planeUp(Vector3D(0_d, 0_d, 1_d));
    Plane planeDown(Vector3D(0_d, 0_d, -1_d));
    Plane planeLeft(Vector3D(-2_d, 0_d, 0_d));
    Plane planeRight(Vector3D(2_d, 0_d, 0_d));

    EXPECT_TRUE(aabb.computeCollision(planeUp, contact));
    EXPECT_TRUE(aabb.computeCollision(planeDown, contact));
    EXPECT_TRUE(aabb.computeCollision(planeLeft, contact));
    EXPECT_TRUE(aabb.computeCollision(planeRight, contact));
    EXPECT_TRUE(planeUp.computeCollision(aabb, contact));
    EXPECT_TRUE(planeDown.computeCollision(aabb, contact));
    EXPECT_TRUE(planeLeft.computeCollision(aabb, contact));
    EXPECT_TRUE(planeRight.computeCollision(aabb, contact));
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
TEST(SphereTest, NarrowCollisionDefaultCase)
{
    Sphere      sphere(Vector3D(0_d, 0_d, 0_d));
    DummyObject dummy;
    Contact     contact;

    EXPECT_FALSE(sphere.computeCollision(dummy, contact)); // Default case should return false
    EXPECT_FALSE(dummy.computeCollision(sphere, contact));
}
// AABB vs Unknown
TEST(AABB, computeCollisionDefaultCase)
{
    AABB        box(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 1_d));
    DummyObject dummy;
    Contact     contact;

    EXPECT_FALSE(box.computeCollision(dummy, contact));
    EXPECT_FALSE(dummy.computeCollision(box, contact)); // Default case should return false
}
// Plane vs Unknown
TEST(PlaneTest, NarrowCollisionDefaultCase)
{
    Plane       plane(Vector3D(0_d, 0_d, 0_d), Vector3D(1_d, 1_d, 1_d));
    DummyObject dummy;
    Contact     contact;

    EXPECT_FALSE(plane.computeCollision(dummy, contact)); // Default case should return false
    EXPECT_FALSE(dummy.computeCollision(plane, contact));
}
