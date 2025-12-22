#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/command.hpp"
#include "world/physicsWorld.hpp"

#include <deque>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

using namespace std;

// ============================================================================
// Test Fixture for Command Tests
// ============================================================================
class CommandUtilitiesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a world for testing
        world = std::make_unique<PhysicsWorld>();
    }

    void TearDown() override { world.reset(); }

    std::unique_ptr<PhysicsWorld> world;
};

// ============================================================================
// Command String Utilities Tests
// ============================================================================
TEST_F(CommandUtilitiesTest, ParseWords)
{
    // Empty string
    deque<string> result = parseWords("");
    EXPECT_TRUE(result.empty());

    // Single word
    deque<string> _result = parseWords("help");
    ASSERT_EQ(_result.size(), 1);
    EXPECT_EQ(_result[0], "help");

    // Multiple words
    deque<string> __result = parseWords("add sphere 1.0 2.0 3.0");
    ASSERT_EQ(__result.size(), 5);
    EXPECT_EQ(__result[0], "add");
    EXPECT_EQ(__result[1], "sphere");
    EXPECT_EQ(__result[2], "1.0");
    EXPECT_EQ(__result[4], "3.0");

    // Extra Spaces
    deque<string> ___result = parseWords("  set   obj   1   pos   1 2 3  ");
    ASSERT_EQ(___result.size(), 7);
    EXPECT_EQ(___result[0], "set");
    EXPECT_EQ(___result[1], "obj");
    EXPECT_EQ(___result[2], "1");
    EXPECT_EQ(___result[3], "pos");
    EXPECT_EQ(___result[5], "2");
    EXPECT_EQ(___result[6], "3");
}

TEST_F(CommandUtilitiesTest, PopNext)
{
    // Empty deque
    deque<string> words;
    string        result = popNext(words);
    EXPECT_TRUE(result.empty());
    EXPECT_TRUE(words.empty());

    // Single element
    deque<string> _words  = { "hello" };
    string        _result = popNext(_words);
    EXPECT_EQ(_result, "hello");
    EXPECT_TRUE(_words.empty());

    // Multiple elements
    deque<string> __words  = { "set", "obj", "1", "pos" };
    string        __result = popNext(__words);
    EXPECT_EQ(__result, "set");
    ASSERT_EQ(__words.size(), 3);
    EXPECT_EQ(__words[0], "obj");
    EXPECT_EQ(__words[1], "1");
    EXPECT_EQ(__words[2], "pos");
}

TEST_F(CommandUtilitiesTest, IsValidName)
{
    // Valid names
    EXPECT_TRUE(isValidName("sphere"));
    EXPECT_TRUE(isValidName("Sphere123"));
    EXPECT_TRUE(isValidName("obj1"));
    EXPECT_TRUE(isValidName("AABB"));
    EXPECT_TRUE(isValidName("test123"));

    // Invalid names
    EXPECT_FALSE(isValidName(""));
    EXPECT_FALSE(isValidName("test-name"));
    EXPECT_FALSE(isValidName("test name"));
    EXPECT_FALSE(isValidName("test@name"));
    EXPECT_FALSE(isValidName("test_name"));
}

TEST_F(CommandUtilitiesTest, ConsumeNames)
{
    // Empty deque
    deque<string>  words;
    vector<string> names = consumeNames(words);
    EXPECT_TRUE(names.empty());
    EXPECT_TRUE(words.empty());

    // Only names
    deque<string>  _words = { "obj1", "obj2", "obj3" };
    vector<string> _names = consumeNames(_words);
    ASSERT_EQ(_names.size(), 3);
    EXPECT_EQ(_names[0], "obj1");
    EXPECT_EQ(_names[1], "obj2");
    EXPECT_EQ(_names[2], "obj3");
    EXPECT_TRUE(_words.empty());

    // Mixed content
    deque<string>  __words = { "obj1", "obj2", "123", "obj3" };
    vector<string> __names = consumeNames(__words);
    ASSERT_EQ(__names.size(), 2);
    EXPECT_EQ(__names[0], "obj1");
    EXPECT_EQ(__names[1], "obj2");
    ASSERT_EQ(__words.size(), 2);
    EXPECT_EQ(__words[1], "obj3");
}

// ============================================================================
// Property Setter Tests
// ============================================================================
TEST_F(CommandUtilitiesTest, SetVector3Property_ValidArgs)
{
    // Create a test object
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();

    vector<string> args = { "1.0", "2.0", "3.0" };
    setVector3Property(obj, args, &Object::setPosition, "pos");

    // Check if position was set correctly
    Vector3D pos = obj->getPosition();
    EXPECT_DOUBLE_EQ(pos.getX(), 1.0);
    EXPECT_DOUBLE_EQ(pos.getY(), 2.0);
    EXPECT_DOUBLE_EQ(pos.getZ(), 3.0);
}

TEST_F(CommandUtilitiesTest, SetVector3Property_InsufficientArgs)
{
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();

    vector<string> args = { "1.0", "2.0" };

    // Capture output
    testing::internal::CaptureStdout();
    setVector3Property(obj, args, &Object::setPosition, "pos");
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("Usage") != string::npos);
}

TEST_F(CommandUtilitiesTest, SetScalarProperty_ValidArgs)
{
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();

    vector<string> args = { "5.0" };
    setScalarProperty(obj, args, &Object::setMass, "mass");

    // Check if mass was set correctly
    EXPECT_DOUBLE_EQ(obj->getMass(), 5.0);
}

TEST_F(CommandUtilitiesTest, SetScalarProperty_NoArgs)
{
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();

    vector<string> args;

    testing::internal::CaptureStdout();
    setScalarProperty(obj, args, &Object::setMass, "mass");
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("Usage") != string::npos);
}

TEST_F(CommandUtilitiesTest, SetFixedProperty_ValidTrueValues)
{
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();

    // Test various true values
    vector<string> trueValues = { "1", "true", "yes" };
    for (const auto& val : trueValues)
    {
        vector<string> args = { val };
        setFixedProperty(obj, args);
        EXPECT_TRUE(obj->getIsFixed());
    }
}

TEST_F(CommandUtilitiesTest, SetFixedProperty_ValidFalseValues)
{
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();
    obj->setIsFixed(true); // Start with fixed

    // Test various false values
    vector<string> falseValues = { "0", "false", "no" };
    for (const auto& val : falseValues)
    {
        vector<string> args = { val };
        setFixedProperty(obj, args);
        EXPECT_FALSE(obj->getIsFixed());
    }
}

TEST_F(CommandUtilitiesTest, SetFixedProperty_NoArgs)
{
    auto    sphere = std::make_unique<Sphere>(Vector3D(0, 0, 0), 1.0, Vector3D(0, 0, 0), 1.0);
    Object* obj    = sphere.get();

    vector<string> args;

    testing::internal::CaptureStdout();
    setFixedProperty(obj, args);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("Usage") != string::npos);
}

// ============================================================================
// Command Handler Tests
// ============================================================================
TEST_F(CommandUtilitiesTest, EmptyWords)
{
    PhysicsWorld            world;
    std::deque<std::string> words;

    EXPECT_FALSE(handleSetCommand(world, words));
}
TEST_F(CommandUtilitiesTest, HandleSetCommand_SetTimeStep)
{
    deque<string> words = { "dt", "0.016" };

    testing::internal::CaptureStdout();
    bool   result = handleSetCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
    EXPECT_TRUE(output.find("Timestep set to") != string::npos);
    EXPECT_TRUE(words.empty());
}

TEST_F(CommandUtilitiesTest, HandleSetCommand_SetGravity)
{
    deque<string> words = { "g", "9.81" };

    testing::internal::CaptureStdout();
    bool   result = handleSetCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
    EXPECT_TRUE(output.find("Gravity set to") != string::npos);
    EXPECT_TRUE(words.empty());

    // Missing value
    PhysicsWorld            _world;
    std::deque<std::string> _words = { "g" };

    EXPECT_FALSE(handleSetCommand(_world, _words));
}

TEST_F(CommandUtilitiesTest, HandleSetCommand_SetObjectProperty_InvalidObject)
{
    // Add an object first
    auto sphere = std::make_unique<Sphere>(Vector3D(0, 0, 10), 0.2, Vector3D(0, 0, 0), 1.0);
    world->addObject(sphere.release());

    deque<string> words = { "obj", "999", "pos", "1", "2", "3" }; // Invalid ID

    testing::internal::CaptureStdout();
    bool   result = handleSetCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(result);
    EXPECT_TRUE(output.find("Invalid object id or property") != string::npos);
}

TEST_F(CommandUtilitiesTest, HandleSetCommand_SetObjectProperty_InvalidProperty)
{
    // Add an object first
    auto sphere = std::make_unique<Sphere>(Vector3D(0, 0, 10), 0.2, Vector3D(0, 0, 0), 1.0);
    world->addObject(sphere.release());

    deque<string> words = { "obj", "0", "invalidprop", "1", "2", "3" };

    testing::internal::CaptureStdout();
    bool   result = handleSetCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(result);
    EXPECT_TRUE(output.find("Invalid object id or property") != string::npos);
}

TEST_F(CommandUtilitiesTest, HandleSetCommand_InsufficientArgs)
{
    deque<string> words = { "dt" }; // No value

    testing::internal::CaptureStdout();
    bool   result = handleSetCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(result);
    EXPECT_TRUE(output.empty()); // No output for insufficient args
}

TEST_F(CommandUtilitiesTest, HandleAddCommand_AddSphere)
{
    deque<string> words = { "sphere" };

    testing::internal::CaptureStdout();
    bool   result = handleAddCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
    EXPECT_TRUE(output.find("Added sphere") != string::npos);
    EXPECT_GE(world->getObject().size(), 1);

    deque<string> _words = { "obj", "0", "vel", "1", "2", "3" };

    testing::internal::CaptureStdout();
    bool   _result = handleSetCommand(*world, _words);
    string _output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
}

TEST_F(CommandUtilitiesTest, HandleAddCommand_AddPlane)
{
    deque<string> words = { "plane" };

    testing::internal::CaptureStdout();
    bool   result = handleAddCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
    EXPECT_TRUE(output.find("Added plane") != string::npos);
    EXPECT_EQ(world->getObject().size(), 1);

    deque<string> _words = { "obj", "0", "acc", "1", "2", "3" };

    testing::internal::CaptureStdout();
    bool   _result = handleSetCommand(*world, _words);
    string _output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
}

TEST_F(CommandUtilitiesTest, HandleAddCommand_AddAABB)
{
    deque<string> words = { "AABB" };

    testing::internal::CaptureStdout();
    bool   result = handleAddCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(result);
    EXPECT_TRUE(output.find("Added AABB") != string::npos);
    EXPECT_GE(world->getObject().size(), 1);
}

TEST_F(CommandUtilitiesTest, HandleAddCommand_InvalidType)
{
    deque<string> words = { "invalidtype" };

    testing::internal::CaptureStdout();
    bool   result = handleAddCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(result);
    EXPECT_TRUE(output.find("Unknown object type") != string::npos);
}

TEST_F(CommandUtilitiesTest, HandleAddCommand_NoArgs)
{
    deque<string> words;

    testing::internal::CaptureStdout();
    bool   result = handleAddCommand(*world, words);
    string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(words.empty());
    EXPECT_FALSE(result);
    EXPECT_TRUE(output.find("Usage") != string::npos);
}

// ============================================================================
// Error Handling Tests
// ============================================================================
TEST_F(CommandUtilitiesTest, ParseWords_EdgeCases)
{
    // Test with tabs
    deque<string> result1 = parseWords("add\tsphere\t1.0\t2.0\t3.0");
    ASSERT_EQ(result1.size(), 5);

    // Test with newlines (shouldn't happen but let's be safe)
    deque<string> result2 = parseWords("add\nsphere");
    ASSERT_EQ(result2.size(), 2);

    // Test very long command
    string        longCommand = "set obj 1 pos " + string(100, '1') + " 2 3";
    deque<string> result3     = parseWords(longCommand);
    EXPECT_GE(result3.size(), 6);
}

// ============================================================================
// Main function for running tests
// ============================================================================
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
