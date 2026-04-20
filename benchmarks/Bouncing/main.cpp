/**
 * @file main.cpp
 *
 * @brief Bouncing Ball Benchmark
 *
 * Metrics measured per simulation:
 *   - Max peak height error : max relative error on analytical peak heights
 *   - Max energy drift      : max( |E(t) - E0| / E0 ) over the simulation
 *   - Final energy drift    : |E(T) - E0| / E0
 *   - CPU time (µs)         : duration of the simulation loop
 *   - Bounce count          : number of bounces detected
 *
 * Analytical reference for peak heights:
 *   Sphere: z0=20, radius=2, v0=0, g=9.81, restitution e=0.9
 *   Contact when z_centre = radius  =>  effective drop height = z0 - radius = 18
 *   Peak after bounce n (1-indexed) : z_peak(n) = radius + (z0 - radius) * e^(2n)
 *
 * Two CSV files are produced:
 *   benchmark.csv      — one row per (solver, dt), all aggregated metrics
 *   energy_drift.csv   — E(t) time series for a subset of dt values
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
#include <string>
#include <vector>

// Result structure

struct SimResult
{
    decimal maxEnergyDrift;   // max |E(t) - E0| / E0 over the whole simulation
    decimal finalEnergyDrift; // |E(T) - E0| / E0
    decimal cpuUs;            // simulation loop duration (microseconds)

    int     bounceCount;    // number of bounces detected
    decimal maxHeightError; // max relative error on analytical peak heights

    // Energy samples for E(t) plots — filled only if recordEnergy = true
    std::vector<decimal> energyTimes;
    std::vector<decimal> energyDrifts;
};

// Total mechanical energy of the sphere
// E = Ek + Ep = 0.5 * m * v^2 + m * g * z

static inline decimal computeEnergy(const Sphere& sphere, decimal g = 9.81_d)
{
    const Vector3D& v          = sphere.getVelocity();
    const decimal   velocitySq = v.dotProduct(v);
    const decimal   z          = sphere.getPosition().getZ();
    const decimal   mass       = sphere.getMass();
    return 0.5_d * mass * velocitySq + mass * g * z;
}

// Analytical peak height after bounce n (1-indexed)
//   z_peak(n) = radius + (z0 - radius) * e^(2n)

static inline decimal analyticalPeakHeight(decimal z0, decimal radius,
                                            decimal restitution, int bounceIndex)
{
    return radius + (z0 - radius) * std::pow(restitution, 2 * bounceIndex);
}

// Single simulation run

SimResult simulation(const std::string& solver,
                     decimal            timestep,
                     int                maxiter,
                     bool               recordEnergy = false)
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
    auto ground = std::make_unique<Plane>(
        Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));
    auto sphere = std::make_unique<Sphere>(
        Vector3D(0_d, 0_d, 20_d), 2_d, 1_d); // radius=2, mass=1, v0=(0,0,0)

    sphere->setIsFixed(false);
    sphere->setRestitutionCst(0.9_d);
    world.addObject(sphere.get());
    world.addObject(ground.get());
    world.start();

    // Analytical parameters (read from the actual sphere to stay consistent)
    const decimal z0          = sphere->getPosition().getZ(); // 20
    const decimal radius      = sphere->getRadius();          // 2
    const decimal restitution = sphere->getRestitutionCst();  // 0.9

    // Initial energy reference
    const decimal E0 = computeEnergy(*sphere);

    // Result initialisation
    SimResult result;
    result.maxEnergyDrift   = 0_d;
    result.finalEnergyDrift = 0_d;
    result.cpuUs            = 0_d;
    result.bounceCount      = 0;
    result.maxHeightError   = 0_d;

    const decimal timeStep = config.getTimeStep();
    const auto  maxIter  = static_cast<size_t>(config.getMaxIterations());

    // Sub-sample energy recording to at most 500 points
    const size_t recordEvery = recordEnergy
                                   ? std::max(size_t(1), maxIter / 500)
                                   : 0;

    // Simulation loop
    Timer  simulationTimer;
    size_t counter    = 0;
    decimal previousVz = sphere->getVelocity().getZ(); // 0 at t=0
    decimal currentPeak = z0; // maximum z tracked while the sphere is rising

    while (counter < maxIter && world.getIsRunning())
    {
        const decimal time = static_cast<decimal>(counter) * timeStep;

        world.integrate();

        const decimal z  = sphere->getPosition().getZ();
        const decimal vz = sphere->getVelocity().getZ();

        // -- Track maximum z while the sphere is moving upward --
        if (vz > 0_d)
            currentPeak = std::max(currentPeak, z);

        // -- Detect bounce: vz flips from negative to positive --
        // Increment bounceCount FIRST so the index is already correct
        // when we check the apex on the next downward flip.
        if (previousVz < 0_d && vz > 0_d)
        {
            result.bounceCount++;
            currentPeak = z; // start fresh peak tracking from the bounce position
        }

        // -- Detect apex: vz flips from positive to negative --
        // currentPeak holds the best estimate of the reached peak height.
        // bounceCount is already the 1-indexed bounce number for this ascent.
        if (previousVz > 0_d && vz <= 0_d && result.bounceCount > 0)
        {
            const decimal expected = analyticalPeakHeight(z0, radius, restitution,
                                                           result.bounceCount);
            const decimal err      = (expected > 0_d)
                                         ? commonMaths::absVal((currentPeak - expected) / expected)
                                         : commonMaths::absVal(currentPeak - expected);
            if (err > result.maxHeightError)
                result.maxHeightError = err;

            // Reset peak tracker for the next ascent
            currentPeak = radius;
        }

        previousVz = vz;

        // -- Energy drift --
        const decimal E     = computeEnergy(*sphere);
        const decimal drift = (E0 != 0_d)
                                  ? commonMaths::absVal((E - E0) / E0)
                                  : commonMaths::absVal(E - E0);

        if (drift > result.maxEnergyDrift)
            result.maxEnergyDrift = drift;

        // -- Optional E(t) recording --
        if (recordEnergy && (counter % recordEvery == 0))
        {
            result.energyTimes.push_back(time);
            result.energyDrifts.push_back(drift);
        }

        ++counter;
    }

    result.cpuUs = simulationTimer.elapsedMicroseconds();

    // Final energy drift
    {
        const decimal E         = computeEnergy(*sphere);
        result.finalEnergyDrift = (E0 != 0_d)
                                      ? commonMaths::absVal((E - E0) / E0)
                                      : commonMaths::absVal(E - E0);
    }

    world.clearObjects();
    return result;
}

// Entry point

int main(int argc, char** argv)
{
    const decimal totalTime = 20_d; // long enough to observe multiple bounces

    // Timesteps: logarithmic spacing
    constexpr std::size_t N_DT = 50;
    std::array<decimal, N_DT> timesteps;
    std::array<int,     N_DT> maxIterations;

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
    auto shouldRecord = [&](size_t j) {
        for (auto idx : energyRecordIdx)
            if (idx == j) return true;
        return false;
    };

    // Run benchmark
    for (size_t iS = 0; iS < solvers.size(); ++iS)
    {
        std::cout << "Solver: " << solvers[iS] << "\n";
        for (size_t jDt = 0; jDt < N_DT; ++jDt)
        {
            results[iS][jDt] = simulation(
                solvers[iS], timesteps[jDt], maxIterations[jDt],
                shouldRecord(jDt));

            std::cout << "  dt="               << timesteps[jDt]
                      << "  height_error="     << results[iS][jDt].maxHeightError
                      << "  bounces="          << results[iS][jDt].bounceCount
                      << "  max_energy_drift=" << results[iS][jDt].maxEnergyDrift
                      << "  cpu="              << results[iS][jDt].cpuUs << " µs\n";
        }
    }

    // CSV 1: benchmark.csv
    // Columns: solver, dt, max_height_error, max_energy_drift,
    //          final_energy_drift, cpu_us, ops_total, bounce_count
    
    {
        std::ofstream file("benchmarks/Bouncing/benchmark.csv");
        if (!file)
        {
            std::cerr << "Cannot open benchmark.csv\n";
            return 1;
        }

        file << "solver,dt,max_height_error,max_energy_drift,"
                "final_energy_drift,cpu_us,ops_total,bounce_count\n";

        // Force evaluations per step: Euler=1, Verlet=1, RK4=4
        const std::array<int, 3> opsPerStep { 1, 1, 4 };

        for (size_t iS = 0; iS < solvers.size(); ++iS)
        {
            for (size_t jDt = 0; jDt < N_DT; ++jDt)
            {
                const SimResult& r = results[iS][jDt];
                const long long  opsTotal =
                    static_cast<long long>(maxIterations[jDt]) * opsPerStep[iS];

                file << solvers[iS]        << ","
                     << timesteps[jDt]     << ","
                     << r.maxHeightError   << ","
                     << r.maxEnergyDrift   << ","
                     << r.finalEnergyDrift << ","
                     << r.cpuUs           << ","
                     << opsTotal          << ","
                     << r.bounceCount     << "\n";
            }
        }
    }

    // CSV 2: energy_drift.csv
    // Columns: solver, dt, time, energy_drift

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
                    file << solvers[iS]       << ","
                         << timesteps[jDt]    << ","
                         << r.energyTimes[k]  << ","
                         << r.energyDrifts[k] << "\n";
                }
            }
        }
    }

    std::cout << "\nCSV files written to benchmarks/Bouncing/\n";
    return 0;
}