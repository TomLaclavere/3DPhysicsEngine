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

    // Step-level timing distribution (one dedicated profiling run, batch-averaged)
    // Measures algorithm regularity: jitter from collision branches, cache warm-up, etc.
    decimal stepUs_mean;
    decimal stepUs_min;
    decimal stepUs_max;
    decimal stepUs_stddev;
    decimal stepUs_cv;

    // Memory
    long long peakRSS_kb; // peak resident set size at benchmark end (kilobytes)
};

// Analytical FLOP estimate.
// Force phases scale as: gravity O(N), collision detection O(N²) with ~45 FLOPs/pair.
// Integration scales as O(N). RK4 re-evaluates forces 4× per step.
long long estimateFlopsPerStep(const std::string& solver, int nObjects)
{
    const auto      n               = static_cast<long long>(nObjects);
    const long long gravity_flops   = 6LL * n;
    const long long collision_flops = 45LL * n * (n - 1) / 2;
    const long long force_flops     = gravity_flops + collision_flops;
    const long long integrate_flops = (solver == "Verlet") ? 18LL : 12LL;
    if (solver == "RK4")
        return force_flops * 4LL + integrate_flops * n;
    return force_flops + integrate_flops * n;
}

SimResult simulation(const std::string& solver, decimal timestep, int maxiter, int nObjects, int nWarmup,
                     int nTimingRuns)
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

    // Grid layout for N spheres.
    // Spheres are arranged in a ceil(sqrt(N)) × ceil(N/cols) grid, spaced 3 radii apart
    // so they do not overlap at t=0 and have room to fall independently.
    const decimal sphRadius = 2_d;
    const decimal spacing   = sphRadius * 3_d;
    const int     ncols = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(nObjects)))));
    const int     nrows = (nObjects + ncols - 1) / ncols;
    const decimal halfSize = std::max(50_d, static_cast<decimal>(ncols + 1) * spacing);

    // Lambda that builds a fresh scene into `world` and returns the sphere handles.
    auto buildScene = [&](PhysicsWorld& world)
    {
        auto ground = std::make_unique<Plane>(Vector3D(0_d), Vector3D(halfSize, halfSize, 0_d),
                                              Vector3D(0_d, 0_d, 1_d));

        std::vector<std::unique_ptr<Sphere>> spheres;
        spheres.reserve(static_cast<size_t>(nObjects));
        for (int obj = 0; obj < nObjects; ++obj)
        {
            const int     col = obj % ncols;
            const int     row = obj / ncols;
            const decimal x   = (static_cast<decimal>(col) - static_cast<decimal>(ncols - 1) / 2_d) * spacing;
            const decimal y   = (static_cast<decimal>(row) - static_cast<decimal>(nrows - 1) / 2_d) * spacing;
            auto          sph = std::make_unique<Sphere>(Vector3D(x, y, 20_d), sphRadius, 1_d);
            sph->setIsFixed(false);
            sph->setRestitutionCst(1_d);
            // PhysicsWorld::addObject() takes ownership (deletes on clearObjects()/dtor),
            // so release() rather than get() - passing get() here left both the world and
            // this unique_ptr owning the same pointer, double-freeing it once the run ends.
            world.addObject(sph.release());
            spheres.push_back(std::move(sph));
        }
        world.addObject(ground.release());
        world.start();
        // ground/spheres are now empty (released to world); returned only so the caller's
        // structured binding keeps compiling unchanged.
        return std::make_pair(std::move(ground), std::move(spheres));
    };

    const int totalRuns = nWarmup + nTimingRuns;

    for (int run = 0; run < totalRuns; ++run)
    {
        const bool isWarmup = (run < nWarmup);

        PhysicsWorld world(config);
        auto [ground, spheres] = buildScene(world);

        size_t counter = 0;
        Timer  runTimer;

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

    // ── Step-level profiling run ───────────────────────────────────────────
    // Separate from the timed runs: measures per-step cost distribution using
    // batch timing so that clock overhead (~50 ns/call) is negligible even at
    // fine dt (many steps). Batch size targets ~500 samples regardless of dt.
    {
        const int              batchSize = std::max(1, maxiter / 500);
        std::vector<long long> stepTimings;
        stepTimings.reserve(static_cast<size_t>((maxiter + batchSize - 1) / batchSize));

        PhysicsWorld world(config);
        auto [ground, spheres] = buildScene(world);

        Timer  batchTimer;
        size_t counter = 0;
        while (counter < maxIter && world.getIsRunning())
        {
            const int thisBatch = std::min(batchSize, static_cast<int>(maxIter - counter));
            batchTimer.reset();
            for (int b = 0; b < thisBatch; ++b, ++counter)
                world.integrate();
            const long long batchUs = batchTimer.elapsedMicroseconds();
            stepTimings.push_back(batchUs / static_cast<long long>(thisBatch));
        }

        world.clearObjects();

        const long long ssum = std::accumulate(stepTimings.begin(), stepTimings.end(), 0LL);
        const auto      sn   = static_cast<int>(stepTimings.size());
        result.stepUs_mean   = static_cast<decimal>(ssum) / static_cast<decimal>(sn);
        result.stepUs_min    = static_cast<decimal>(*std::ranges::min_element(stepTimings));
        result.stepUs_max    = static_cast<decimal>(*std::ranges::max_element(stepTimings));

        decimal ssq = 0_d;
        for (const long long t : stepTimings)
        {
            const decimal d = static_cast<decimal>(t) - result.stepUs_mean;
            ssq += d * d;
        }
        result.stepUs_stddev = (sn > 1) ? std::sqrt(ssq / static_cast<decimal>(sn - 1)) : 0_d;
        result.stepUs_cv =
            (result.stepUs_mean > 0_d) ? 100_d * result.stepUs_stddev / result.stepUs_mean : 0_d;
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
    result.flopsPerStep       = estimateFlopsPerStep(solver, nObjects);
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
    int         nObjects      = 1;

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
        else if (std::strcmp(argv[i], "--n_runs") == 0 && i + 1 < argc)
            N_RUN = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--n_objects") == 0 && i + 1 < argc)
            nObjects = std::atoi(argv[++i]);
    }

    const int         maxIterations = static_cast<int>(duration / timestep);
    const std::string outputPath    = "benchmarks/performance/results";

    // Results
    SimResult results;

    // Run benchmark
    std::cout << "Solver: " << solver << "  n_objects=" << nObjects << "\n";

    results = simulation(solver, timestep, maxIterations, nObjects, N_WARMUP, N_RUN);

    std::cout << "  dt=" << timestep << "  wall=" << results.wallTime_s << " s"
              << "  cpu_mean=" << results.cpuUs_mean << " µs"
              << "  stddev=" << results.cpuUs_stddev << " µs"
              << "  CV=" << results.cv_percent << "%"
              << "  step_mean=" << results.stepUs_mean << " µs"
              << "  step_cv=" << results.stepUs_cv << "%"
              << "  steps/s=" << results.stepsPerSecond << "  GFLOP/s=" << results.gflops_per_second
              << "  RSS=" << results.peakRSS_kb << " kB\n";

    // Created results directory
    std::filesystem::create_directory(outputPath);

    // CSV 1: benchmark.csv
    // --append skips the header so the shell can accumulate rows across invocations.
    {
        const auto openMode =
            appendMode ? (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc);
        std::ofstream file(outputPath + "/benchmark.csv", openMode);
        if (!file)
        {
            std::cerr << "Cannot open benchmark.csv\n";
            return 1;
        }

        if (!appendMode)
            file << "solver,dt,n_objects,"
                    "cpu_us_mean,cpu_us_min,cpu_us_max,cpu_us_stddev,cv_percent,"
                    "wall_time_s,time_per_step_us,steps_per_second,"
                    "flops_per_step,gflops_per_second,"
                    "step_us_mean,step_us_min,step_us_max,step_us_stddev,step_us_cv,"
                    "peak_rss_kb\n";

        const SimResult& r = results;

        file << solver << "," << timestep << "," << nObjects << "," << r.cpuUs_mean << "," << r.cpuUs_min
             << "," << r.cpuUs_max << "," << r.cpuUs_stddev << "," << r.cv_percent << "," << r.wallTime_s
             << "," << r.timePerStep_us << "," << r.stepsPerSecond << "," << r.flopsPerStep << ","
             << r.gflops_per_second << "," << r.stepUs_mean << "," << r.stepUs_min << "," << r.stepUs_max
             << "," << r.stepUs_stddev << "," << r.stepUs_cv << "," << r.peakRSS_kb << "\n";
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