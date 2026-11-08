/**
 * @file main.cpp
 *
 * @brief Bouncing Ball Simulation
 *
 */

#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "precision.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <iostream>
#include <random>

// ============================================================================
// Main entry point
// ============================================================================
int main(int argc, char** argv)
{
    Timer       totalTimer;
    std::string directory = "examples/Random_Balls_2D";

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

    // Obj properties
    int     N_obj      = 20;
    decimal size_obj   = 1;
    decimal size_box   = size_obj * static_cast<decimal>(N_obj);
    decimal size_plane = 2 * size_box;

    // Set Box
    auto* ground = new Plane(Vector3D(0_d, 0_d, -size_box), Vector3D(size_plane, size_plane, 0_d),
                             Vector3D(0_d, 0_d, 1_d));
    ground->setIsFixed(true);
    ground->setName("Ground");
    world.addObject(ground);

    auto* roof = new Plane(Vector3D(0_d, 0_d, size_box), Vector3D(size_plane, size_plane, 0_d),
                           Vector3D(0_d, 0_d, -1_d));
    roof->setIsFixed(true);
    roof->setName("Roof");
    world.addObject(roof);

    auto* wall_right = new Plane(Vector3D(size_box, 0_d, 0_d), Vector3D(size_plane, size_plane, 0_d),
                                 Vector3D(-1_d, 0_d, 0_d));
    wall_right->setIsFixed(true);
    wall_right->setName("Wall right");
    world.addObject(wall_right);

    auto* wall_left = new Plane(Vector3D(-size_box, 0_d, 0_d), Vector3D(size_plane, size_plane, 0_d),
                                Vector3D(1_d, 0_d, 0_d));
    wall_left->setIsFixed(true);
    wall_left->setName("Wall left");
    world.addObject(wall_left);

    // Set balls
    std::mt19937 rng(42);

    const decimal radius = size_obj / 2_d;
    const decimal v0     = 2_d * size_box;

    std::uniform_real_distribution<decimal> randPos(-size_box + radius, size_box - radius);
    std::uniform_real_distribution<decimal> randSpeed(0.5_d * v0, v0);
    std::uniform_real_distribution<decimal> randAngle(0_d, 2_d * std::numbers::pi_v<decimal>);

    std::vector<Vector3D> positions;
    while (positions.size() < static_cast<size_t>(N_obj))
    {
        const decimal  x = randPos(rng);
        const decimal  z = randPos(rng);
        const Vector3D pos(x, 0_d, z);

        const bool isFree = std::ranges::none_of(positions, [&](const Vector3D& other)
                                                 { return (pos - other).getNorm() < size_obj; });
        if (!isFree)
            continue;

        const decimal  speed = randSpeed(rng);
        const decimal  angle = randAngle(rng);
        const Vector3D vel(speed * std::cos(angle), 0_d, speed * std::sin(angle));

        positions.push_back(pos);
        world.addObject(new Sphere(pos, size_obj, vel, 1_d));
    }

    world.start();

    // Initialise saving
    world.initCSV(directory + "/CSV");
    world.saveObjectsCSV();

    std::cout << "Initializing world took: " << initTimer.elapsedMilliseconds() << " ms\n\n";

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

        ++counter;

        // if save, save motion information to CSV
        world.saveMotionCSV(time);
    }
    world.closeCSV();

    const decimal   simTimeSec = simulationTimer.elapsedSeconds();
    const long long avgStepUs  = simulationTimer.elapsedMicroseconds() / static_cast<long long>(maxIter);

    std::cout << "\nSimulation took: " << simTimeSec << " s\n";
    std::cout << "Average iteration time: " << avgStepUs << " µs\n";
    std::cout << "Total execution time: " << totalTimer.elapsedSeconds() << " s\n";
    std::cout << '\n';

    world.clearObjects();
    return 0;
}
