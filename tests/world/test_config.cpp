#include "test_functions.hpp"
#include "world/config.hpp"

#include <cstdio> // for std::remove
#include <fstream>
#include <gtest/gtest.h>

// Helper: create a temporary YAML file
static std::string createTempConfigFile(const std::string& content)
{
    std::string   path = "temp_config.yaml";
    std::ofstream file(path);
    file << content;
    file.close();
    return path;
}

TEST(ConfigTest, DefaultValue)
{
    Config& config = Config::get();
    EXPECT_DECIMAL_EQ(config.getGravity(), -9.81_d);
    EXPECT_DECIMAL_EQ(config.getTimeStep(), 0.01_d);
    EXPECT_EQ(config.getMaxIterations(), 10u);
}

TEST(ConfigTest, LoadFromFile)
{
    // Full
    std::string yaml1 = R"(
        gravity: 15.2
        timestep: 0.005
        solverIters: 50
    )";
    std::string path1 = createTempConfigFile(yaml1);

    Config& config1 = Config::get();
    config1.loadFromFile(path1);

    EXPECT_DECIMAL_EQ(config1.getGravity(), 15.2);
    EXPECT_DECIMAL_EQ(config1.getTimeStep(), 0.005);
    EXPECT_EQ(config1.getMaxIterations(), 50u);

    std::remove(path1.c_str());

    // Partial
    std::string yaml2 = R"(
        timestep: 0.02
    )";
    std::string path2 = createTempConfigFile(yaml2);

    Config& config2 = Config::get();
    config2.loadFromFile(path2);

    EXPECT_DECIMAL_EQ(config2.getTimeStep(), 0.02_d);
    // Other fields unchanged (should keep previous values)
    EXPECT_DECIMAL_EQ(config2.getGravity(), 15.2_d);
    EXPECT_EQ(config2.getMaxIterations(), 50u);

    std::remove(path2.c_str());

    // Invalid file
    Config& configInvalid = Config::get();
    EXPECT_THROW(configInvalid.loadFromFile("nonexistent.yaml"), std::exception);
}

TEST(ConfigTest, OverrideFromCommandLine)
{
    Config& config = Config::get();

    const char* argv[] = { "program", "--gravity", "12.5", "--timestep", "0.002", "--iters", "200" };
    int         argc   = sizeof(argv) / sizeof(argv[0]);

    config.overrideFromCommandLine(argc, const_cast<char**>(argv));

    EXPECT_DECIMAL_EQ(config.getGravity(), 12.5_d);
    EXPECT_DECIMAL_EQ(config.getTimeStep(), 0.002_d);   // unchanged
    EXPECT_DECIMAL_EQ(config.getMaxIterations(), 200u); // unchanged
}
TEST(ConfigTest, OverrideFromCommandLineInvalid)
{
    Config& config = Config::get();

    // Test invalid args
    const char* argv[] = { "program", "--unknow", "999", "--gravity" };
    int         argc   = sizeof(argv) / sizeof(argv[0]);

    config.overrideFromCommandLine(argc, const_cast<char**>(argv));

    // Values unchanged
    EXPECT_DOUBLE_EQ(config.getGravity(), -9.81_d);
    EXPECT_DOUBLE_EQ(config.getTimeStep(), 0.01_d);
    EXPECT_EQ(config.getMaxIterations(), 10);
}
