#include "config.hpp"
#include "objects/sphere.hpp"
#include "world/physicsWorld.hpp"

#include <chrono>
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
    auto          totalDuration = std::chrono::high_resolution_clock::now();
    const decimal timeStep      = config.getTimeStep();
    size_t        counter       = 0;
    world.start();

    // Header with better alignment
    std::cout << "\nStarting simulation...\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::setw(8) << "Time(s)" << std::setw(25) << "Position(x,y,z)" << std::setw(25)
              << "Velocity(x,y,z)" << std::setw(20) << "Duration(µs)" << "\n";
    std::cout << std::string(80, '-') << "\n";

    while (counter < config.getMaxIterations())
    {
        auto start = std::chrono::high_resolution_clock::now();

        decimal  time = counter * timeStep;
        Vector3D pos  = sphere->getPosition();
        Vector3D vel  = sphere->getVelocity();

        world.update(timeStep);

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // Print
        if (counter % 100 == 0)
        {
            std::cout << std::setw(10) << time << std::setw(10) << pos << std::setw(10) << vel
                      << std::setw(10) << duration.count() << "\n";
        }

        counter++;
    }

    std::cout << "Simulation took : "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::high_resolution_clock::now() - totalDuration)
              << "\n";

    // Cleanup
    delete sphere;

    return 0;
}
