#include "objects/aabb.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include "scenario.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
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

// Analytical FLOP estimate, impulse mode only.
// Gravity O(N), collision detection O(N²) with ~45 FLOPs/pair, integration O(N).
// Collisions are solved once per step whatever the solver; RK4 re-evaluates gravity 4× per step.
// Not estimated (returns 0) for the other broad phases (they don't test every pair) or with contact
// forces (the number of pair loops per step then depends on the solver).
long long estimateFlopsPerStep(const std::string& solver, const std::string& broad, int nObjects,
                               bool contactForces)
{
    if (broad != "Brut" || contactForces)
        return 0;
    const auto      n               = static_cast<long long>(nObjects);
    const long long gravity_flops   = 6LL * n * ((solver == "RK4") ? 4LL : 1LL);
    const long long collision_flops = 45LL * n * (n - 1) / 2;
    const long long integrate_flops = (solver == "Verlet") ? 18LL : 12LL;
    return gravity_flops + collision_flops + integrate_flops * n;
}

SimResult simulation(const Scenario& sc, int nWarmup, int nTimingRuns)
{
    configure(sc);
    const auto maxIter = static_cast<size_t>(sc.steps());
    const int  maxiter = sc.steps();

    SimResult              result {};
    std::vector<long long> timings;
    timings.reserve(static_cast<size_t>(nTimingRuns));

    // Generated once: every run sees the same scene (scenario.hpp)
    const Cloud cloud = makeCloud(sc.nObjects);

    const int totalRuns = nWarmup + nTimingRuns;

    for (int run = 0; run < totalRuns; ++run)
    {
        const bool isWarmup = (run < nWarmup);

        ScenarioWorld scenario(cloud);

        Timer runTimer;
        runSteps(scenario.world, maxiter);
        const long long elapsed = runTimer.elapsedMicroseconds();

        if (!isWarmup)
            timings.push_back(elapsed);
    }

    // ── Step-level profiling run ───────────────────────────────────────────
    // Separate from the timed runs: measures per-step cost distribution using
    // batch timing so that clock overhead (~50 ns/call) is negligible even at
    // fine dt (many steps). Batch size targets ~500 samples regardless of dt.
    // Timed in nanoseconds: a step can take well under 1 µs, so whole-microsecond
    // batch times divided by the batch size would round down to 0.
    {
        using Clock                      = std::chrono::steady_clock;
        const int            batchSize   = std::max(1, maxiter / 500);
        std::vector<decimal> stepTimings;
        stepTimings.reserve(static_cast<size_t>((maxiter + batchSize - 1) / batchSize));

        ScenarioWorld run(cloud);

        size_t counter = 0;
        while (counter < maxIter && run.world.getIsRunning())
        {
            const int  thisBatch  = std::min(batchSize, static_cast<int>(maxIter - counter));
            const auto batchStart = Clock::now();
            runSteps(run.world, thisBatch);
            const auto batchNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - batchStart);
            stepTimings.push_back(static_cast<decimal>(batchNs.count()) * 1e-3_d / static_cast<decimal>(thisBatch));
            counter += static_cast<size_t>(thisBatch);
        }

        const decimal ssum = std::accumulate(stepTimings.begin(), stepTimings.end(), 0_d);
        const auto    sn   = static_cast<int>(stepTimings.size());
        result.stepUs_mean = ssum / static_cast<decimal>(sn);
        result.stepUs_min  = *std::ranges::min_element(stepTimings);
        result.stepUs_max  = *std::ranges::max_element(stepTimings);

        decimal ssq = 0_d;
        for (const decimal t : stepTimings)
        {
            const decimal d = t - result.stepUs_mean;
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
    result.flopsPerStep       = estimateFlopsPerStep(sc.solver, sc.broad, sc.nObjects, sc.contactForces);
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
    Scenario    sc;
    bool        appendMode = false;
    int         N_WARMUP   = 1;
    int         N_RUN      = 5;
    std::string outputPath = "benchmarks/performance/results";

    // Argument parsing: scenario options are shared with gbench.cpp (scenario.hpp)
    for (int i = 1; i < argc; ++i)
    {
        if (parseScenarioOption(sc, i, argc, argv))
            continue;
        if (std::strcmp(argv[i], "--append") == 0)
            appendMode = true;
        else if (std::strcmp(argv[i], "--n_warmup") == 0 && i + 1 < argc)
            N_WARMUP = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--n_runs") == 0 && i + 1 < argc)
            N_RUN = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--out") == 0 && i + 1 < argc)
            outputPath = argv[++i];
    }

    if (!isKnownBroad(sc.broad))
    {
        std::cerr << "Unknown broad phase: " << sc.broad << " (expected Brut, Grid or SweepPrune)\n";
        return 1;
    }

    // Results
    SimResult results;

    // Run benchmark
    std::cout << "Solver: " << sc.solver << "  broad=" << sc.broad << "  n_objects=" << sc.nObjects
              << "  collisions=" << (sc.contactForces ? "contact forces" : "impulse") << "\n";

    results = simulation(sc, N_WARMUP, N_RUN);

    std::cout << "  dt=" << sc.dt << "  wall=" << results.wallTime_s << " s"
              << "  cpu_mean=" << results.cpuUs_mean << " µs"
              << "  stddev=" << results.cpuUs_stddev << " µs"
              << "  CV=" << results.cv_percent << "%"
              << "  step_mean=" << results.stepUs_mean << " µs"
              << "  step_cv=" << results.stepUs_cv << "%"
              << "  steps/s=" << results.stepsPerSecond << "  GFLOP/s=" << results.gflops_per_second
              << "  RSS=" << results.peakRSS_kb << " kB\n";

    std::filesystem::create_directories(outputPath);

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
            file << "solver,broad,dt,n_objects,"
                    "cpu_us_mean,cpu_us_min,cpu_us_max,cpu_us_stddev,cv_percent,"
                    "wall_time_s,time_per_step_us,steps_per_second,"
                    "flops_per_step,gflops_per_second,"
                    "step_us_mean,step_us_min,step_us_max,step_us_stddev,step_us_cv,"
                    "peak_rss_kb\n";

        const SimResult& r = results;

        file << sc.solver << "," << sc.broad << "," << sc.dt << "," << sc.nObjects << "," << r.cpuUs_mean << "," << r.cpuUs_min
             << "," << r.cpuUs_max << "," << r.cpuUs_stddev << "," << r.cv_percent << "," << r.wallTime_s
             << "," << r.timePerStep_us << "," << r.stepsPerSecond << "," << r.flopsPerStep << ","
             << r.gflops_per_second << "," << r.stepUs_mean << "," << r.stepUs_min << "," << r.stepUs_max
             << "," << r.stepUs_stddev << "," << r.stepUs_cv << "," << r.peakRSS_kb << "\n";
    }

    std::cout << "\nCSV written to " << outputPath << "/benchmark.csv\n";
    return 0;
}