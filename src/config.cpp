// config.cpp
#include "config.hpp"

#include <iostream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

Config& Config::get()
{
    static Config instance;
    return instance;
}

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
    }
}
