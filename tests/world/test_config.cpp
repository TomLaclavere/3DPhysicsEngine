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
    EXPECT_DECIMAL_EQ(config.getGravity(), 9.81_d);
    EXPECT_DECIMAL_EQ(config.getTimeStep(), 0.01_d);
    EXPECT_DECIMAL_EQ(config.getSimulationDuration(), 10_d);
    EXPECT_EQ(config.getMaxIterations(), 1000);
    EXPECT_TRUE(config.getVerbose());
}

TEST(ConfigTest, LoadFromFile)
{
    // Full
    std::string yaml1 = R"(
        gravity: 15.2
        timestep: 0.005
        duration: 50
        solver: Euler
        verbose: true
    )";
    std::string path1 = createTempConfigFile(yaml1);

    Config& config1 = Config::get();
    config1.loadFromFile(path1);

    EXPECT_DECIMAL_EQ(config1.getGravity(), 15.2);
    EXPECT_DECIMAL_EQ(config1.getTimeStep(), 0.005);
    EXPECT_DECIMAL_EQ(config1.getSimulationDuration(), 50);
    EXPECT_EQ(config1.getMaxIterations(), 10000u);
    EXPECT_EQ(config1.getSolver(), "Euler");
    EXPECT_TRUE(config1.getVerbose());

    std::remove(path1.c_str());

    // Partial
    std::string yaml2 = R"(
        timestep: 0.02
    )";
    std::string path2 = createTempConfigFile(yaml2);

    Config& config2 = Config::get();
    config2.loadFromFile(path2);

    EXPECT_DECIMAL_EQ(config2.getTimeStep(), 0.02_d);
    EXPECT_DECIMAL_EQ(config2.getGravity(), 15.2_d);
    EXPECT_EQ(config2.getMaxIterations(), 2500);

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

TEST(ConfigTest, AdditionalGetters)
{
    Config& config = Config::get();

    EXPECT_FALSE(config.getSave()); // default is false
    EXPECT_GE(config.getSimulationDuration(), 0_d);
    EXPECT_GE(config.getDefaultStiffness(), 0_d);
    EXPECT_GE(config.getDefaultDamping(), 0_d);
    EXPECT_GE(config.getDefaultFriction(), 0_d);
    EXPECT_GE(config.getDefaultRestitution(), 0_d);
    EXPECT_TRUE(config.getSimplifiedCollision()); // default is true
}

TEST(ConfigTest, SetterValidation)
{
    Config& config = Config::get();

    EXPECT_THROW(config.setGravity(-1_d), std::invalid_argument);
    EXPECT_THROW(config.setTimeStep(-0.1_d), std::invalid_argument);
    EXPECT_THROW(config.setTimeStep(0_d), std::invalid_argument);
    EXPECT_THROW(config.setSimulationDuration(-5_d), std::invalid_argument);
    EXPECT_THROW(config.setMaxIterations(0), std::invalid_argument);
}

TEST(ConfigTest, LoadFromFile_Extended)
{
    std::string yaml = R"(
        save: true
        simplifiedCollision: false
        material:
            stiffness: 2000.0
            damping: 80.0
            friction: 0.5
            restitution: 0.6
    )";
    std::string path = createTempConfigFile(yaml);

    Config& config = Config::get();
    config.loadFromFile(path);

    EXPECT_TRUE(config.getSave());
    EXPECT_FALSE(config.getSimplifiedCollision());
    EXPECT_DECIMAL_EQ(config.getDefaultStiffness(), 2000_d);
    EXPECT_DECIMAL_EQ(config.getDefaultDamping(), 80_d);
    EXPECT_DECIMAL_EQ(config.getDefaultFriction(), 0.5_d);
    EXPECT_DECIMAL_EQ(config.getDefaultRestitution(), 0.6_d);

    config.setSave(false);
    config.setSimplifiedCollision(true);

    std::remove(path.c_str());
}

TEST(ConfigTest, OverrideFromCommandLine_Extended)
{
    Config& config = Config::get();

    const char* argv[] = { "prog", "--duration", "30.0", "--verbose", "false", "--save", "true" };
    int         argc   = sizeof(argv) / sizeof(argv[0]);
    config.overrideFromCommandLine(argc, const_cast<char**>(argv));

    EXPECT_DECIMAL_EQ(config.getSimulationDuration(), 30_d);
    EXPECT_FALSE(config.getVerbose());
    EXPECT_TRUE(config.getSave());

    config.setVerbose(true);
    config.setSave(false);
}

TEST(ConfigTest, OverrideFromCommandLineInvalid)
{
    Config& config = Config::get();

    // Test invalid args
    const char* argv[] = { "program", "--unknow", "999", "--gravity" };
    int         argc   = sizeof(argv) / sizeof(argv[0]);

    config.overrideFromCommandLine(argc, const_cast<char**>(argv));

    // Values unchanged
    EXPECT_DOUBLE_EQ(config.getGravity(), 9.81_d);
    EXPECT_DOUBLE_EQ(config.getTimeStep(), 0.01_d);
    EXPECT_EQ(config.getMaxIterations(), 1000u);

    // Test valid args
    const char* _argv[] = { "program", "--gravity", "1.62", "--solver", "RK4", "--timestep", "0.005" };
    int         _argc   = sizeof(_argv) / sizeof(_argv[0]);

    config.overrideFromCommandLine(_argc, const_cast<char**>(_argv));

    // Values changed
    EXPECT_DOUBLE_EQ(config.getGravity(), 1.62_d);
    EXPECT_DOUBLE_EQ(config.getTimeStep(), 0.005_d);
    EXPECT_EQ(config.getSolver(), "RK4");
}
