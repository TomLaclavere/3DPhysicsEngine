#include "collision/contact.hpp"
#include "mathematics/vector.hpp"
#include "objects/sphere.hpp"
#include "test_functions.hpp"
#include "world/physics.hpp"

#include <gtest/gtest.h>

// ============================================================================
// Helpers
// ============================================================================
TEST(PhysicsTest, Helpers)
{
    EXPECT_DOUBLE_EQ(Physics::reducedMass(2.0_d, 3.0_d), 6.0_d / 5.0_d);
    EXPECT_DOUBLE_EQ(Physics::reducedMass(0_d, 3.0_d), 3_d);
    EXPECT_DOUBLE_EQ(Physics::reducedMass(2.0_d, 0_d), 2_d);
    // Test negative mass (should return 0)
    EXPECT_DOUBLE_EQ(Physics::reducedMass(-2.0_d, 3.0_d), 3_d);
    EXPECT_DOUBLE_EQ(Physics::reducedMass(2.0_d, -3.0_d), 2_d);

    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(2.0_d, 3.0_d), 6.0_d / 5.0_d);
    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(0_d, 3.0_d), 3.0_d);
    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(2.0_d, 0_d), 2.0_d);
    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(0_d, 0_d), 0_d); // Both zero
}

// ============================================================================
// Gravity
// ============================================================================
TEST(PhysicsTest, GravityForceAndAcc)
{
    Sphere obj;
    obj.setMass(2_d);
    Vector3D gAcc = Physics::computeGravityAcc(9.81_d);
    EXPECT_VECTOR_EQ(gAcc, Vector3D(0_d, 0_d, -9.81_d));

    Vector3D gForce = Physics::computeGravityForce(9.81_d, obj);
    EXPECT_VECTOR_EQ(gForce, Vector3D(0_d, 0_d, -19.62_d));
}

// ============================================================================
// Spring Force
// ============================================================================
TEST(PhysicsTest, SpringForce)
{
    Sphere obj1, obj2;

    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj1.setSize(Vector3D(2_d));
    obj2.setSize(Vector3D(2_d));

    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);

    Contact contact12;
    obj1.computeCollision(obj2, contact12);

    Vector3D f = Physics::computeSpringForce(obj1, obj2, contact12);
    EXPECT_VECTOR_EQ(f, Vector3D(-6.0_d / 5.0_d, 0_d, 0_d)); // (k1*k2/(k1+k2)) * r * n

    // Edge case - zero Stiffness
    Sphere  obj3, obj4;
    Contact contact34;
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(1_d, 0_d, 0_d));
    obj3.setSize(Vector3D(2_d));
    obj4.setSize(Vector3D(2_d));
    obj3.computeCollision(obj4, contact34);
    obj3.setStiffnessCst(0_d);
    obj4.setStiffnessCst(3_d);
    EXPECT_VECTOR_EQ(Physics::computeSpringForce(obj3, obj4, contact34), Vector3D(-3.0_d, 0_d, 0_d));
}

// ============================================================================
// Damping Force
// ============================================================================
TEST(PhysicsTest, DampingForce)
{
    Sphere obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj1.setVelocity(Vector3D(1_d, 0_d, 0_d));
    obj2.setVelocity(Vector3D(0_d));

    obj1.setMass(1_d);
    obj2.setMass(2_d);

    obj1.setSize(Vector3D(2_d));
    obj2.setSize(Vector3D(2_d));

    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);

    obj1.setDampingCst(0.5_d);
    obj2.setDampingCst(0.5_d);

    Contact contact12;
    obj1.computeCollision(obj2, contact12);

    Vector3D f = Physics::computeDampingForce(obj1, obj2, contact12);
    EXPECT_NE(f.getX(), 0_d); // damping applied along x

    // Test k==0 case for computeDampingForce with explicit e and k
    Sphere obj3, obj4;
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(1_d, 0_d, 0_d));
    obj3.setMass(1_d);
    obj4.setMass(2_d);
    obj3.setStiffnessCst(0_d);
    obj4.setStiffnessCst(0_d);

    Contact contact34;
    obj3.computeCollision(obj4, contact34);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj3, obj4, contact34), Vector3D(0_d));

    // Test mu==0 case for computeDampingForce with explicit e and k
    Sphere obj5, obj6;
    obj5.setMass(0_d);
    obj6.setMass(2_d);
    obj5.setPosition(Vector3D(0_d));
    obj6.setPosition(Vector3D(1_d, 0_d, 0_d));

    Contact contact56;
    obj5.computeCollision(obj6, contact56);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj5, obj6, contact56), Vector3D(0_d));

    // Edge cases for computeDampingForce(obj1, obj2) - r.isNull() branch
    Sphere obj7, obj8;
    obj7.setPosition(Vector3D(0_d));
    obj8.setPosition(Vector3D(0_d)); // r.isNull()
    obj7.setMass(1_d);
    obj8.setMass(2_d);
    obj7.setStiffnessCst(2_d);
    obj8.setStiffnessCst(3_d);
    obj7.setRestitutionCst(0.5_d);
    obj8.setRestitutionCst(0.5_d);

    Contact contact78;
    obj7.computeCollision(obj8, contact78);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj7, obj8, contact78), Vector3D(0_d));

    // Edge case: zero k_rel or zero mu
    Sphere obj9, obj10; // obj9 has mass 0, so mu=0
    obj9.setMass(0_d);
    obj10.setMass(1_d);
    obj9.setPosition(Vector3D(0_d));
    obj10.setPosition(Vector3D(1_d, 0_d, 0_d));
    obj9.setStiffnessCst(2_d);
    obj10.setStiffnessCst(3_d);

    Contact contact910;
    obj9.computeCollision(obj10, contact910);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj9, obj10, contact910), Vector3D(0_d));
}

// ============================================================================
// Normal Force
// ============================================================================
TEST(PhysicsTest, NormalForce)
{
    Sphere obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    Contact contact12;
    obj1.computeCollision(obj2, contact12);

    Vector3D f     = Physics::computeNormalForces(obj1, obj2, contact12);
    Vector3D f_exp = Physics::computeSpringForce(obj1, obj2, contact12) +
                     Physics::computeDampingForce(obj1, obj2, contact12);
    EXPECT_VECTOR_EQ(f, f_exp);
}

// ============================================================================
// Friction Force
// ============================================================================
TEST(PhysicsTest, FrictionForce)
{
    Sphere obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj1.setSize(Vector3D(2_d));
    obj2.setSize(Vector3D(2_d));

    obj1.setVelocity(Vector3D(2_d, 1_d, 0_d)); // Has both normal and tangential components
    obj2.setVelocity(Vector3D(-1_d, 0_d, 0_d));

    obj1.setFrictionCst(0.5_d);
    obj2.setFrictionCst(0.5_d);

    // Set up Stiffness and restitution to get non-zero normal force
    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);
    obj1.setRestitutionCst(0.5_d);
    obj2.setRestitutionCst(0.5_d);
    obj1.setMass(1_d);
    obj2.setMass(2_d);

    Contact contact12;

    // This should now exercise the lines that compute normal force
    obj1.computeCollision(obj2, contact12);
    Vector3D f = Physics::computeFrictionForce(obj1, obj2, contact12,
                                               Physics::computeNormalForces(obj1, obj2, contact12).getNorm());
    // Should have some non-zero component (likely in y-direction since tangential velocity has y component)
    EXPECT_NE(f.getNorm(), 0_d);

    // Test mu=0 case
    obj1.setFrictionCst(0_d);
    obj2.setFrictionCst(0_d);
    EXPECT_VECTOR_EQ(
        Physics::computeFrictionForce(obj1, obj2, contact12,
                                      Physics::computeNormalForces(obj1, obj2, contact12).getNorm()),
        Vector3D(0_d));

    // Edge cases
    Sphere obj3, obj4;
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj3.setVelocity(Vector3D(1_d, 0_d, 0_d));
    obj4.setVelocity(Vector3D(1_d, 0_d, 0_d)); // same velocity -> v_tan.isNull()

    obj3.setFrictionCst(0.5_d);
    obj4.setFrictionCst(0.5_d);
    obj3.setStiffnessCst(2_d);
    obj4.setStiffnessCst(3_d);
    obj3.setRestitutionCst(0.5_d);
    obj4.setRestitutionCst(0.5_d);

    Contact contact34;
    obj3.computeCollision(obj4, contact34);

    // v_tan.isNull() branch
    EXPECT_VECTOR_EQ(
        Physics::computeFrictionForce(obj3, obj4, contact34,
                                      Physics::computeNormalForces(obj3, obj4, contact34).getNorm()),
        Vector3D(0_d));

    // r.isNull() -> returns early before computing normal force
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(0_d)); // r.isNull()
    EXPECT_VECTOR_EQ(
        Physics::computeFrictionForce(obj3, obj4, contact34,
                                      Physics::computeNormalForces(obj3, obj4, contact34).getNorm()),
        Vector3D(0_d));

    // Test case where normal force magnitude is 0
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(1_d, 0_d, 0_d));
    // Set Stiffness to 0 so normal force will be 0
    obj3.setStiffnessCst(0_d);
    obj4.setStiffnessCst(0_d);
    // But have tangential velocity
    obj3.setVelocity(Vector3D(0_d, 1_d, 0_d));
    obj4.setVelocity(Vector3D(0_d, 0_d, 0_d));

    // This should compute normal force and find it's 0
    Vector3D zero_normal_friction = Physics::computeFrictionForce(
        obj3, obj4, contact34, Physics::computeNormalForces(obj3, obj4, contact34).getNorm());
    EXPECT_VECTOR_EQ(zero_normal_friction, Vector3D(0_d));
}

// ============================================================================
// Contact Force
// ============================================================================
TEST(PhysicsTest, ContactForce)
{
    Sphere obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    Contact contact12;
    obj1.computeCollision(obj2, contact12);

    // Set properties to get non-zero forces
    obj1.setMass(1_d);
    obj2.setMass(2_d);
    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);
    obj1.setRestitutionCst(0.5_d);
    obj2.setRestitutionCst(0.5_d);
    obj1.setFrictionCst(0.5_d);
    obj2.setFrictionCst(0.5_d);
    obj1.setVelocity(Vector3D(0_d, 1_d, 0_d));
    obj2.setVelocity(Vector3D(0_d, 0_d, 0_d));

    Vector3D f      = Physics::computeContactForce(obj1, obj2, contact12);
    Vector3D normal = Physics::computeNormalForces(obj1, obj2, contact12);
    Vector3D f_exp  = normal + Physics::computeFrictionForce(obj1, obj2, contact12, normal.getNorm());
    EXPECT_VECTOR_EQ(f, f_exp);
}