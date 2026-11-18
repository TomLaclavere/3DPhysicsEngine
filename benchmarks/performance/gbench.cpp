/**
 * @file gbench.cpp
 *
 * @brief The performance benchmark point of benchmark.cpp, timed by Google Benchmark instead.
 *
 * Same options, scene and timed function (scenario.hpp); only the measurement differs. One
 * iteration = one call to runSteps(world, scenario.steps()) on a freshly built scene: exactly what
 * one timed run of benchmark.cpp times. The reported time is that of the whole run (divide by the
 * "steps" counter for the time per step).
 *
 * Usage: GBench [scenario options] [--n_runs N] [--benchmark_* options]
 */

#include "scenario.hpp"

#include <benchmark/benchmark.h>

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv); // consumes the --benchmark_* options

    Scenario sc;
    int      nRuns = 5;
    for (int i = 1; i < argc; ++i)
    {
        if (parseScenarioOption(sc, i, argc, argv))
            continue;
        if (std::string(argv[i]) == "--n_runs" && i + 1 < argc)
            nRuns = std::atoi(argv[++i]);
        else
        {
            std::cerr << "Unknown argument: " << argv[i] << "\n";
            return 1;
        }
    }
    if (!isKnownBroad(sc.broad))
    {
        std::cerr << "Unknown broad phase: " << sc.broad << " (expected Brut, Grid or SweepPrune)\n";
        return 1;
    }

    configure(sc);
    const Cloud cloud = makeCloud(sc.nObjects); // generated once, as in benchmark.cpp

    const std::string name = sc.solver + "/" + sc.broad + "/" + std::to_string(sc.nObjects);
    benchmark::RegisterBenchmark(name,
                                 [&](benchmark::State& state)
                                 {
                                     ScenarioWorld run(cloud); // not timed
                                     for (auto _ : state)
                                         runSteps(run.world, sc.steps());
                                     state.counters["n_objects"] = sc.nObjects;
                                     state.counters["dt"]        = static_cast<double>(sc.dt);
                                     state.counters["steps"]     = sc.steps();
                                 })
        ->Iterations(1) // one run per repetition, as benchmark.cpp
        ->Repetitions(nRuns)
        ->UseRealTime() // wall clock, as benchmark.cpp
        ->Unit(benchmark::kMicrosecond);

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
