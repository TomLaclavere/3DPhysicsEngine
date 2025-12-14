// config.cpp
#include "world/config.hpp"

#include <iostream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

// ============================================================================
//  Getters
// ============================================================================
Config& Config::get()
{
    static Config instance;
    return instance;
}
decimal      Config::getGravity() const { return gravity; }
decimal      Config::getTimeStep() const { return timeStep; }
unsigned int Config::getMaxIterations() const { return maxIterations; }
std::string  Config::getSolver() const { return solver; }
bool         Config::getVerbose() const { return verbose; }

// ============================================================================
//  Loading Methods
// ============================================================================
void Config::loadFromFile(const std::string& path)
{
    try
    {
        YAML::Node node = YAML::LoadFile(path);

        if (node["gravity"])
            gravity = node["gravity"].as<decimal>();
        if (node["timestep"])
            timeStep = node["timestep"].as<decimal>();
        if (node["solverIters"])
            maxIterations = node["solverIters"].as<unsigned>();
        if (node["solver"])
            solver = node["solver"].as<std::string>();
        if (node["verbose"])
            verbose = node["verbose"].as<bool>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
        throw;
    }
}

void Config::overrideFromCommandLine(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--gravity" && i + 1 < argc)
        {
            gravity = std::stod(argv[++i]);
        }
        else if (arg == "--timestep" && i + 1 < argc)
        {
            timeStep = std::stod(argv[++i]);
        }
        else if (arg == "--iters" && i + 1 < argc)
        {
            maxIterations = std::stoi(argv[++i]);
        }
        else if (arg == "--solver" && i + 1 < argc)
        {
            solver = std::string(argv[++i]);
        }
    }
}
