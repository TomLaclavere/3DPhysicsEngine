# Developer scripts — USAGE

This repository provides small, robust scripts in `scripts/` to simplify common tasks:
- `build.sh` — configure & build with CMake (accepts bare `-D` args)
- `run_app.sh` — build (if needed) and run the executable (forwards runtime args)
- `run_tests.sh` — build & run tests (uses ctest or discovered test binaries)
- `coverage.sh` — build and generate coverage (HTML or console)
- `clean.sh` — remove the build directory

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
# help
./scripts/build.sh -h

# default build
./scripts/build.sh

# pass CMake compilation args
./scripts/build.sh -DCMAKE_CXX_FLAGS="-O3"

# pass CMake args
./scripts/build.sh -D3DPE_USE_CLANG=ON -D3DPE_USE_DOUBLE_PRECISION=ON

# set build dir and type
./scripts/build.sh --build-dir out --build-type Release -D3DPE_USE_CLANG=ON
```

### Run

Build (if needed) and run main code of the simulation.
Any argument starting with '-' that is not recognized is forwarded to CMake.
Use '--' to separate script/CMake args from arguments forwarded to the executable.

```bash
# help
./scripts/run.sh -h

# default build
./scripts/run.sh

# force rebuild (using Clang instead of default GCC), then run
./scripts/run.sh --rebuild -D3DPE_USE_CLANG=ON

# pass compile-time flags (use double precision) and forward runtime args after -- (give configuration file)
./scripts/run.sh -D3DPE_USE_DOUBLE_PRECISION=ON -- --config config.yaml
```

### Test

Build (if needed) and run tests.
Any option starting with '-' that isn't recognized is forwarded to CMake.
The script calls ./scripts/build.sh to (re)configure and build.
After building, it prefers to run 'ctest' (if available). If ctest is missing, it will try to discover and execute test binaries inside the build tree.

```bash
# help
./scripts/tests.sh -h

# run tests
./scripts/tests.sh

# pass compile flags and filter gtest names
./scripts/tests.sh -D3DPE_USE_CLANG=ON --gtest-filter MySuite.*

# force rebuild and run (changing compiler)
./scripts/tests.sh --rebuild -D3DPE_USE_CLANG=ON
```

### Coverage

Build (if needed) and compute coverage of unit tests.
Coverage can be printed in current console or saved (and opened) using HTML format (default is console).

```bash
# help
./scripts/coverage.sh -h

# generate console report [default format]
./scripts/coverage.sh --format console 

# generate html report [and open in default browser]
./scripts/coverage.sh --format html [--open]

# add compile-time flags
./scripts/coverage.sh --format html -D3DPE_USE_CLANG=ON --open
```

### Clean

Remove build directory to clean repository.

```bash
# remove build directory
./scripts/clean.sh
```
