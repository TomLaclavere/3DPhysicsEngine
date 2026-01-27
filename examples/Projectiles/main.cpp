/**
 * @file main.cpp
 *
 * @brief Projectiles Simulation
 *
 * This file is building a physical world to test the motion of various projectiles under Earth gravity
 * (without atmosphere, for now) on the objects implemented in the simulation. Its goal is to compare the time
 * where these objects hit the ground between this simulation and the analytical computation. It will then
 * allows to verify the physical coherence of this simulation, as well as the collision code for each objects.
 * This example will also test higher number for speed and ground size than usual.
 * @see examples/Free_Fall/README.md for complete explation.
 *
 */

#include "mathematics/math_io.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
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
    Timer totalTimer;

    // Load configuration
    Timer   configTimer;
    Config& config = Config::get();
    config.loadFromFile("examples/Projectiles/config.yml");
    config.overrideFromCommandLine(argc, argv);

    std::cout << "----------------------------------------\n";
    std::cout << "Simulation Parameters:\n";
    std::cout << "Gravity: " << config.getGravity() << " m/s²\n";
    std::cout << "Timestep: " << config.getTimeStep() << " s\n";
    std::cout << "Max iterations: " << config.getMaxIterations() << "\n";
    std::cout << "Loading configuration took: " << configTimer.elapsedMilliseconds() << " ms\n";

    // Initialize simulation
    Timer        initTimer;
    PhysicsWorld world(config);
    auto* ground = new Plane(Vector3D(0_d), Vector3D(100000_d, 100000_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    auto* sphereBulletMotion  = new Sphere(Vector3D(0_d, 0_d, 1.5_d), 0.2_d, Vector3D(1000_d, 0_d, 0_d), 1_d);
    auto* planeVerticalMotion = new Plane(Vector3D(10_d, 0_d, 1_d), Vector3D(0.2_d, 0.2_d, 0_d),
                                          Vector3D(0_d, 0_d, 100_d), 1_d, Vector3D(0_d, 1_d, 0_d));
    auto* cubeParabolicMotion =
        new AABB(Vector3D(0_d, 10_d, 1_d), Vector3D(0.1_d), Vector3D(0_d, 50_d, 50_d), 1_d);
    sphereBulletMotion->setIsFixed(false);
    planeVerticalMotion->setIsFixed(false);
    cubeParabolicMotion->setIsFixed(false);

    world.addObject(sphereBulletMotion);
    world.addObject(planeVerticalMotion);
    world.addObject(cubeParabolicMotion);
    world.addObject(ground);
    world.start();

    // Contact times with the ground, computed in README.md
    decimal  analyticalContactTimeSphere   = 0.5342499768054424_d;
    decimal  analyticalContactTimePlane    = 20.39535670016235_d;
    decimal  analyticalContactTimeCube     = 10.212231941071188_d;
    Vector3D analyticalFinalPositionSphere = Vector3D(534.249977_d, 0_d, 0_d);
    Vector3D analyticalFinalPositionPlane  = Vector3D(10_d, 0_d, 0_d);
    Vector3D analyticalFinalPositionCube   = Vector3D(0_d, 520.611597_d, 0_d);

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

        const decimal time = static_cast<decimal>(counter) * timeStep;

        world.integrateWithoutCollisions();

        if (counter % 1000 == 0)
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

        if (sphereBulletMotion->checkCollision(*ground) && simulationContactTimeSphere == 0_d)
            simulationContactTimeSphere = time;
        if (planeVerticalMotion->checkCollision(*ground) && simulationContactTimePlane == 0_d)
            simulationContactTimePlane = time;
        if (cubeParabolicMotion->checkCollision(*ground) && simulationContactTimeCube == 0_d)
            simulationContactTimeCube = time;

        ++counter;
    }

    const double    simTimeSec = simulationTimer.elapsedSeconds();
    const long long avgStepUs  = simulationTimer.elapsedMicroseconds() / static_cast<long long>(maxIter);

    std::cout << "\nSimulation took: " << simTimeSec << " s\n";
    std::cout << "Average iteration time: " << avgStepUs << " µs\n";
    std::cout << "Total execution time: " << totalTimer.elapsedSeconds() << " s\n";
    std::cout << '\n';

    // Verify contact times
    std::cout << "Note : The time comparison are done with an approximation of 2 times the timestep value, "
                 "in order to prevent numerical uncertainties. \n";
    // Verify Sphere contact time
    if (commonMaths::approxEqual(analyticalContactTimeSphere, simulationContactTimeSphere, 2 * timeStep))
        std::cout
            << "Bullet  motion (sphere) : analytical and simulation times are compatible. Contact time = "
            << analyticalContactTimeSphere << " (s).\n";
    else
        std::cout << "Bullet  motion (sphere) : analytical and simulation times are not compatible. "
                     "Analytical contact time = "
                  << analyticalContactTimeSphere
                  << " (s), and Simulation contact time = " << simulationContactTimeSphere << " (s).\n";
    // Verify Plane contact time
    if (commonMaths::approxEqual(analyticalContactTimePlane, simulationContactTimePlane, 2 * timeStep))
        std::cout
            << "Vertical motion (plane) : analytical and simulation times are compatible. Contact time = "
            << analyticalContactTimePlane << " (s).\n";
    else
        std::cout << "Vertical motion (plane) : analytical and simulation times are not compatible. "
                     "Analytical contact time = "
                  << analyticalContactTimePlane
                  << " (s), and Simulation contact time = " << simulationContactTimePlane << " (s).\n";
    // Verify Cube contact time
    if (commonMaths::approxEqual(analyticalContactTimeCube, simulationContactTimeCube, 2 * timeStep))
        std::cout
            << "Parabolic Motion (cube) : analytical and simulation times are compatible. Contact time = "
            << analyticalContactTimeCube << " (s).\n";
    else
        std::cout << "Parabolic Motion (cube) : analytical and simulation times are not compatible. "
                     "Analytical contact time = "
                  << analyticalContactTimeCube
                  << " (s), and Simulation contact time = " << simulationContactTimeCube << " (s).\n";

    // Verify final position

    // Verify Sphere final position
    if (analyticalFinalPositionSphere.approxEqual(sphereBulletMotion->getPosition(), 0.2_d))
        std::cout << "Bullet  motion (sphere) : analytical and simulation final position are compatible. "
                     "Final position = "
                  << analyticalFinalPositionSphere << " (m).\n";
    else
        std::cout << "Bullet  motion (sphere) : analytical and simulation final position are not compatible. "
                     "Analytical Final position = "
                  << analyticalFinalPositionSphere
                  << " (m), and Simulation Final position = " << sphereBulletMotion->getPosition()
                  << " (m).\n";
    // Verify Plane final position
    if (analyticalFinalPositionPlane.approxEqual(planeVerticalMotion->getPosition(), 0.2_d))
        std::cout << "Bullet  motion (plane) : analytical and simulation final position are compatible. "
                     "Final position = "
                  << analyticalFinalPositionPlane << " (m).\n";
    else
        std::cout << "Bullet  motion (plane) : analytical and simulation final position are not compatible. "
                     "Analytical Final position = "
                  << analyticalFinalPositionPlane
                  << " (m), and Simulation Final position = " << planeVerticalMotion->getPosition()
                  << " (m).\n";
    // Verify Cube final position
    if (analyticalFinalPositionCube.approxEqual(cubeParabolicMotion->getPosition(), 0.2_d))
        std::cout << "Bullet  motion (cube) : analytical and simulation final position are compatible. "
                     "Final position = "
                  << analyticalFinalPositionCube << " (m).\n";
    else
        std::cout << "Bullet  motion (cube) : analytical and simulation final position are not compatible. "
                     "Analytical Final position = "
                  << analyticalFinalPositionCube
                  << " (m), and Simulation Final position = " << cubeParabolicMotion->getPosition()
                  << " (m).\n";

    world.clearObjects();

    return 0;
}
