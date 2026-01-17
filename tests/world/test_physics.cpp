#include "objects/object.hpp"
#include "test_functions.hpp"
#include "world/physics.hpp"

#include <cmath>
#include <gtest/gtest.h>

// ============================================================================
// Dummy object for testing
// ============================================================================
struct DummyObject : public Object
{
    DummyObject(decimal m = 1.0_d) { setMass(m); }

    ObjectType getType() const override { return ObjectType::Generic; }
    bool       checkCollision(const Object&) override { return false; }
};

// ============================================================================
// Helpers
// ============================================================================
TEST(PhysicsTest, Helpers)
{
    EXPECT_DOUBLE_EQ(Physics::reducedMass(2.0_d, 3.0_d), 6.0_d / 5.0_d);
    EXPECT_DOUBLE_EQ(Physics::reducedMass(0_d, 3.0_d), 0_d);
    EXPECT_DOUBLE_EQ(Physics::reducedMass(2.0_d, 0_d), 0_d);
    // Test negative mass (should return 0)
    EXPECT_DOUBLE_EQ(Physics::reducedMass(-2.0_d, 3.0_d), 0_d);
    EXPECT_DOUBLE_EQ(Physics::reducedMass(2.0_d, -3.0_d), 0_d);

    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(2.0_d, 3.0_d), 6.0_d / 5.0_d);
    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(0_d, 3.0_d), 3.0_d);
    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(2.0_d, 0_d), 2.0_d);
    EXPECT_DOUBLE_EQ(Physics::effectiveStiffness(0_d, 0_d), 0_d); // Both zero

    EXPECT_DOUBLE_EQ(Physics::dampingRatioFromRestitution(0_d), 1.0_d);
    EXPECT_DOUBLE_EQ(Physics::dampingRatioFromRestitution(1.0_d), 0.0_d);
    decimal e     = 0.5_d;
    decimal ratio = Physics::dampingRatioFromRestitution(e);
    EXPECT_GT(ratio, 0.0_d);
    EXPECT_LT(ratio, 1.0_d);
    // Test negative restitution (should return 1)
    EXPECT_DOUBLE_EQ(Physics::dampingRatioFromRestitution(-0.5_d), 1.0_d);
    // Test >1 restitution (should return 0)
    EXPECT_DOUBLE_EQ(Physics::dampingRatioFromRestitution(1.5_d), 0.0_d);
}

// ============================================================================
// Gravity
// ============================================================================
TEST(PhysicsTest, GravityForceAndAcc)
{
    DummyObject obj(2.0_d);
    Vector3D    gAcc = Physics::computeGravityAcc(9.81_d);
    EXPECT_VECTOR_EQ(gAcc, Vector3D(0_d, 0_d, -9.81_d));

    Vector3D gForce = Physics::computeGravityForce(9.81_d, obj);
    EXPECT_VECTOR_EQ(gForce, Vector3D(0_d, 0_d, -19.62_d));
}

// ============================================================================
// Spring Force
// ============================================================================
TEST(PhysicsTest, SpringForce)
{
    DummyObject obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);

    Vector3D f = Physics::computeSpringForce(obj1, obj2);
    EXPECT_VECTOR_EQ(f, Vector3D(-6.0_d / 5.0_d, 0_d, 0_d)); // -(k1*k2/(k1+k2)) * r

    // Edge case - zero stiffness
    DummyObject obj3, obj4;
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(1_d, 0_d, 0_d));
    obj3.setStiffnessCst(0_d);
    obj4.setStiffnessCst(3_d);
    EXPECT_VECTOR_EQ(Physics::computeSpringForce(obj3, obj4), Vector3D(-3.0_d, 0_d, 0_d));

    // Test with explicit stiffness
    Vector3D f_explicit = Physics::computeSpringForce(obj1, obj2, 2.5_d);
    EXPECT_VECTOR_EQ(f_explicit, Vector3D(-2.5_d, 0_d, 0_d));

    // Edge cases for computeSpringForce with explicit k
    DummyObject obj5, obj6;
    obj5.setPosition(Vector3D(0_d));
    obj6.setPosition(Vector3D(0_d)); // r.isNull() true
    EXPECT_VECTOR_EQ(Physics::computeSpringForce(obj5, obj6, 2_d), Vector3D(0_d));

    // k==0 case
    EXPECT_VECTOR_EQ(Physics::computeSpringForce(obj1, obj2, 0_d), Vector3D(0_d));
}

// ============================================================================
// Damping Force
// ============================================================================
TEST(PhysicsTest, DampingForce)
{
    DummyObject obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj1.setVelocity(Vector3D(1_d, 0_d, 0_d));
    obj2.setVelocity(Vector3D(0_d));

    obj1.setMass(1_d);
    obj2.setMass(2_d);

    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);

    obj1.setRestitutionCst(0.5_d);
    obj2.setRestitutionCst(0.5_d);

    Vector3D f = Physics::computeDampingForce(obj1, obj2);
    EXPECT_NE(f.getX(), 0_d); // damping applied along x

    Vector3D f_fixed_e = Physics::computeDampingForce(obj1, obj2, 0.5_d);
    EXPECT_NE(f_fixed_e.getX(), 0_d);

    // Test with explicit restitution and stiffness
    Vector3D f_explicit = Physics::computeDampingForce(obj1, obj2, 0.5_d, 2.5_d);
    EXPECT_NE(f_explicit.getX(), 0_d);

    // Test k==0 case for computeDampingForce with explicit e and k
    DummyObject obj3, obj4;
    obj3.setPosition(Vector3D(0_d));
    obj4.setPosition(Vector3D(1_d, 0_d, 0_d));
    obj3.setMass(1_d);
    obj4.setMass(2_d);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj3, obj4, 0.5_d, 0_d), Vector3D(0_d));

    // Test mu==0 case for computeDampingForce with explicit e and k
    DummyObject obj5(0_d), obj6(2_d);
    obj5.setPosition(Vector3D(0_d));
    obj6.setPosition(Vector3D(1_d, 0_d, 0_d));
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj5, obj6, 0.5_d, 2_d), Vector3D(0_d));

    // Edge cases for computeDampingForce(obj1, obj2) - r.isNull() branch
    DummyObject obj7, obj8;
    obj7.setPosition(Vector3D(0_d));
    obj8.setPosition(Vector3D(0_d)); // r.isNull()
    obj7.setMass(1_d);
    obj8.setMass(2_d);
    obj7.setStiffnessCst(2_d);
    obj8.setStiffnessCst(3_d);
    obj7.setRestitutionCst(0.5_d);
    obj8.setRestitutionCst(0.5_d);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj7, obj8), Vector3D(0_d));

    // Edge case: zero k_rel or zero mu
    DummyObject obj9(0_d), obj10(1_d); // obj9 has mass 0, so mu=0
    obj9.setPosition(Vector3D(0_d));
    obj10.setPosition(Vector3D(1_d, 0_d, 0_d));
    obj9.setStiffnessCst(2_d);
    obj10.setStiffnessCst(3_d);
    EXPECT_VECTOR_EQ(Physics::computeDampingForce(obj9, obj10), Vector3D(0_d));
}

// ============================================================================
// Normal Force
// ============================================================================
TEST(PhysicsTest, NormalForce)
{
    DummyObject obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    Vector3D f     = Physics::computeNormalForce(obj1, obj2);
    Vector3D f_exp = Physics::computeSpringForce(obj1, obj2) + Physics::computeDampingForce(obj1, obj2);
    EXPECT_VECTOR_EQ(f, f_exp);

    // Test with explicit constants
    decimal  e          = 0.5_d;
    decimal  k          = 2.5_d;
    Vector3D f_explicit = Physics::computeNormalForce(obj1, obj2, e, k);
    Vector3D f_exp_explicit =
        Physics::computeSpringForce(obj1, obj2, k) + Physics::computeDampingForce(obj1, obj2, e, k);
    EXPECT_VECTOR_EQ(f_explicit, f_exp_explicit);
}

// ============================================================================
// Friction Force
// ============================================================================
TEST(PhysicsTest, FrictionForce)
{
    DummyObject obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    obj1.setVelocity(Vector3D(2_d, 1_d, 0_d)); // Has both normal and tangential components
    obj2.setVelocity(Vector3D(-1_d, 0_d, 0_d));

    obj1.setFrictionCst(0.5_d);
    obj2.setFrictionCst(0.5_d);

    // Set up stiffness and restitution to get non-zero normal force
    obj1.setStiffnessCst(2_d);
    obj2.setStiffnessCst(3_d);
    obj1.setRestitutionCst(0.5_d);
    obj2.setRestitutionCst(0.5_d);
    obj1.setMass(1_d);
    obj2.setMass(2_d);

    // This should now exercise the lines that compute normal force
    Vector3D f = Physics::computeFrictionForce(obj1, obj2);
    // Should have some non-zero component (likely in y-direction since tangential velocity has y component)
    EXPECT_NE(f.getNorm(), 0_d);

    Vector3D _f = Physics::computeFrictionForce(obj1, obj2, 0.5_d);
    EXPECT_NE(_f.getNorm(), 0_d);

    Vector3D __f = Physics::computeFrictionForce(obj1, obj2, 0.5_d, 0.5_d, 2_d);
    EXPECT_NE(__f.getNorm(), 0_d);

    // Test mu=0 case
    EXPECT_VECTOR_EQ(Physics::computeFrictionForce(obj1, obj2, 0_d), Vector3D(0_d));
    EXPECT_VECTOR_EQ(Physics::computeFrictionForce(obj1, obj2, 0_d, 0.5_d, 2_d), Vector3D(0_d));

    // Edge cases
    DummyObject obj3, obj4;
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

    decimal mu = 0.5_d;
    decimal e  = 0.5_d;
    decimal k  = 2_d;

    // v_tan.isNull() branch
    EXPECT_VECTOR_EQ(Physics::computeFrictionForce(obj3, obj4, mu, e, k), Vector3D(0_d));

    // r.isNull() -> returns early before computing normal force
    DummyObject obj5, obj6;
    obj5.setPosition(Vector3D(0_d));
    obj6.setPosition(Vector3D(0_d)); // r.isNull()
    EXPECT_VECTOR_EQ(Physics::computeFrictionForce(obj5, obj6, mu, e, k), Vector3D(0_d));

    // Test case where normal force magnitude is 0
    DummyObject obj7, obj8;
    obj7.setPosition(Vector3D(0_d));
    obj8.setPosition(Vector3D(1_d, 0_d, 0_d));
    // Set stiffness to 0 so normal force will be 0
    obj7.setStiffnessCst(0_d);
    obj8.setStiffnessCst(0_d);
    // But have tangential velocity
    obj7.setVelocity(Vector3D(0_d, 1_d, 0_d));
    obj8.setVelocity(Vector3D(0_d, 0_d, 0_d));

    // This should compute normal force and find it's 0
    Vector3D zero_normal_friction = Physics::computeFrictionForce(obj7, obj8, mu);
    EXPECT_VECTOR_EQ(zero_normal_friction, Vector3D(0_d));

    // Same for the 3-parameter version
    Vector3D zero_normal_friction_explicit = Physics::computeFrictionForce(obj7, obj8, mu, e, 0_d);
    EXPECT_VECTOR_EQ(zero_normal_friction_explicit, Vector3D(0_d));
}

// ============================================================================
// Contact Force
// ============================================================================
TEST(PhysicsTest, ContactForce)
{
    DummyObject obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

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

    Vector3D f     = Physics::computeContactForce(obj1, obj2);
    Vector3D f_exp = Physics::computeNormalForce(obj1, obj2) + Physics::computeFrictionForce(obj1, obj2);
    EXPECT_VECTOR_EQ(f, f_exp);
}

// ============================================================================
// Contact Force with constants
// ============================================================================
TEST(PhysicsTest, ContactForceWithConstants)
{
    DummyObject obj1, obj2;
    obj1.setPosition(Vector3D(0_d));
    obj2.setPosition(Vector3D(1_d, 0_d, 0_d));

    decimal mu = 0.5_d;
    decimal e  = 0.5_d;
    decimal k  = 2_d;

    // Set velocities to get non-zero friction
    obj1.setVelocity(Vector3D(0_d, 1_d, 0_d));
    obj2.setVelocity(Vector3D(0_d, 0_d, 0_d));

    Vector3D f = Physics::computeContactForce(obj1, obj2, mu, e, k);
    Vector3D f_exp =
        Physics::computeNormalForce(obj1, obj2, e, k) + Physics::computeFrictionForce(obj1, obj2, mu, e, k);
    EXPECT_VECTOR_EQ(f, f_exp);
}
