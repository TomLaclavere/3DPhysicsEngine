/**
 * @file main.cpp
 *
 * @brief Bouncing Benchmark
 *
 * Metrics measured per simulation:
 *   - Contact time error    : |t_contact_numerical - t_contact_analytical|
 *   - Max energy drift      : max( |E(t) - E0| / E0 )
 *   - Final energy drift    : |E(T) - E0| / E0
 *   - CPU time (ms)         : duration of the simulation loop
 *
 * Two CSV files are produced:
 *   benchmark.csv      — one row per (solver, dt), all aggregated metrics
 *   energy_drift.csv   — one row per (solver, dt, timestep) to plot E(t)
 *                        (only for a subset of dt values to limit file size)
 */

#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

//------------------------------------------------------------------------
// Result structure
//------------------------------------------------------------------------

struct SimResult
{
    decimal maxEnergyDrift;
    decimal finalEnergyDrift;
    decimal cpuMs;

    int     bounceCount;
    decimal maxHeightError;

    std::vector<decimal> energyTimes;
    std::vector<decimal> energyDrifts;
};

//------------------------------------------------------------------------
// Total mechanical energy of the sphere
// E = Ek + Ep = 0.5 * m * v^2 + m * g * z
//------------------------------------------------------------------------

static inline decimal computeEnergy(const Sphere& sphere, decimal g = 9.81_d)
{
    const Vector3D& v          = sphere.getVelocity();
    const decimal   velocitySq = v.dotProduct(v);
    const decimal   z          = sphere.getPosition().getZ();
    const decimal   mass       = sphere.getMass();
    return 0.5_d * mass * velocitySq + mass * g * z;
}

//------------------------------------------------------------------------
// Single simulation run
//------------------------------------------------------------------------

SimResult simulation(const std::string& solver, decimal timestep, int maxiter, bool recordEnergy = false)
{
    // Configuration
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(timestep);
    config.setMaxIterations(maxiter);
    config.setSimplifiedCollision(true);
    Contact contact;

    // Scene setup
    PhysicsWorld world(config);
    auto ground = std::make_unique<Plane>(Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    auto sphere = std::make_unique<Sphere>(Vector3D(0_d, 0_d, 20_d), 2_d, 1_d);

    sphere->setIsFixed(false);
    sphere->setRestitutionCst(0.9_d);
    world.addObject(sphere.get());
    world.addObject(ground.get());
    world.start();

    // Initial energy reference
    const decimal E0 = computeEnergy(*sphere);

    // Result initialisation
    SimResult result;
    result.maxEnergyDrift   = 0_d;
    result.finalEnergyDrift = 0_d;
    result.cpuMs            = 0_d;

    const decimal timeStep = config.getTimeStep();
    const auto    maxIter  = static_cast<size_t>(config.getMaxIterations());

    // Sub-sample energy recording to at most 500 points
    const size_t recordEvery = recordEnergy ? std::max(size_t(1), maxIter / 500) : 0;

    // Simulation loop
    Timer  simulationTimer;
    size_t counter = 0;

    decimal previousVz  = sphere->getVelocity().getZ();
    decimal currentPeak = sphere->getPosition().getZ();
    decimal h0          = currentPeak;
    decimal restitution = 1_d;

    int     bounceCount    = 0;
    decimal maxHeightError = 0_d;

    while (counter < maxIter && world.getIsRunning())
    {
        const decimal time = static_cast<decimal>(counter) * timeStep;

        world.integrate();

        const decimal z  = sphere->getPosition().getZ();
        const decimal vz = sphere->getVelocity().getZ();

        // Track peak while going up
        if (vz > 0_d)
            currentPeak = std::max(currentPeak, z);

        // Detect bounce (velocity flip: downward → upward)
        // Detect peak (top of trajectory)
        if (previousVz > 0_d && vz < 0_d)
        {
            // Expected height AFTER previous bounce
            const decimal expected = h0 * std::pow(restitution, 2 * bounceCount);

            const decimal err = commonMaths::absVal((z - expected) / expected);
            if (err > maxHeightError)
                maxHeightError = err;
        }

        // Detect bounce (for counting only)
        if (previousVz < 0_d && vz > 0_d)
        {
            bounceCount++;
        }

        previousVz = vz;

        // Energy drift
        const decimal E     = computeEnergy(*sphere);
        const decimal drift = (E0 != 0_d) ? commonMaths::absVal((E - E0) / E0) : commonMaths::absVal(E - E0);

        if (drift > result.maxEnergyDrift)
            result.maxEnergyDrift = drift;

        if (recordEnergy && (counter % recordEvery == 0))
        {
            result.energyTimes.push_back(time);
            result.energyDrifts.push_back(drift);
        }

        ++counter;
    }

    result.cpuMs          = simulationTimer.elapsedMicroseconds();
    result.bounceCount    = bounceCount;
    result.maxHeightError = maxHeightError;

    // Final energy drift
    {
        const decimal E = computeEnergy(*sphere);
        result.finalEnergyDrift =
            (E0 != 0_d) ? commonMaths::absVal((E - E0) / E0) : commonMaths::absVal(E - E0);
    }

    world.clearObjects();
    return result;
}

//------------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Analytical reference
    // Free fall: z0=20, v0z=-1, r=0.2, g=9.81
    // Contact when z_centre = r  =>  20 - t - 0.5*9.81*t^2 = 0.2
    // =>  4.905 t^2 + t - 19.8 = 0
    // =>  t = (-1 + sqrt(1 + 4*4.905*19.8)) / (2*4.905)
    const decimal analyticalContactTime = 1.914861584038593_d;
    const decimal totalTime             = 20_d;

    // Timesteps: logarithmic spacing
    constexpr std::size_t     N_DT = 50;
    std::array<decimal, N_DT> timesteps;
    std::array<int, N_DT>     maxIterations;

    const decimal dt_max = 0.5_d;
    const decimal dt_min = 1e-5_d;

    for (size_t i = 0; i < N_DT; ++i)
    {
        const decimal alpha = decimal(i) / decimal(N_DT - 1);
        timesteps[i]        = dt_max * std::pow(dt_min / dt_max, alpha);
        maxIterations[i]    = static_cast<int>(totalTime / timesteps[i]);
    }

    // Solvers
    const std::array<std::string, 3> solvers { "Euler", "Verlet", "RK4" };

    // Results
    std::array<std::array<SimResult, N_DT>, 3> results;

    // Indices of dt values for which E(t) is recorded (5 representative values)
    const std::array<size_t, 5> energyRecordIdx { 5, 15, 25, 35, 45 };
    auto                        shouldRecord = [&](size_t j)
    {
        for (auto idx : energyRecordIdx)
            if (idx == j)
                return true;
        return false;
    };

    // Run benchmark
    for (size_t iS = 0; iS < solvers.size(); ++iS)
    {
        std::cout << "Solver: " << solvers[iS] << "\n";
        for (size_t jDt = 0; jDt < N_DT; ++jDt)
        {
            results[iS][jDt] = simulation(solvers[iS], timesteps[jDt], maxIterations[jDt], shouldRecord(jDt));

            std::cout << "  dt=" << timesteps[jDt] << "  height_error=" << results[iS][jDt].maxHeightError
                      << "  bounces=" << results[iS][jDt].bounceCount
                      << "  max_energy_drift=" << results[iS][jDt].maxEnergyDrift
                      << "  cpu=" << results[iS][jDt].cpuMs << " ms\n";
        }
    }

    //--------------------------------------------------------------------
    // CSV 1: benchmark.csv
    // Columns: solver, dt, contact_error, max_energy_drift,
    //          final_energy_drift, cpu_ms, ops_total
    //--------------------------------------------------------------------
    {
        std::ofstream file("benchmarks/Bouncing/benchmark.csv");
        if (!file)
        {
            std::cerr << "Cannot open benchmark.csv\n";
            return 1;
        }

        file << "solver,dt,max_height_error,max_energy_drift,"
                "final_energy_drift,cpu_ms,ops_total,bounce_count\n";

        // Force evaluations per step: Euler=1, Verlet=1, RK4=4
        const std::array<int, 3> opsPerStep { 1, 1, 4 };

        for (size_t iS = 0; iS < solvers.size(); ++iS)
        {
            for (size_t jDt = 0; jDt < N_DT; ++jDt)
            {
                const SimResult& r = results[iS][jDt];

                // Total force evaluations (normalised cost metric)
                const long long opsTotal = static_cast<long long>(maxIterations[jDt]) * opsPerStep[iS];

                file << solvers[iS] << "," << timesteps[jDt] << "," << r.maxHeightError << ","
                     << r.maxEnergyDrift << "," << r.finalEnergyDrift << "," << r.cpuMs << "," << opsTotal
                     << "," << r.bounceCount << "\n";
            }
        }
    }

    //--------------------------------------------------------------------
    // CSV 2: energy_drift.csv
    // Columns: solver, dt, time, energy_drift
    // Contains E(t) samples for selected dt values
    //--------------------------------------------------------------------
    {
        std::ofstream file("benchmarks/Bouncing/energy_drift.csv");
        if (!file)
        {
            std::cerr << "Cannot open energy_drift.csv\n";
            return 1;
        }

        file << "solver,dt,time,energy_drift\n";

        for (size_t iS = 0; iS < solvers.size(); ++iS)
        {
            for (size_t jDt : energyRecordIdx)
            {
                const SimResult& r = results[iS][jDt];
                for (size_t k = 0; k < r.energyTimes.size(); ++k)
                {
                    file << solvers[iS] << "," << timesteps[jDt] << "," << r.energyTimes[k] << ","
                         << r.energyDrifts[k] << "\n";
                }
            }
        }
    }

    std::cout << "\nCSV files written to benchmarks/Bouncing/\n";
    return 0;
}