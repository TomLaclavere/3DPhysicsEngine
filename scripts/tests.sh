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

if [[ "$REBUILD" == true ]]; then
    echo "[TEST] Rebuilding project..."
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
else
    # If build directory missing or not configured, build
    if [[ ! -d "$BUILD_DIR" ]] || [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        echo "[TEST] Build directory missing or not configured—running build..."
        ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
    else
        echo "[TEST] Build directory exists—incremental build"
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
