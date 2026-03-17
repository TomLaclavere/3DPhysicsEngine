#include "objects/material.hpp"

#include "precision.hpp"

#include <iostream>
#include <yaml-cpp/yaml.h>

// Getters
Material& Material::get() const
{
    static Material instance;
    return instance;
}
std::string Material::getName() const { return name; }
decimal     Material::getYoung() const { return young; }
decimal     Material::getDamping() const { return damping; }
decimal     Material::getFriction() const { return friction; }
decimal     Material::getRestitution() const { return restitution; }

// Loading Methods
void Material::loadFromFile(const std::string& path)
{
    try
    {
        YAML::Node node = YAML::LoadFile(path);

        if (node["name"])
            setName(node["name"].as<std::string>());
        if (node["young"])
            setYoung(node["young"].as<decimal>());
        if (node["damping"])
            setDamping(node["damping"].as<decimal>());
        if (node["friction"])
            setFriction(node["friction"].as<decimal>());
        if (node["restitution"])
            setRestitution(node["restitution"].as<decimal>());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading config file: " << e.what() << "\n";
        throw;
    }
}