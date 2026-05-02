

#include "mathematics/common.hpp"
#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct SimResult
{
    // Physics quality (from the last timing run)
    decimal maxEnergyDrift;
    decimal finalEnergyDrift;
    decimal maxFlightEnergyDrift;
    int     bounceCount;
    decimal maxHeightError;

    // Timing statistics (across N_TIMING_RUNS timed runs)
    long long cpuUs_mean; // arithmetic mean (µs)
    long long cpuUs_min;
    long long cpuUs_max;
    decimal   cpuUs_stddev; // sample std dev (Bessel-corrected)

    // Derived metrics
    decimal   timePerStep_us;
    long long estimatedFlopsTotal; // flopsPerStep * numSteps — approximate
    decimal   mflopsPerSecond;

    // Energy time series (last timing run only, when recordEnergy = true)
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

static inline decimal analyticalPeakHeight(decimal z0, decimal radius, decimal restitution, int bounceIndex)
{
    return radius + (z0 - radius) * std::pow(restitution, 2 * bounceIndex);
}

SimResult simulation(const std::string& solver, decimal timestep, int maxiter, int nWarmup, int nTimingRuns,
                     int flopsPerStep, bool recordEnergy = false)
{
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(timestep);
    config.setMaxIterations(maxiter);
    config.setSimplifiedCollision(true);

    const decimal timeStep    = config.getTimeStep();
    const auto    maxIter     = static_cast<size_t>(config.getMaxIterations());
    const size_t  recordEvery = recordEnergy ? std::max(size_t(1), maxIter / 500) : 0;

    SimResult              result {};
    std::vector<long long> timings;
    timings.reserve(static_cast<size_t>(nTimingRuns));

    const int totalRuns = nWarmup + nTimingRuns;

    for (int run = 0; run < totalRuns; ++run)
    {
        const bool isWarmup       = (run < nWarmup);
        const bool isLastRun      = (run == totalRuns - 1);
        const bool doRecordEnergy = !isWarmup && isLastRun && recordEnergy;

        // Scene setup (recreated each run so state is identical)
        PhysicsWorld world(config);
        auto         ground =
            std::make_unique<Plane>(Vector3D(0_d), Vector3D(50_d, 50_d, 0_d), Vector3D(0_d, 0_d, 1_d));
        auto sphere = std::make_unique<Sphere>(Vector3D(0_d, 0_d, 20_d), 2_d, 1_d);
        sphere->setIsFixed(false);
        sphere->setRestitutionCst(1_d);
        world.addObject(sphere.get());
        world.addObject(ground.get());
        world.start();

        const decimal z0          = sphere->getPosition().getZ();
        const decimal radius      = sphere->getRadius();
        const decimal restitution = sphere->getRestitutionCst();
        const decimal E0          = computeEnergy(*sphere);

        decimal runMaxEnergyDrift       = 0_d;
        decimal runFinalEnergyDrift     = 0_d;
        decimal runMaxFlightEnergyDrift = 0_d;
        int     runBounceCount          = 0;
        decimal runMaxHeightError       = 0_d;

        decimal       E_flight_ref     = E0;
        const decimal FLIGHT_THRESHOLD = radius * 1.05_d;

        size_t  counter     = 0;
        decimal previousVz  = sphere->getVelocity().getZ();
        decimal currentPeak = z0;

        if (doRecordEnergy)
        {
            result.energyTimes.clear();
            result.energyDrifts.clear();
        }

        Timer runTimer;

        while (counter < maxIter && world.getIsRunning())
        {
            const decimal time = static_cast<decimal>(counter) * timeStep;

            world.integrate();

            const decimal z  = sphere->getPosition().getZ();
            const decimal vz = sphere->getVelocity().getZ();

            if (vz > 0_d)
                currentPeak = std::max(currentPeak, z);

            // Detect bounce: vz flips negative → positive
            // Increment bounceCount FIRST so the index is already correct
            // when we check the apex on the next downward flip.
            if (previousVz < 0_d && vz > 0_d)
            {
                runBounceCount++;
                currentPeak = z;

                // Reset flight energy reference to current E so the next free-flight
                // phase is measured relative to the post-bounce energy level.
                const decimal E_now = computeEnergy(*sphere);
                if (E_now > 0_d)
                    E_flight_ref = E_now;
            }

            // Detect apex: vz flips positive → negative
            if (previousVz > 0_d && vz <= 0_d && runBounceCount > 0)
            {
                const decimal expected = analyticalPeakHeight(z0, radius, restitution, runBounceCount);
                const decimal err      = (expected > 0_d)
                                             ? commonMaths::absVal((currentPeak - expected) / expected)
                                             : commonMaths::absVal(currentPeak - expected);
                if (err > runMaxHeightError)
                    runMaxHeightError = err;
                currentPeak = radius;
            }

            previousVz = vz;

            const decimal E = computeEnergy(*sphere);
            const decimal drift =
                (E0 != 0_d) ? commonMaths::absVal((E - E0) / E0) : commonMaths::absVal(E - E0);

            if (drift > runMaxEnergyDrift)
                runMaxEnergyDrift = drift;

            // Isolates integrator error from physical energy loss at contact
            if (z > FLIGHT_THRESHOLD && E_flight_ref > 0_d)
            {
                const decimal flightDrift = commonMaths::absVal((E - E_flight_ref) / E_flight_ref);
                if (flightDrift > runMaxFlightEnergyDrift)
                    runMaxFlightEnergyDrift = flightDrift;
            }

            if (doRecordEnergy && (counter % recordEvery == 0))
            {
                result.energyTimes.push_back(time);
                result.energyDrifts.push_back(drift);
            }

            ++counter;
        }

        const long long elapsed = runTimer.elapsedMicroseconds();

        {
            const decimal E = computeEnergy(*sphere);
            runFinalEnergyDrift =
                (E0 != 0_d) ? commonMaths::absVal((E - E0) / E0) : commonMaths::absVal(E - E0);
        }

        world.clearObjects();

        if (!isWarmup)
        {
            timings.push_back(elapsed);
            result.maxEnergyDrift       = runMaxEnergyDrift;
            result.finalEnergyDrift     = runFinalEnergyDrift;
            result.maxFlightEnergyDrift = runMaxFlightEnergyDrift;
            result.bounceCount          = runBounceCount;
            result.maxHeightError       = runMaxHeightError;
        }
    }

    // Timing statistics
    const long long tsum = std::accumulate(timings.begin(), timings.end(), 0LL);
    result.cpuUs_mean    = tsum / static_cast<long long>(timings.size());
    result.cpuUs_min     = *std::min_element(timings.begin(), timings.end());
    result.cpuUs_max     = *std::max_element(timings.begin(), timings.end());

    const auto mean_d = static_cast<decimal>(result.cpuUs_mean);
    decimal    sq_sum = 0_d;
    for (const long long t : timings)
    {
        const decimal d = static_cast<decimal>(t) - mean_d;
        sq_sum += d * d;
    }
    const int n         = static_cast<int>(timings.size());
    result.cpuUs_stddev = (n > 1) ? std::sqrt(sq_sum / static_cast<decimal>(n - 1)) : 0_d;

    result.timePerStep_us = static_cast<decimal>(result.cpuUs_mean) / static_cast<decimal>(maxiter);

    result.estimatedFlopsTotal = static_cast<long long>(flopsPerStep) * static_cast<long long>(maxiter);
    result.mflopsPerSecond =
        (result.cpuUs_mean > 0) ? static_cast<decimal>(result.estimatedFlopsTotal) /
                                      (static_cast<decimal>(result.cpuUs_mean) * static_cast<decimal>(1e-6)) /
                                      static_cast<decimal>(1e6)
                                : 0_d;

    return result;
}

// Entry point

int main(int argc, char** argv)
{
    const std::string outputPath = "benchmarks/performance/results";
    const decimal     totalTime  = 22_d; // long enough to observe multiple bounces

    constexpr int N_WARMUP      = 1;
    constexpr int N_TIMING_RUNS = 5;

    // Estimated FLOPs per step for 1-sphere/1-plane simplified collision:
    // Euler ~20, Verlet ~35, RK4 ~100. Exact figures need a profiler (perf/Maqao).
    const std::array<int, 3> flopsPerStep { 20, 35, 100 };

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
            results[iS][jDt] = simulation(solvers[iS], timesteps[jDt], maxIterations[jDt], N_WARMUP,
                                          N_TIMING_RUNS, flopsPerStep[iS], shouldRecord(jDt));

            std::cout << "  dt=" << timesteps[jDt] << "  height_error=" << results[iS][jDt].maxHeightError
                      << "  bounces=" << results[iS][jDt].bounceCount
                      << "  max_energy_drift=" << results[iS][jDt].maxEnergyDrift
                      << "  flight_energy_drift=" << results[iS][jDt].maxFlightEnergyDrift
                      << "  cpu_mean=" << results[iS][jDt].cpuUs_mean << " µs"
                      << "  stddev=" << results[iS][jDt].cpuUs_stddev << " µs\n";
        }
    }

    // Created results directory
    std::filesystem::create_directory(outputPath);

    // CSV 1: benchmark.csv
    {
        std::ofstream file(outputPath + "/benchmark.csv");
        if (!file)
        {
            std::cerr << "Cannot open benchmark.csv\n";
            return 1;
        }

        file << "solver,dt,max_height_error,max_energy_drift,final_energy_drift,"
                "max_flight_energy_drift,bounce_count,"
                "cpu_us_mean,cpu_us_min,cpu_us_max,cpu_us_stddev,"
                "time_per_step_us,estimated_flops_total,mflops_per_second\n";

        for (size_t iS = 0; iS < solvers.size(); ++iS)
        {
            for (size_t jDt = 0; jDt < N_DT; ++jDt)
            {
                const SimResult& r = results[iS][jDt];

                file << solvers[iS] << "," << timesteps[jDt] << "," << r.maxHeightError << ","
                     << r.maxEnergyDrift << "," << r.finalEnergyDrift << "," << r.maxFlightEnergyDrift << ","
                     << r.bounceCount << "," << r.cpuUs_mean << "," << r.cpuUs_min << "," << r.cpuUs_max
                     << "," << r.cpuUs_stddev << "," << r.timePerStep_us << "," << r.estimatedFlopsTotal
                     << "," << r.mflopsPerSecond << "\n";
            }
        }
    }

    // CSV 2: energy_drift.csv
    // Columns: solver, dt, time, energy_drift

    {
        std::ofstream file(outputPath + "/energy_drift.csv");
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

    std::cout << "\nCSV files written to " << outputPath << "\n";
    return 0;
}