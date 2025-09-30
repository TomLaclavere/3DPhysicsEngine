#!/usr/bin/env bash
set -euo pipefail

# Defaults
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
APP_NAME=${APP_NAME:-PhysicsEngine}
BIN_DIR=${BIN_DIR:-"$BUILD_DIR/bin"}
REBUILD=0
CMAKE_ARGS=()
APP_ARGS=()

usage() {
    cat <<EOF
Usage: $0 [options] [cmake-args...] -- [app-args...]
Options:
  --build-dir DIR     Build directory (default: ${BUILD_DIR})
  --build-type TYPE   CMake build type (Debug|Release) (default: ${BUILD_TYPE})
  --app-name NAME     Executable name (default: ${APP_NAME})
  --rebuild           Force rebuild before running
  -h,--help           Show this help
Any argument starting with '-' that is not recognized is forwarded to CMake.
Use '--' to separate script/CMake args from arguments forwarded to the executable.
Examples:
  ./scripts/run_app.sh -DUSE_CLANG=ON -- --config config.yaml --seed 42
  ./scripts/run_app.sh --rebuild -DUSE_DOUBLE_PRECISION=ON
EOF
}

# Parse args; collect cmake args (any -*) until `--` then remaining are app args
while [[ $# -gt 0 ]]; do
    case "$1" in
        --) shift; APP_ARGS=("$@"); break;;
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --app-name) APP_NAME="$2"; shift 2;;
        --rebuild) REBUILD=1; shift;;
        -h|--help) usage; exit 0;;
        -*) CMAKE_ARGS+=("$1"); shift;;
        *) echo "Unknown positional arg: $1"; usage; exit 1;;
    esac
done

BIN_PATH="$BIN_DIR/$APP_NAME"

# Build if requested or if binary missing
if [[ $REBUILD -eq 1 || ! -x "$BIN_PATH" ]]; then
    echo "Building (build dir = $BUILD_DIR, type = $BUILD_TYPE) with CMake args: ${CMAKE_ARGS[*]}"
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
fi

if [[ ! -x "$BIN_PATH" ]]; then
    echo "Error: executable not found or not executable: $BIN_PATH"
    echo "Check APP_NAME or RUNTIME_OUTPUT_DIRECTORY in your CMakeLists.txt"
    exit 2
fi

# Exec the program with forwarded arguments
exec "$BIN_PATH" "${APP_ARGS[@]}"
