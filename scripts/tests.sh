#!/usr/bin/env bash
set -euo pipefail

# Defaults
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
REBUILD=0
CMAKE_ARGS=()
GTEST_FILTER=${GTEST_FILTER:-""}

usage() {
    cat <<EOF
Usage: $0 [options] [cmake-args...]
Options:
  --build-dir DIR      Build directory (default: ${BUILD_DIR})
  --build-type TYPE    Build type (Debug|Release) (default: ${BUILD_TYPE})
  --rebuild            Force rebuild before running tests
  --gtest-filter STR   If running GoogleTest binaries directly, pass --gtest_filter=STR
  -h,--help            Show this help

Behavior:
  - Any option starting with '-' that isn't recognized is forwarded to CMake.
  - The script calls ./scripts/build.sh to (re)configure and build.
  - After building, it prefers to run 'ctest' (if available). If ctest is missing,
    it will try to discover and execute test binaries inside the build tree.
Examples:
  # Build with double precision and run tests
  ./scripts/run_tests.sh -DUSE_DOUBLE_PRECISION=ON

  # Force rebuild and run only tests filtered by GoogleTest name
  ./scripts/run_tests.sh --rebuild --gtest-filter MySuite.* -- -DUSE_CLANG=ON
EOF
}

# parse args
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --rebuild) REBUILD=1; shift;;
        --gtest-filter) GTEST_FILTER="$2"; shift 2;;
        -h|--help) usage; exit 0;;
        -*) CMAKE_ARGS+=("$1"); shift;;
        *) echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

echo "Run tests (build dir = $BUILD_DIR, type = $BUILD_TYPE)"
echo "CMake extra args: ${CMAKE_ARGS[*]}"

# Build if requested or binary missing
if [[ $REBUILD -eq 1 ]]; then
    echo "Requested rebuild..."
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
else
    # Try to build if build dir is absent or seems incomplete
    if [[ ! -d "$BUILD_DIR" ]] || [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        echo "Build directory missing or not configured: running build"
        ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"
    else
        # we still ensure targets are built incrementally
        cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -j "$(nproc)"
    fi
fi

# Run via ctest
if ! command -v ctest >/dev/null 2>&1; then
  echo "ctest not found—please install CTest (CMake) to run tests."
  exit 2
fi

echo "Running tests via ctest in $BUILD_DIR (type=$BUILD_TYPE)…"
export CTEST_PARALLEL_LEVEL=${CTEST_PARALLEL_LEVEL:-$(nproc)}
CTEST_ARGS=(--test-dir "$BUILD_DIR" -C "$BUILD_TYPE" --output-on-failure)
[[ -n "$GTEST_FILTER" ]] && CTEST_ARGS+=(-R "$GTEST_FILTER")
ctest "${CTEST_ARGS[@]}"
exit $?
