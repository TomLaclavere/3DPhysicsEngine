#!/usr/bin/env bash
set -euo pipefail

# =====================================================================
# Defaults
# =====================================================================
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
REBUILD=false
GTEST_FILTER=${GTEST_FILTER:-""}
CMAKE_ARGS=()
VERBOSE=false

# =====================================================================
# Help message
# =====================================================================
usage() {
    cat <<'EOF'
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
EOF
}

# =====================================================================
# Parse arguments
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --rebuild) REBUILD=true; shift;;
        --gtest-filter) GTEST_FILTER="$2"; shift 2;;
        -v|--verbose) VERBOSE=true; shift;;
        -h|--help) usage; exit 0;;
        -*) CMAKE_ARGS+=("$1"); shift;;
        *) echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

# =====================================================================
# Build phase
# =====================================================================
[[ "$VERBOSE" == true ]] && echo "[TEST] Build dir = $BUILD_DIR, type = $BUILD_TYPE"
[[ "$VERBOSE" == true ]] && echo "[TEST] Extra CMake args: ${CMAKE_ARGS[*]}"

if [[ "$REBUILD" == true || ${#CMAKE_ARGS[@]} -gt 0 ]]; then
    echo "[TEST] Rebuilding project..."
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
else
    # If build directory missing or not configured, build
    if [[ ! -d "$BUILD_DIR" ]] || [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        echo "[TEST] Build directory missing or not configured-running build..."
        ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
    else
        echo "[TEST] Build directory exists-incremental build"
        cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -j "$(nproc)"
    fi
fi

# =====================================================================
# Run tests
# =====================================================================
if ! command -v ctest >/dev/null 2>&1; then
    echo "[ERROR] ctest not found. Please install CTest (comes with CMake)."
    exit 2
fi

echo "[TEST] Running tests via ctest in $BUILD_DIR (type=$BUILD_TYPE)..."
export CTEST_PARALLEL_LEVEL=${CTEST_PARALLEL_LEVEL:-$(nproc)}

CTEST_ARGS=(--test-dir "$BUILD_DIR" -C "$BUILD_TYPE" --output-on-failure)
[[ -n "$GTEST_FILTER" ]] && CTEST_ARGS+=(-R "$GTEST_FILTER")

[[ "$VERBOSE" == true ]] && echo "[TEST] ctest args: ${CTEST_ARGS[*]}"

ctest "${CTEST_ARGS[@]}"
