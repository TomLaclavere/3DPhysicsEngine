#include "config.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    // Load configuration
    Config& config = Config::get();
    config.loadFromFile("src/config.yaml");

    // Print
    std::cout << "Gravity: " << config.getGravity() << " m/s^2\n";
    std::cout << "Timestep: " << config.getTimeStep() << " s\n";
    std::cout << "Max iterations: " << config.getMaxIterations() << "\n";

    return 0;
}
