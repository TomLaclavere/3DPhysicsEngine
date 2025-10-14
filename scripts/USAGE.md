# Developer scripts — USAGE

This repository provides small, robust scripts in `scripts/` to simplify common tasks:
- `build.sh` — configure & build with CMake (accepts raw `-D` arguments)
- `run.sh` — build (if needed) and run the executable (forwards runtime arguments)
- `tests.sh` — build & run tests (uses ctest or auto-discovered binaries)
- `coverage.sh` — build and generate coverage (HTML or console)
- `clean.sh` — remove the build directory

All scripts are portable (POSIX-compliant), use `set -euo pipefail`, and support argument forwarding for flexible workflows.

## Quick setup
Make scripts executable:
```bash
chmod +x scripts/*.sh
```

## Examples

Examples to show how to use each bash scripts. These are not exhaustive examples, but only the most common cases. For more details about all flags, arguments and usage, use `--help`.

### Build

Configure & build using CMake. 

```bash
# show help
./scripts/build.sh -h

# default build (Debug mode, build dir = ./build)
./scripts/build.sh

# pass compiler flags
./scripts/build.sh -DCMAKE_CXX_FLAGS="-O3"

# enable optional features
./scripts/build.sh -D3DPE_USE_CLANG=ON -D3DPE_USE_DOUBLE_PRECISION=ON

# set build directory and build type
./scripts/build.sh --build-dir out --build-type Release -D3DPE_USE_CLANG=ON
```

Notes:
- Any unrecognised argument starting with `-D` is forwarded directly to CMake.
- Build directory defaults to `build/`.
- Supported build types: `Debug`, `Release`.

### Run

Build (if needed) and run the main simulation executable.
The script will automatically build the binary if it does not exist or if `--rebuild` is passed.

```bash
# show help
./scripts/run.sh -h

# default build and run
./scripts/run.sh

# force rebuild using Clang
./scripts/run.sh --rebuild -D3DPE_USE_CLANG=ON

# pass compile-time options and forward runtime arguments (after --)
./scripts/run.sh -D3DPE_USE_DOUBLE_PRECISION=ON -- --config config.yaml --iters 100
```

Notes:
- Use `--` to separate build/CMake options from executable runtime arguments.
- Example: everything before `--` goes to CMake; everything after goes to your program.
- Executable defaults to `build/PhysicsEngine`.

### Test

Build (if needed) and run tests.
The script uses `ctest` if available, otherwise it automatically finds and executes test binaries in the build tree.

```bash
# show help
./scripts/tests.sh -h

# default build and run all tests
./scripts/tests.sh

# pass compile options and run
./scripts/tests.sh -D3DPE_USE_CLANG=ON

# rebuild with a specific compiler
./scripts/tests.sh --rebuild -D3DPE_USE_CLANG=ON

# filter specific GoogleTest suite
./scripts/tests.sh --gtest-filter MySuite.*
```

Notes:
- Unknown options starting with `-D` are forwarded to CMake.
- Uses `./scripts/build.sh` internally for consistency.
- Exits non-zero on failed tests.

### Coverage

Build (if needed) and compute coverage of unit tests.
Coverage can be printed to the console or exported as an HTML report.

```bash
# show help
./scripts/coverage.sh -h

# generate console report (default)
./scripts/coverage.sh --format console

# generate HTML report
./scripts/coverage.sh --format html

# generate HTML report and open it in your default browser
./scripts/coverage.sh --format html --open

# add compile-time flags (example: build with Clang)
./scripts/coverage.sh --format html -D3DPE_USE_CLANG=ON --open
```

Notes:
- Coverage is automatically enabled unless explicitly disabled with `-D3DPE_ENABLE_COVERAGE=OFF`.
- HTML reports are generated under : `build/coverage_report/coverage_report.html`.
- The script will try to open the report automatically on Linux (`xdg-open`) or macOS (`open`) if `--open` is provided. 

### Clean

Remove the build directory and clean the repository safely.
Uses the same default build directory as other scripts.

```bash
# show help
./scripts/clean.sh -h

# remove the build directory (default: ./build)
./scripts/clean.sh

# remove a custom build directory
./scripts/clean.sh --build-dir out
```

Notes: 
- Safer than a raw `rm -rf`, warns if the directory looks suspicious.
- Defaults to `$BUILD_DIR` or `build/`.

### Environment Variables

All scripts support the following environment variables for consistency:
- `BUILD_DIR`: Build directory. Default: `build`.
- `BUILD_TYPE`: CMake build type. Default: `Debug`.
- `APP_NAME`: Main executable name. Default: `PhysicsEngine`.

These can be overridden globally, for example

```bash
export BUILD_DIR=out
export BUILD_TYPE=Release
./scripts/run.sh
```
