#!/usr/bin/env bash
set -euo pipefail

# =====================================================================
# Default configuration
# =====================================================================
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
NUM_JOBS=${NUM_JOBS:-$(nproc)}
CMAKE_EXTRA_ARGS=()
CLEAN_FIRST=false

CLEAN_SCRIPT="./scripts/clean.sh"

# =====================================================================
# Help message
# =====================================================================
usage() {
    cat <<'EOF'
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
EOF
}

# =====================================================================
# Pre-checks
# =====================================================================
command -v cmake >/dev/null 2>&1 || {
    echo "Error: cmake not found in PATH." >&2
    exit 1
}

# =====================================================================
# Parse arguments
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --cmake-arg) CMAKE_EXTRA_ARGS+=("$2"); shift 2;;
        --jobs) NUM_JOBS="$2"; shift 2;;
        --clean) CLEAN_FIRST=true; shift;;
        -h|--help) usage; exit 0;;
        --)
            shift
            while [[ $# -gt 0 ]]; do
                CMAKE_EXTRA_ARGS+=("$1"); shift
            done
            ;;
        -*)
            CMAKE_EXTRA_ARGS+=("$1"); shift;;
        *)
            echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

# =====================================================================
# Clean build directory (if requested)
# =====================================================================
if [[ "$CLEAN_FIRST" == true ]]; then
    echo "[CLEAN] Cleaning project..."
    if [[ -x "$CLEAN_SCRIPT" ]]; then
        "$CLEAN_SCRIPT" --build-dir "$BUILD_DIR"
    else
        echo "Warning: Clean script not found or not executable: $CLEAN_SCRIPT"
        echo "Falling back to: rm -rf $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
fi

# =====================================================================
# Select build generator automatically
# =====================================================================
if [[ ! " ${CMAKE_EXTRA_ARGS[*]} " =~ "-G" ]]; then
    if command -v ninja >/dev/null 2>&1; then
        echo "[INFO] Using Ninja generator"
        CMAKE_EXTRA_ARGS+=("-G" "Ninja")
    fi
fi

# =====================================================================
# Summary
# =====================================================================
echo "[INFO] Build directory : $BUILD_DIR"
echo "[INFO] Build type      : $BUILD_TYPE"
echo "[INFO] Parallel jobs   : $NUM_JOBS"
echo "[INFO] CMake args      : ${CMAKE_EXTRA_ARGS[*]:-(none)}"

# =====================================================================
# Configure and build
# =====================================================================
mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${CMAKE_EXTRA_ARGS[@]}"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -j"$NUM_JOBS"

echo "[DONE] Build completed successfully in '$BUILD_DIR' ($BUILD_TYPE)"
