/**
 * @file main.cpp
 *
 * @brief Free Fall Simulation
 *
 * This file is building a physical world to test the free fall under Earth gravity (without atmosphere, for
 * now) on the objects implemented in the simulation. Its goal is to compare the time where these objects hit
 * the ground between this simulation and the analytical computation. It will then allows to verify the
 * physical coherence of this simulation, as well as the collision code for each objects.
 * @see examples/Free_Fall/README.md for complete explation.
 *
 */

#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

// ============================================================================
// Main entry point
// ============================================================================
int main(int argc, char** argv)
{
    Timer totalTimer;

    // Load configuration
    Timer   configTimer;
    Config& config = Config::get();
    config.loadFromFile("examples/Free_Fall/config.yml");
    config.overrideFromCommandLine(argc, argv);

    std::cout << "Machine epsilon: " << PRECISION_MACHINE << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Simulation Parameters:\n";
    std::cout << "Gravity: " << config.getGravity() << " m/s²\n";
    std::cout << "Timestep: " << config.getTimeStep() << " s\n";
    std::cout << "Max iterations: " << config.getMaxIterations() << "\n";
    std::cout << "Solver: " << config.getSolver() << std::endl;
    std::cout << "Loading configuration took: " << configTimer.elapsedMilliseconds() << " ms\n";

    // Initialize simulation
    Timer        initTimer;
    PhysicsWorld world(config);
    auto*        ground = new Plane(Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    auto*        sphere = new Sphere(Vector3D(0_d, 0_d, 20_d), 0.2_d, Vector3D(0_d, 0_d, -1_d), 1_d);
    auto*        plane  = new Plane(Vector3D(10_d, 0_d, 15_d), Vector3D(1_d, 0.4_d, 0_d), Vector3D(0_d), 1_d,
                                    Vector3D(0_d, 1_d, 0_d));
    auto*        cube   = new AABB(Vector3D(0_d, 10_d, 10_d), Vector3D(0.15_d), Vector3D(0_d), 1_d);
    sphere->setIsFixed(false);
    plane->setIsFixed(false);
    cube->setIsFixed(false);

    world.addObject(sphere);
    world.addObject(plane);
    world.addObject(cube);
    world.addObject(ground);
    world.start();

    // Contact times with the ground, computed in README.md
    decimal analyticalContactTimeSphere = 1.915_d;
    decimal analyticalContactTimePlane  = 1.737_d;
    decimal analyticalContactTimeCube   = 1.420_d;

    decimal simulationContactTimeSphere = 0_d;
    decimal simulationContactTimePlane  = 0_d;
    decimal simulationContactTimeCube   = 0_d;

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
        Timer stepTimer;

        const decimal time = counter * timeStep;

        world.integrate();

        if (counter % 25 == 0)
        {
            for (auto* obj : world.getObject())
            {
                if (!obj->isFixed())
                    std::cout << std::left << std::setw(col_obj) << obj->getType() << std::setw(col_time)
                              << std::fixed << std::setprecision(3) << time << std::setw(col_vec)
                              << obj->getPosition().formatVector() << std::setw(col_vec)
                              << obj->getVelocity().formatVector() << std::right << std::setw(col_step)
                              << stepTimer.elapsedMicroseconds() << "\n";
            }
            std::cout << std::string(n, '-') << '\n';
        }

        if (sphere->checkCollision(*ground) && simulationContactTimeSphere == 0_d)
            simulationContactTimeSphere = time;
        if (plane->checkCollision(*ground) && simulationContactTimePlane == 0_d)
            simulationContactTimePlane = time;
        if (cube->checkCollision(*ground) && simulationContactTimeCube == 0_d)
            simulationContactTimeCube = time;

        ++counter;
    }

    const double simTimeSec = simulationTimer.elapsedSeconds();
    const double avgStepUs  = simulationTimer.elapsedMicroseconds() / static_cast<double>(maxIter);

    std::cout << "\nSimulation took: " << simTimeSec << " s\n";
    std::cout << "Average iteration time: " << avgStepUs << " µs\n";
    std::cout << "Total execution time: " << totalTimer.elapsedSeconds() << " s\n";
    std::cout << '\n';

    // Verify Sphere contact times
    std::cout << "Note : The time comparison are done with an approximation of 2 times the timestep value, "
                 "in order to prevent numerical uncertainties. \n";
    if (commonMaths::approxEqual(analyticalContactTimeSphere, simulationContactTimeSphere, 2 * timeStep))
        std::cout << "Sphere : analytical and simulation times are compatible. Contact time = "
                  << analyticalContactTimeSphere << " (s).\n";
    else
        std::cout << "Sphere : analytical and simulation times are not compatible. Analytical contact time = "
                  << analyticalContactTimeSphere
                  << " (s), and Simulation contact time = " << simulationContactTimeSphere << " (s).\n";
    std::cout << analyticalContactTimeSphere - simulationContactTimeSphere << std::endl;
    // Verify Plane contact times
    if (commonMaths::approxEqual(analyticalContactTimePlane, simulationContactTimePlane, 2 * timeStep))
        std::cout << "Plane : analytical and simulation times are compatible. Contact time = "
                  << analyticalContactTimePlane << " (s).\n";
    else
        std::cout << "Plane : analytical and simulation times are not compatible. Analytical contact time = "
                  << analyticalContactTimePlane
                  << " (s), and Simulation contact time = " << simulationContactTimePlane << " (s).\n";
    std::cout << analyticalContactTimePlane - simulationContactTimePlane << std::endl;
    // Verify Cube contact times
    if (commonMaths::approxEqual(analyticalContactTimeCube, simulationContactTimeCube, 2 * timeStep))
        std::cout << "Cube : analytical and simulation times are compatible. Contact time = "
                  << analyticalContactTimeCube << " (s).\n";
    else
        std::cout << "Cube : analytical and simulation times are not compatible. Analytical contact time = "
                  << analyticalContactTimeCube
                  << " (s), and Simulation contact time = " << simulationContactTimeCube << " (s).\n";
    std::cout << analyticalContactTimeCube - simulationContactTimeCube << std::endl;
    world.clearObjects();

    return 0;
}
