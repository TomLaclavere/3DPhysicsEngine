# Scripts Help Summary

## benchmark.sh

Usage: ./scripts/benchmark.sh [options]

Builds the scalar and optimised reference profiles and runs the solver x n_objects x dt
sweep for each, writing benchmarks/performance/results/benchmark.csv.

Options:
  --solvers LIST     space-separated solver names  (default: "Euler Verlet RK4")
  --n-objects LIST   space-separated object counts  (default: "1 2 5 10 20 50 100")
  --dt-min VALUE     smallest timestep  (default: 1e-5)
  --dt-max VALUE     largest timestep   (default: 1e-2)
  --dt-n N           number of log-spaced dt values  (default: 5)
  --dur VALUE        simulated duration in s  (default: 2.0)
  --n-warmup N       discarded warmup runs  (default: 1)
  --n-runs N         timed runs  (default: 5)
  --skip-build       reuse existing binaries instead of an (incremental) rebuild
  --skip-analysis    skip the Python analysis step
  -h, --help         show this help

Every run is pinned to one P-core (see benchmarks/performance/profiles.sh). With the default
sweep, expect about 40 minutes, dominated by the scalar profile at N=100 (--dt-min 1e-4
roughly divides that by 3).

Output:
  benchmarks/performance/results_scalar/benchmark.csv   scalar profile
  benchmarks/performance/results/benchmark.csv          optimised profile
  benchmarks/performance/results/plot_*.png, benchmark_report.pdf   (benchmark_analysis.py)

Next steps (not run by this script):
  scripts/profile.sh                               all three below, in one command
  benchmarks/performance/perf/profile_perf.sh      hardware counters + flamegraph
  benchmarks/performance/maqao/profile_maqao.sh    vectorisation + roofline
  benchmarks/performance/malt/profile_malt.sh      allocation profiling

---

## build.sh

Usage:
  $0 [options] [cmake-args...]

Options:
  --build-dir DIR          Build directory (default: `${BUILD_DIR}`)
  --build-type TYPE        CMake build type: Debug or Release (default: `${BUILD_TYPE}`)
  --jobs N                 Number of parallel build jobs (default: `${NUM_JOBS}`)
  --cmake-arg "ARG"        Add one CMake argument (can be repeated)
  --clean                  Clean the project before building (uses `${CLEAN_SCRIPT}` if available)
  -h, --help               Show this help message and exit

Behavior:
  Use the double dash (--) to separate build/CMake options from runtime arguments.
  Everything before "--" is passed to CMake.
  Everything after "--" is passed to the executable.

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

Usage: $0 [options]

Options:
  --build-dir DIR   Specify the build directory to remove (default: ${BUILD_DIR})
  --all             Remove additional CMake cache and generated files
  -v, --verbose     Show detailed actions
  -h, --help        Show this help message

Behaviour:
    - Safer than a raw `rm -rf`, validates target paths.
    - Defaults to "$BUILD_DIR" or "build/".
    - Prevents accidental deletion of critical system directories.

Examples:
    # show help
    ./scripts/clean.sh -h

    # remove the build directory (default: ./build)
    ./scripts/clean.sh

    # remove a custom build directory
    ./scripts/clean.sh --build-dir out

---

## coverage.sh

Usage: $0 [options] [cmake-args...]

Options:
  --build-dir DIR       Build directory (default: ${BUILD_DIR})
  --build-type TYPE     Build type (Debug|Release) (default: ${BUILD_TYPE})
  --format html|console Output format (default: console)
  --open                If html, open report in default browser
  -v, --verbose         Enable verbose output
  -h, --help            Show this help

Behavior:
  - Coverage is automatically enabled unless explicitly disabled with `-DD3DPE_ENABLE_COVERAGE=OFF`.
  - HTML reports are generated under: `build/coverage_report/coverage_report.html`.
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

## full_report.sh

Usage: ./scripts/full_report.sh

Runs scripts/benchmark.sh (~40 min), then scripts/profile.sh (~3 min), with their default
settings, and prints where the two reports are. The profiling step runs even if the sweep
failed. For other settings, run the two scripts directly.

Do not build, clean or test the project while it runs (scripts/clean.sh, tests.sh and
coverage.sh delete or rebuild build/, which removes the binaries being measured).

Output:
  benchmarks/performance/results/benchmark_report.pdf               benchmark sweep
  benchmarks/performance/profiling_report/profiling_report.pdf      profiling

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

## profile.sh

Usage: ./scripts/profile.sh [options]

Runs, in order: perf/profile_perf.sh, maqao/profile_maqao.sh, malt/profile_malt.sh, then
profiling_analysis.py, which turns whatever results exist into figures and a PDF report
(benchmarks/performance/profiling_report/). A tool that isn't installed fails that one step
(with install instructions) without stopping the others - see the summary printed at the end.

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

Usage: $0 [options] [cmake-args...] -- [app-args...]

Options:
  --build-dir DIR     Build directory (default: ${BUILD_DIR})
  --build-type TYPE   CMake build type (Debug|Release) (default: ${BUILD_TYPE})
  --app-name NAME     Executable name (default: ${APP_NAME})
  --rebuild           Force rebuild before running
  --no-build          Skip build step even if executable is missing
  -v, --verbose       Enable verbose output
  -h, --help          Show this help message

Arguments:
  You may pass additional CMake arguments before '--'.
  All arguments after '--' are passed to the application.

Behaviour:
    - Use `--` to separate build/CMake options from executable runtime arguments.
    - Example: everything before `--` goes to CMake; everything after goes to your program.
    - Executable defaults to `build/PhysicsEngine`.

Examples:
    # show help
    ./scripts/run.sh -h

    # default build and run
    ./scripts/run.sh

    # force rebuild using Clang
    ./scripts/run.sh --rebuild -D3DPE_USE_CLANG=ON

    # pass compile-time options and forward runtime arguments (after --)
    ./scripts/run.sh -D3DPE_USE_DOUBLE_PRECISION=ON -- --config config.yaml --iters 100

---

## tests.sh

Usage: $0 [options] [cmake-args...]

Options:
  --build-dir DIR      Build directory (default: ${BUILD_DIR})
  --build-type TYPE    Build type (Debug|Release) (default: ${BUILD_TYPE})
  --rebuild            Force rebuild before running tests
  --gtest-filter STR   Run only GoogleTest tests matching filter
  -v, --verbose        Enable verbose output
  -h, --help           Show this help

Behavior:
  - Unknown options starting with `-D` are forwarded to CMake.
  - Uses `./scripts/build.sh` internally for consistency.
  - Exits non-zero on failed tests.

Examples:
  ./scripts/test.sh -DUSE_DOUBLE_PRECISION=ON
  ./scripts/test.sh --rebuild --gtest-filter MySuite.*
  ./scripts/test.sh -v -- -DUSE_CLANG=ON

---

