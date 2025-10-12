#!/usr/bin/env bash
set -euo pipefail

# =====================================================================
# Configuration (defaults)
# =====================================================================
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
APP_NAME=${APP_NAME:-PhysicsEngine}
BIN_DIR=${BIN_DIR:-"$BUILD_DIR"}
REBUILD=false
NO_BUILD=false
VERBOSE=false
CMAKE_ARGS=()
APP_ARGS=()

# =====================================================================
# Help message
# =====================================================================
usage() {
    cat <<'EOF'
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
EOF
}

# =====================================================================
# Parse arguments
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --) shift; APP_ARGS=("$@"); break;;
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --app-name) APP_NAME="$2"; shift 2;;
        --rebuild) REBUILD=true; shift;;
        --no-build) NO_BUILD=true; shift;;
        -v|--verbose) VERBOSE=true; shift;;
        -h|--help) usage; exit 0;;
        -*) CMAKE_ARGS+=("$1"); shift;;
        *) echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

# =====================================================================
# Compute paths
# =====================================================================
BIN_PATH="${BIN_DIR}/${APP_NAME}"

# =====================================================================
# Build phase
# =====================================================================
if [[ "$REBUILD" == true ]]; then
    echo "[RUN] Rebuilding project..."
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
elif [[ ! -x "$BIN_PATH" && "$NO_BUILD" == false ]]; then
    echo "[RUN] Binary not found: $BIN_PATH"
    echo "[RUN] Triggering build..."
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
elif [[ "$NO_BUILD" == true && ! -x "$BIN_PATH" ]]; then
    echo "[ERROR] Binary not found: $BIN_PATH"
    echo "Use --rebuild or remove --no-build."
    exit 1
fi

# =====================================================================
# Validation
# =====================================================================
if [[ ! -x "$BIN_PATH" ]]; then
    echo "[ERROR] Executable not found or not executable: $BIN_PATH"
    echo "Check APP_NAME or RUNTIME_OUTPUT_DIRECTORY in your CMakeLists.txt."
    exit 2
fi

# =====================================================================
# Execution phase
# =====================================================================
[[ "$VERBOSE" == true ]] && echo "[RUN] Launching: $BIN_PATH ${APP_ARGS[*]}"

exec "$BIN_PATH" "${APP_ARGS[@]}"
