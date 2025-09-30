#include "config.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    // Load configuration
    Config& config = Config::get();
    config.loadFromFile("src/config.yaml");

    // Print
    std::cout << "Gravity: " << config.gravity << " m/s^2\n";
    std::cout << "Timestep: " << config.timeStep << " s\n";
    std::cout << "Max iterations: " << config.maxIterations << "\n";

    return 0;
}
