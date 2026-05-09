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
#include <sys/resource.h>
#include <vector>

struct SimResult
{
    // Timing statistics (across N_RUN timed runs)
    long long cpuUs_mean; // arithmetic mean (µs)
    long long cpuUs_min;
    long long cpuUs_max;
    decimal   cpuUs_stddev; // sample std dev (Bessel-corrected)
    decimal   cv_percent;   // coefficient of variation

    // Derived timing
    decimal wallTime_s;     // mean wall-clock duration of one full run (seconds)
    decimal timePerStep_us; // mean wall-clock time per integration step (µs)
    decimal stepsPerSecond; // effective throughput

    // FLOP/s (analytical estimate, not hardware-counter measured)
    long long flopsPerStep;
    decimal   gflops_per_second;

    // Memory
    long long peakRSS_kb; // peak resident set size at benchmark end (kilobytes)
};

// Analytical FLOP estimate for the benchmark scene: 1 dynamic sphere + 1 fixed plane, impulse mode.
// Euler/Verlet: 1 force eval per step. RK4: 4 force evals per step. See md for more details.
long long estimateFlopsPerStep(const std::string& solver)
{
    if (solver == "RK4")
        return 192LL;
    if (solver == "Verlet")
        return 54LL;
    return 48LL; // Euler
}

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

        // Scene setup
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

        if (!isWarmup)
            timings.push_back(elapsed);

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
    result.cv_percent   = (result.cpuUs_mean > 0) ? 100_d * result.cpuUs_stddev / mean_d : 0_d;

    // Derived timing
    result.wallTime_s     = mean_d * 1e-6_d;
    result.timePerStep_us = mean_d / static_cast<decimal>(maxiter);
    result.stepsPerSecond = (result.timePerStep_us > 0_d) ? 1'000'000.0_d / result.timePerStep_us : 0_d;

    // FLOP/s
    result.flopsPerStep       = estimateFlopsPerStep(solver);
    const decimal total_flops = static_cast<decimal>(result.flopsPerStep) * static_cast<decimal>(maxiter);
    result.gflops_per_second  = (result.wallTime_s > 0_d) ? total_flops / (result.wallTime_s * 1e9_d) : 0_d;

    // Peak memory
    struct rusage usage {};
    getrusage(RUSAGE_SELF, &usage);
    result.peakRSS_kb = usage.ru_maxrss;

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
    bool        appendMode    = false;
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
        else if (std::strcmp(argv[i], "--append") == 0)
            appendMode = true;
        else if (std::strcmp(argv[i], "--n_warmup") == 0 && i + 1 < argc)
            N_WARMUP = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--n_warmpup") == 0 && i + 1 < argc)  // legacy typo
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

    std::cout << "  dt=" << timestep << "  wall=" << results.wallTime_s << " s"
              << "  cpu_mean=" << results.cpuUs_mean << " µs"
              << "  stddev=" << results.cpuUs_stddev << " µs"
              << "  CV=" << results.cv_percent << "%"
              << "  time/step=" << results.timePerStep_us << " µs"
              << "  steps/s=" << results.stepsPerSecond << "  GFLOP/s=" << results.gflops_per_second
              << "  RSS=" << results.peakRSS_kb << " kB\n";

    // Created results directory
    std::filesystem::create_directory(outputPath);

    // CSV 1: benchmark.csv
    // --append skips the header so the shell can accumulate rows across invocations.
    {
        const auto openMode = appendMode
                                  ? (std::ios::out | std::ios::app)
                                  : (std::ios::out | std::ios::trunc);
        std::ofstream file(outputPath + "/benchmark.csv", openMode);
        if (!file)
        {
            std::cerr << "Cannot open benchmark.csv\n";
            return 1;
        }

        if (!appendMode)
            file << "solver,dt,"
                    "cpu_us_mean,cpu_us_min,cpu_us_max,cpu_us_stddev,cv_percent,"
                    "wall_time_s,time_per_step_us,steps_per_second,"
                    "flops_per_step,gflops_per_second,peak_rss_kb\n";

        const SimResult& r = results;

        file << solver << "," << timestep << "," << r.cpuUs_mean << "," << r.cpuUs_min << "," << r.cpuUs_max
             << "," << r.cpuUs_stddev << "," << r.cv_percent << "," << r.wallTime_s << "," << r.timePerStep_us
             << "," << r.stepsPerSecond << "," << r.flopsPerStep << "," << r.gflops_per_second << ","
             << r.peakRSS_kb << "\n";
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