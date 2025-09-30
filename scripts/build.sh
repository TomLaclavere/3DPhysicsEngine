#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
NUM_JOBS=${NUM_JOBS:-$(nproc)}
CMAKE_EXTRA_ARGS=()

usage() {
    cat <<EOF
Usage: $0 [options] [cmake-args...]
Options:
  --build-dir DIR         Build directory (default: ${BUILD_DIR})
  --build-type TYPE       CMake build type (Debug, Release) (default: ${BUILD_TYPE})
  --jobs N                Parallel jobs for build (default: ${NUM_JOBS})
  --cmake-arg "ARG"       Add one CMake argument (can repeat)
  -h, --help              Show this help
You may also pass plain CMake-style options directly, e.g.:
  $0 -DUSE_CLANG=ON -DCMAKE_CXX_FLAGS="-O3 -march=native"
Or generator flags:
  $0 -G "Ninja"
EOF
}

# parse args
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --cmake-arg) CMAKE_EXTRA_ARGS+=("$2"); shift 2;;
        --jobs) NUM_JOBS="$2"; shift 2;;
        -h|--help) usage; exit 0;;
        --) shift;
             while [[ $# -gt 0 ]]; do
                 CMAKE_EXTRA_ARGS+=("$1"); shift
             done
             ;;
        -*)
             CMAKE_EXTRA_ARGS+=("$1"); shift;;
        *) echo "Unknown positional argument: $1"; usage; exit 1;;
    esac
done

# configure & build
mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${CMAKE_EXTRA_ARGS[@]}"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -j "$NUM_JOBS"
