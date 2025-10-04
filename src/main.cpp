#include "config.hpp"
#include "objects/sphere.hpp"
#include "world/physicsWorld.hpp"

#include <iomanip>
#include <iostream>

int main(int argc, char** argv)
{
    // Load configuration
    Config& config = Config::get();
    config.loadFromFile("src/config.yaml");

    // Override with command line arguments if provided
    config.overrideFromCommandLine(argc, argv);

    // Print
    std::cout << "----------------------------------------\n";
    std::cout << "Simulation Parameters :\n";
    std::cout << "Gravity: " << config.getGravity() << " m/s^2\n";
    std::cout << "Timestep: " << config.getTimeStep() << " s\n";
    std::cout << "Max iterations: " << config.getMaxIterations() << "\n";

    // Initialize simulation
    PhysicsWorld world = PhysicsWorld(config);

    // Add objects
    auto* sphere = new Sphere(Vector3D(0_d, 0_d, 0_d), 1_d, 1_d);
    world.addObject(sphere);

    // Main loop
    const decimal timeStep = config.getTimeStep();
    size_t        counter  = 0;
    world.start();

    // Header with better alignment
    std::cout << "\nStarting simulation...\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << std::setw(8) << "Time(s)" << std::setw(25) << "Position(x,y,z)" << std::setw(25)
              << "Velocity(x,y,z)" << "\n";
    std::cout << std::string(98, '-') << "\n";

    while (counter < config.getMaxIterations())
    {
        decimal  time = counter * timeStep;
        Vector3D pos  = sphere->getPosition();
        Vector3D vel  = sphere->getVelocity();

        // Print every 10 iterations
        if (counter % 10 == 0)
        {
            std::cout << std::setw(8) << time << std::setw(10) << pos << std::setw(10) << vel << "\n";
        }

        world.update(timeStep);
        counter++;
    }

    // Cleanup
    delete sphere;

    return 0;
}
