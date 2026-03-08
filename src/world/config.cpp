// config.cpp
#include "world/config.hpp"

#include "precision.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

//  Getters
Config& Config::get()
{
    static Config instance;
    return instance;
}
decimal     Config::getGravity() const { return gravity; }
decimal     Config::getTimeStep() const { return timeStep; }
decimal     Config::getSimulationDuration() const { return simulationDuration; }
std::size_t Config::getMaxIterations() const { return maxIterations; }
std::string Config::getSolver() const { return solver; }
bool        Config::getVerbose() const { return verbose; }
bool        Config::getSave() const { return save; }

//  Loading Methods
void Config::loadFromFile(const std::string& path)
{
    try
    {
        YAML::Node node = YAML::LoadFile(path);

        if (node["gravity"])
            setGravity(node["gravity"].as<decimal>());
        if (node["timestep"])
            setTimeStep(node["timestep"].as<decimal>());
        if (node["duration"])
            setSimulationDuration(node["duration"].as<decimal>());
        if (node["solver"])
            setSolver(node["solver"].as<std::string>());
        if (node["verbose"])
            setVerbose(node["verbose"].as<bool>());
        if (node["save"])
            setSave(node["save"].as<bool>());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading config file: " << e.what() << "\n";
        throw;
    }
}

void Config::overrideFromCommandLine(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--gravity" && i + 1 < argc)
            setGravity(static_cast<decimal>(std::stold(argv[++i])));
        else if (arg == "--timestep" && i + 1 < argc)
            setTimeStep(static_cast<decimal>(std::stold(argv[++i])));
        else if (arg == "--duration" && i + 1 < argc)
            setSimulationDuration(static_cast<decimal>(std::stold(argv[++i])));
        else if (arg == "--iters" && i + 1 < argc)
            setMaxIterations(static_cast<std::size_t>(std::stoul(argv[++i])));
        else if (arg == "--solver" && i + 1 < argc)
            setSolver(std::string(argv[++i]));
        else if (arg == "--verbose" && i + 1 < argc)
        {
            std::string v = argv[++i];
            setVerbose(v == "1" || v == "true" || v == "yes");
        }
        else if (arg == "--save" && i + 1 < argc)
        {
            std::string s = argv[++i];
            setSave(s == "1" || s == "true" || s == "yes");
        }
        else
            continue;
    }
}
