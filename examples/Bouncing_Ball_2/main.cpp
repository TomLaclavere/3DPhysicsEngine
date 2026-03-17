/**
 * @file main.cpp
 *
 * @brief Bouncing Ball Simulation
 *
 */

#include "mathematics/math_io.hpp"
#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "precision.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <iomanip>
#include <iostream>

// ============================================================================
// Main entry point
// ============================================================================
int main(int argc, char** argv)
{
    Timer       totalTimer;
    std::string directory = "examples/Bouncing_Ball_2";

    // Load configuration
    Timer   configTimer;
    Config& config = Config::get();
    Contact contact;
    config.loadFromFile(directory + "/config.yml");
    config.overrideFromCommandLine(argc, argv);

    std::cout << "----------------------------------------\n";
    std::cout << "Simulation Parameters:\n";
    std::cout << "Gravity: " << config.getGravity() << " m/s²\n";
    std::cout << "Duration: " << config.getSimulationDuration() << " s\n";
    std::cout << "Timestep: " << config.getTimeStep() << " s\n";
    std::cout << "Max iterations: " << config.getMaxIterations() << "\n";
    std::cout << "Solver: " << config.getSolver() << "\n";
    std::cout << "Verbose: " << config.getVerbose() << "\n";
    std::cout << "Save: " << config.getSave() << "\n";
    std::cout << "Loading configuration took: " << configTimer.elapsedMilliseconds() << " ms\n";

    // Initialize simulation
    Timer        initTimer;
    PhysicsWorld world(config);
    auto*        ground_1 = new Plane(Vector3D(2_d, 0_d, 0_d), Vector3D(10_d), Vector3D(-1_d, 0_d, 1_d));
    auto*        ground_2 = new Plane(Vector3D(-2_d, 0_d, 0_d), Vector3D(10_d), Vector3D(1_d, 0_d, 1_d));
    auto*        sphere   = new Sphere(Vector3D(2_d, 0_d, 20_d), 2_d, Vector3D(0_d, 0_d, -1_d), 1_d);

    sphere->setIsFixed(false);
    sphere->setName("Bouncing Ball");
    ground_1->setName("Ground 1");
    ground_2->setName("Ground 2");
    world.addObject(sphere);
    world.addObject(ground_1);
    world.addObject(ground_2);
    world.start();

    // Initialise saving
    world.initCSV(directory + "/CSV");
    world.saveObjectsCSV();

    std::cout << "Initializing world took: " << initTimer.elapsedMilliseconds() << " ms\n\n";

    // Column widths
    constexpr int col_obj  = 10;
    constexpr int col_time = 10;
    constexpr int col_vec  = 40;
    constexpr int col_step = 12;
    size_t        n        = col_obj + col_time + 2 * col_vec + col_step;

    // Header
    std::cout << std::left << std::setw(col_obj) << "Object" << std::setw(col_time) << "Time(s)"
              << std::setw(col_vec) << "Position(x,y,z)" << std::setw(col_vec) << "Velocity(x,y,z)"
              << std::setw(col_step) << "Step (µs)\n";
    std::cout << std::string(n, '-') << "\n";

    // Simulation loop
    Timer         simulationTimer;
    const decimal timeStep = config.getTimeStep();
    const size_t  maxIter  = config.getMaxIterations();
    size_t        counter  = 0;

    while (counter < maxIter && world.getIsRunning())
    {
        Timer         stepTimer;
        const decimal time = static_cast<decimal>(counter) * timeStep;

        world.integrate();

        if (counter % 25 == 0)
        {
            for (auto* obj : world.getObject())
            {
                if (!obj->isFixed())
                    std::cout << std::left << std::setw(col_obj) << obj->getType() << std::setw(col_time)
                              << std::fixed << std::setprecision(3) << time << std::setw(col_vec)
                              << formatVector(obj->getPosition()) << std::setw(col_vec)
                              << formatVector(obj->getVelocity()) << std::right << std::setw(col_step)
                              << stepTimer.elapsedMicroseconds() << "\n";
            }
            std::cout << std::string(n, '-') << '\n';
        }
        ++counter;

        // if save, save motion information to CSV
        world.saveMotionCSV(time);
    }

    const decimal   simTimeSec = simulationTimer.elapsedSeconds();
    const long long avgStepUs  = simulationTimer.elapsedMicroseconds() / static_cast<long long>(maxIter);

    std::cout << "\nSimulation took: " << simTimeSec << " s\n";
    std::cout << "Average iteration time: " << avgStepUs << " µs\n";
    std::cout << "Total execution time: " << totalTimer.elapsedSeconds() << " s\n";
    std::cout << '\n';

    world.clearObjects();
    return 0;
}
