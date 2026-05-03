#include "mathematics/vector.hpp"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

struct SimResult
{
    // Timing statistics (across N_RUN timed runs)
    long long cpuUs_mean; // arithmetic mean (µs)
    long long cpuUs_min;
    long long cpuUs_max;
    decimal   cpuUs_stddev; // sample std dev (Bessel-corrected)

    // Derived metrics
    decimal timePerStep_us;
};

SimResult simulation(const std::string& solver, decimal timestep, int maxiter, int nWarmup, int nTimingRuns)
{
    Config& config = Config::get();
    config.setSolver(solver);
    config.setTimeStep(timestep);
    config.setMaxIterations(maxiter);
    config.setSimplifiedCollision(true);

    const decimal timeStep = config.getTimeStep();
    const auto    maxIter  = static_cast<size_t>(config.getMaxIterations());

    SimResult              result {};
    std::vector<long long> timings;
    timings.reserve(static_cast<size_t>(nTimingRuns));

    const int totalRuns = nWarmup + nTimingRuns;

    for (int run = 0; run < totalRuns; ++run)
    {
        const bool isWarmup  = (run < nWarmup);
        const bool isLastRun = (run == totalRuns - 1);

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

        decimal runMaxEnergyDrift       = 0_d;
        decimal runFinalEnergyDrift     = 0_d;
        decimal runMaxFlightEnergyDrift = 0_d;
        int     runBounceCount          = 0;
        decimal runMaxHeightError       = 0_d;

        size_t  counter     = 0;
        decimal previousVz  = sphere->getVelocity().getZ();
        decimal currentPeak = z0;

        Timer runTimer;

        while (counter < maxIter && world.getIsRunning())
        {
            const decimal time = static_cast<decimal>(counter) * timeStep;

            world.integrate();

            ++counter;
        }

        const long long elapsed = runTimer.elapsedMicroseconds();

        world.clearObjects();
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

    return result;
}

// Entry point

int main(int argc, char** argv)
{
    // Defaults
    const char* solver        = "Verlet";
    double      timestep      = 1e-3;
    double      duration      = 22.0;
    bool        contactForces = false;
    int         N_WARMUP      = 1;
    int         N_RUN         = 5;

    // Argument parsing
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--solver") == 0 && i + 1 < argc)
            solver = argv[++i];
        else if (std::strcmp(argv[i], "--dt") == 0 && i + 1 < argc)
            timestep = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--dur") == 0 && i + 1 < argc)
            duration = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--contact-forces") == 0)
            contactForces = true;
        else if (std::strcmp(argv[i], "--n_warmpup") == 0 && i + 1 < argc)
            N_WARMUP = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--n_runs") == 0 && i + 1 < argc)
            N_RUN = std::atoi(argv[++i]);
    }

    const int         maxIterations = static_cast<int>(duration / timestep);
    const std::string outputPath    = "benchmarks/performance/results";

    // Results
    SimResult results;

    // Run benchmark
    std::cout << "Solver: " << solver << "\n";

    results = simulation(solver, timestep, maxIterations, N_WARMUP, N_RUN);

    std::cout << "  dt=" << timestep << "  cpu_mean=" << results.cpuUs_mean << " µs"
              << "  stddev=" << results.cpuUs_stddev << " µs\n";

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
                "time_per_step_us\n";

        const SimResult& r = results;

        file << solver << "," << timestep << "," << r.cpuUs_mean << "," << r.cpuUs_min << "," << r.cpuUs_max
             << "," << r.cpuUs_stddev << "," << r.timePerStep_us << "\n";
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

        file << "solver,dt,time\n";
        file << solver << "," << timestep << "\n";
    }

    std::cout << "\nCSV files written to " << outputPath << "\n";
    return 0;
}