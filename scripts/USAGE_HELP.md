# Scripts Help Summary

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
    - Safer than a raw `rm -rf`, warns if the directory looks suspicious.
    - Defaults to `$BUILD_DIR` or `build/`.

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
  - Coverage is automatically enabled unless explicitly disabled with `-D3DPE_ENABLE_COVERAGE=OFF`.
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

## generate_usage_md.sh

        awk "/cat <<'?EOF'?/{flag=1;next}/^EOF/{flag=0}flag" "$script" >> "$OUTPUT"

        echo "" >> "$OUTPUT"
        echo "---" >> "$OUTPUT"
        echo "" >> "$OUTPUT"
    fi
done

echo "Generated $OUTPUT"

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

