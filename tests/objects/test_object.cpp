#include "objects/object.hpp"
#include "test_functions.hpp"

#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

// ——————————————————————————————————————————————————————————————————————————
//  Getters
// ——————————————————————————————————————————————————————————————————————————

TEST(ObjectTest, Getters)
{
    Object obj;
    EXPECT_DECIMAL_EQ(obj.getPosition(), Vector3D());
}
