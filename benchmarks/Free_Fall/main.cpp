/**
 * @file main.cpp
 *
 * @brief Free Fall Benchmark
 *
 */

#include "mathematics/common.hpp"
#include "mathematics/math_io.hpp"
#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

decimal simulation(std::string solver, decimal timestep, int maxiter)
{
    Timer totalTimer;

    // Load configuration
    Timer   configTimer;
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(timestep);
    config.setMaxIterations(maxiter);
    Contact contact;

    // Initialize simulation
    Timer        initTimer;
    PhysicsWorld world(config);
    world.initialize();
    auto* ground = new Plane(Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    auto* sphere = new Sphere(Vector3D(0_d, 0_d, 20_d), 0.2_d, Vector3D(0_d, 0_d, -1_d), 1_d);

    sphere->setIsFixed(false);

    world.addObject(sphere);
    world.addObject(ground);
    world.start();

    decimal simulationContactTimeSphere = 0_d;

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

        if (sphere->computeCollision(*ground, contact) && simulationContactTimeSphere == 0_d)
        {
            simulationContactTimeSphere = time;
        }
        ++counter;
    }
    world.clearObjects();

    return simulationContactTimeSphere;
}

int main(int argc, char** argv)
{
    // Simulations parameters
    decimal analyticalContactTimeSphere = 1.914861584038593_d;
    decimal totalTime                   = 2_d;

    // Arrays of tested parameters
    std::array<std::string, 3>        solvers { "Euler", "Verlet", "RK4" };
    std::array<decimal, 50>           timesteps;
    std::array<int, timesteps.size()> maxIterations;

    decimal dt_max = 0.5_d;
    decimal dt_min = 1e-5_d;

    // Logarithmic spacing for timesteps
    for (size_t i = 0; i < timesteps.size(); ++i)
    {
        // Exponent for log spacing: from 0 to 1
        decimal alpha = decimal(i) / decimal(timesteps.size() - 1);
        // Logarithmic interpolation: dt = dt_max * (dt_min/dt_max)^alpha
        timesteps[i]     = dt_max * std::pow(dt_min / dt_max, alpha);
        maxIterations[i] = static_cast<int>(totalTime / timesteps[i]);
    }

    std::array<std::array<decimal, timesteps.size()>, solvers.size()> results;

    // Benchmark
    for (std::size_t iSolver = 0; iSolver < solvers.size(); ++iSolver)
    {
        for (std::size_t jIter = 0; jIter < timesteps.size(); ++jIter)
        {
            results[iSolver][jIter] =
                commonMaths::absVal(simulation(solvers[iSolver], timesteps[jIter], maxIterations[jIter]) -
                                    analyticalContactTimeSphere);
        }
    }

    // Save Benchmark into CSV
    std::ofstream file("benchmarks/Free_Fall/benchmark.csv");

    if (!file)
    {
        std::cerr << "Cannot open output file\n";
        return 1;
    }

    file << "solver,dt,error\n";

    for (std::size_t iSolver = 0; iSolver < solvers.size(); ++iSolver)
    {
        for (std::size_t jIter = 0; jIter < timesteps.size(); ++jIter)
        {
            file << solvers[iSolver] << "," << timesteps[jIter] << "," << results[iSolver][jIter] << "\n";
        }
    }

    file.close();

    return 0;
}
