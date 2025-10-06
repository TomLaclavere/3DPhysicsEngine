#include "config.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.cpp"
#include "world/physicsWorld.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

// ------------------------------------------------------------------------
// Helper: format a Vector3D as a fixed-width string
// ------------------------------------------------------------------------
inline std::string formatVector(const Vector3D& v)
{
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(3) << "(" << std::setw(10) << v[0] << ", " << std::setw(10)
        << v[1] << ", " << std::setw(10) << v[2] << ")";
    return oss.str();
}

// ============================================================================
// Main entry point
// ============================================================================
int main(int argc, char** argv)
{
    Timer totalTimer;

    // Load configuration
    Timer   configTimer;
    Config& config = Config::get();
    config.loadFromFile("src/config.yaml");
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
    auto*        sphere = new Sphere(Vector3D(0_d, 0_d, 0_d), 1_d, 1_d);
    world.addObject(sphere);
    world.start();

    std::cout << "Initializing world took: " << initTimer.elapsedMilliseconds() << " ms\n\n";

    // Column widths
    constexpr int col_time = 10;
    constexpr int col_vec  = 40;
    constexpr int col_step = 12;

    // Header
    std::cout << std::left << std::setw(col_time) << "Time(s)" << std::setw(col_vec) << "Position(x,y,z)"
              << std::setw(col_vec) << "Velocity(x,y,z)" << std::setw(col_step) << "Step (µs)\n";
    std::cout << std::string(col_time + 2 * col_vec + col_step, '-') << "\n";

    // Simulation loop
    Timer         simulationTimer;
    const decimal timeStep = config.getTimeStep();
    const size_t  maxIter  = config.getMaxIterations();

    for (size_t counter = 0; counter < maxIter; ++counter)
    {
        Timer stepTimer;

        const decimal  time = counter * timeStep;
        const Vector3D pos  = sphere->getPosition();
        const Vector3D vel  = sphere->getVelocity();

        world.update(timeStep);

        if (counter % 100 == 0)
        {
            std::cout << std::left << std::setw(col_time) << std::fixed << std::setprecision(3) << time
                      << std::setw(col_vec) << formatVector(pos) << std::setw(col_vec) << formatVector(vel)
                      << std::right << std::setw(col_step) << stepTimer.elapsedMicroseconds() << "\n";
        }
    }

    const double simTimeSec = simulationTimer.elapsedSeconds();
    const double avgStepUs  = simulationTimer.elapsedMicroseconds() / static_cast<double>(maxIter);

    std::cout << "\nSimulation took: " << simTimeSec << " s\n";
    std::cout << "Average iteration time: " << avgStepUs << " µs\n";
    std::cout << "Total execution time: " << totalTimer.elapsedSeconds() << " s\n";

    delete sphere;
    return 0;
}
