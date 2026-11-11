# Scripts Help Summary

## benchmark_and_profile.sh

Usage: ./scripts/benchmark_and_profile.sh

Runs scripts/benchmark.sh (~1 h), then scripts/profile.sh (~3 min), with their default
settings, and prints where the two reports are. The profiling step runs even if the sweep
failed. For other settings, run the two scripts directly.

Do not build, clean or test the project while it runs (scripts/clean.sh, tests.sh and
coverage.sh delete or rebuild build/, which removes the binaries being measured).

Output:
  benchmarks/performance/results/benchmark_report.pdf               benchmark sweep
  benchmarks/performance/profiling_report/profiling_report.pdf      profiling

---

## benchmark.sh

Usage: ./scripts/benchmark.sh [options]

Builds the scalar and optimised reference profiles and, for each, runs two sweeps,
writing benchmarks/performance/results/benchmark.csv:
  - n_objects sweep: every broad phase, one solver, fixed dt,
  - dt sweep: every solver, one broad phase, fixed n_objects.

Options:
  --broads LIST      broad phases of the n_objects sweep  (default: "Brut Grid SweepPrune")
  --n-solver NAME    solver of the n_objects sweep  (default: Verlet)
  --n-objects LIST   object counts of the n_objects sweep  (default: "1 2 5 ... 5000 10000")
  --brut-n-max N     largest object count run with Brut  (default: 2000)
  --dt VALUE         timestep of the n_objects sweep  (default: 1e-3)
  --solvers LIST     solvers of the dt sweep  (default: "Euler Verlet RK4")
  --dt-broad NAME    broad phase of the dt sweep  (default: Brut)
  --n-fixed N        object count of the dt sweep  (default: 100)
  --dt-min VALUE     smallest timestep of the dt sweep  (default: 1e-5)
  --dt-max VALUE     largest timestep of the dt sweep   (default: 1e-2)
  --dt-n N           number of log-spaced dt values  (default: 5)
  --dur VALUE        simulated duration in s  (default: 1.0)
  --n-warmup N       discarded warmup runs  (default: 1)
  --n-runs N         timed runs  (default: 5)
  --skip-build       reuse existing binaries instead of an (incremental) rebuild
  --skip-analysis    skip the Python analysis step
  -h, --help         show this help

Every run is pinned to one P-core, and the CPU governor is set to 'performance' for the whole
run (sudo password asked once, previous governor restored at exit; see
benchmarks/performance/profiles.sh). With the default
sweeps, expect about 1 hour (estimate), dominated by N=5000-10000 and by the dt sweep at
dt=1e-5 (drop them with --n-objects or --dt-min 1e-4).

Output:
  benchmarks/performance/results_scalar/benchmark.csv   scalar profile
  benchmarks/performance/results/benchmark.csv          optimised profile
  benchmarks/performance/results/plot_*.png, benchmark_report.pdf   (benchmark_analysis.py)

Next steps (not run by this script):
  scripts/profile.sh                               all three below, in one command
  benchmarks/performance/perf/profile_perf.sh      hardware counters + flamegraph
  benchmarks/performance/maqao/profile_maqao.sh    vectorisation per loop
  benchmarks/performance/malt/profile_malt.sh      allocation profiling

---

## build.sh

Usage:
  ./scripts/build.sh [options] [cmake-args...]

Configures and builds the project with CMake (Ninja if installed). Run from the repository root.

Options:
  --build-dir DIR          Build directory (default: build, or $BUILD_DIR)
  --build-type TYPE        CMake build type: Debug or Release (default: Debug, or $BUILD_TYPE)
  --jobs N                 Number of parallel build jobs (default: all cores, or $NUM_JOBS)
  --cmake-arg "ARG"        Add one CMake argument (can be repeated)
  --clean                  Delete the build directory first (scripts/clean.sh)
  -h, --help               Show this help message and exit

Behavior:
  Any other argument starting with "-" (e.g. -D3DPE_USE_CLANG=ON) is passed to CMake, and so
  is everything after "--".

Examples:
  Show help
    ./scripts/build.sh -h

  Default build (Debug mode, build directory = ./build)
    ./scripts/build.sh

  Pass compiler flags
    ./scripts/build.sh -DCMAKE_CXX_FLAGS="-O3"

  Enable optional features
    ./scripts/build.sh -D3DPE_USE_CLANG=ON -D3DPE_USE_DOUBLE_PRECISION=ON

  Specify build directory and build type
    ./scripts/build.sh --build-dir out --build-type Release -D3DPE_USE_CLANG=ON

---

## clean.sh

Usage: ./scripts/clean.sh [options]

Removes the build directory. Run from the repository root.

Options:
  --build-dir DIR   Build directory to remove (default: build, or $BUILD_DIR)
  --all             Also remove CMake files generated in the current directory by an in-source
                    configure (CMakeCache.txt, CMakeFiles/, Makefile, compile_commands.json...)
  -v, --verbose     Show detailed actions
  -h, --help        Show this help message

Behaviour:
    - Refuses to remove /, /home, /root or $HOME, and asks for confirmation outside the project.

Examples:
    # show help
    ./scripts/clean.sh -h

    # remove the build directory (default: ./build)
    ./scripts/clean.sh

    # remove a custom build directory
    ./scripts/clean.sh --build-dir out

---

## coverage.sh

Usage: ./scripts/coverage.sh [options] [cmake-args...]

Builds the project with coverage, runs the tests and prints or exports the coverage report.
Run from the repository root.

Options:
  --build-dir DIR       Build directory (default: build, or $BUILD_DIR)
  --build-type TYPE     Build type (Debug|Release) (default: Debug, or $BUILD_TYPE)
  --format html|console Output format (default: console)
  --open                If html, open report in default browser
  -v, --verbose         Enable verbose output
  -h, --help            Show this help

Behavior:
  - Coverage is automatically enabled unless explicitly disabled with `-D3DPE_ENABLE_COVERAGE=OFF`.
  - The HTML report is written to <build-dir>/coverage_report/coverage_report.html.
  - The script will try to open the report automatically on Linux (`xdg-open`) or macOS (`open`) if `--open` is provided.

Examples:
  show help
  ./scripts/coverage.sh -h

  generate console report (default)
  ./scripts/coverage.sh --format console

  generate HTML report
  ./scripts/coverage.sh --format html

  generate HTML report and open it in your default browser
  ./scripts/coverage.sh --format html --open

  add compile-time flags (example: build with Clang)
  ./scripts/coverage.sh --format html -D3DPE_USE_CLANG=ON --open

---

## generate_usage_md.sh

        awk "/cat <<'?EOF'?/{flag=1;next}/^EOF/{flag=0}flag" "$script" >> "$OUTPUT"

        echo "" >> "$OUTPUT"
        echo "---" >> "$OUTPUT"
        echo "" >> "$OUTPUT"
    fi
done

echo "Generated $OUTPUT"

---

## physics_benchmark.sh

Usage: ./scripts/physics_benchmark.sh [options]

Runs the physics validation benchmarks (benchmarks/physics/): each scenario writes its CSV to
benchmarks/physics/<scenario>/results/, then its benchmark_analysis.ipynb is executed, which
saves the figures and benchmark_report.pdf in the same directory. Run from the repository root.

Options:
  --scenarios LIST   space-separated scenarios
                     (default: "Free_Fall Bouncing Bouncing_Conservative Contact_Forces")
  --skip-build       reuse the existing binaries (build/benchmark_physics)
  --skip-analysis    run the benchmarks only, not the notebooks
  -h, --help         show this help

The notebooks are executed with `jupyter execute` (package nbclient) and are not modified. They
need numpy, pandas, matplotlib and scipy.

---

## profile.sh

Usage: ./scripts/profile.sh [options]

Runs, in order: perf/profile_perf.sh, maqao/profile_maqao.sh, malt/profile_malt.sh, then
profiling_analysis.py, which turns whatever results exist into figures and a PDF report
(benchmarks/performance/profiling_report/). A tool that isn't installed fails that one step
(with install instructions) without stopping the others - see the summary printed at the end.
The CPU governor is set to 'performance' for the whole run (sudo password asked once, previous
governor restored at exit).

Options:
  --profile NAME   scalar | optimised | both  - forwarded to perf/MAQAO (default: both)
  --skip-perf      don't run the perf step
  --skip-maqao     don't run the MAQAO step
  --skip-malt      don't run the MALT step
  --skip-plots     don't generate the figures / PDF report
  -h, --help       show this help

MALT needs the `malt` command installed (see benchmarks/performance/malt/malt.md) - if it
isn't found, that step fails with instructions, same as a missing perf/maqao.

Each script also takes its own --solver/--dt/--dur/--n-objects etc. - run it directly
(e.g. ./benchmarks/performance/perf/profile_perf.sh --help) for finer control than this
orchestrator offers.

---

## run.sh

Usage: ./scripts/run.sh [options] [cmake-args...] -- [app-args...]

Builds the project if the executable is missing, then runs it. Run from the repository root
(PhysicsEngine reads src/config.yaml from there).

Options:
  --build-dir DIR     Build directory (default: build, or $BUILD_DIR)
  --build-type TYPE   CMake build type (Debug|Release) (default: Debug, or $BUILD_TYPE)
  --app-name NAME     Executable in the build directory (default: PhysicsEngine, or $APP_NAME)
  --rebuild           Force rebuild before running
  --no-build          Skip build step even if executable is missing
  -v, --verbose       Enable verbose output
  -h, --help          Show this help message

Arguments:
  Arguments starting with "-" before "--" are passed to CMake (only used when building).
  All arguments after "--" are passed to the application.

Examples:
    # show help
    ./scripts/run.sh -h

    # default build and run
    ./scripts/run.sh

    # force rebuild using Clang
    ./scripts/run.sh --rebuild -D3DPE_USE_CLANG=ON

    # pass compile-time options and forward runtime arguments (after --)
    ./scripts/run.sh -D3DPE_USE_DOUBLE_PRECISION=OFF -- --solver RK4 --iters 100

---

## tests.sh

Usage: ./scripts/tests.sh [options] [cmake-args...]

Builds the project (incrementally), then runs the tests with ctest. Run from the repository root.

Options:
  --build-dir DIR      Build directory (default: build, or $BUILD_DIR)
  --build-type TYPE    Build type (Debug|Release) (default: Debug, or $BUILD_TYPE)
  --rebuild            Reconfigure and rebuild before running the tests
  --gtest-filter REGEX Run only the tests whose name (Suite.Test) matches REGEX (ctest -R)
  -v, --verbose        Enable verbose output
  -h, --help           Show this help

Behavior:
  - Other arguments starting with "-" (e.g. -D3DPE_USE_CLANG=ON) are passed to CMake; they
    trigger a reconfiguration through scripts/build.sh.
  - Exits non-zero on failed tests.

Examples:
  ./scripts/tests.sh -D3DPE_USE_DOUBLE_PRECISION=OFF
  ./scripts/tests.sh --rebuild --gtest-filter MySuite.*
  ./scripts/tests.sh -v -D3DPE_USE_CLANG=ON

---

