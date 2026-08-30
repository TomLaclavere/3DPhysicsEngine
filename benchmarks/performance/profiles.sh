#!/usr/bin/env bash

# profiles.sh - shared build-profile definitions for the benchmark/profiling pipeline.
#
# Requires ROOT to already be set by the sourcing script (repo root, absolute path).

SCALAR_PROFILE_FLAGS="-O2 -fno-tree-vectorize -fno-inline -DNDEBUG"
OPTIMISED_PROFILE_FLAGS="-O3 -march=native -DNDEBUG"

# build_profile <scalar|optimised> <build_dir>
#
# Recompiles the project (Release, tests/coverage off) with the compiler flags for the
# given profile. See docs/benchmark.md for why these two flag sets are the reference
# baselines (scalar = auto-vectorisation and inlining disabled; optimised = -O3 -march=native).

build_profile() {
    local profile="$1" build_dir="$2" flags

    case "$profile" in
        scalar)    flags="$SCALAR_PROFILE_FLAGS" ;;
        optimised) flags="$OPTIMISED_PROFILE_FLAGS" ;;
        *)
            echo "build_profile: unknown profile '$profile' (expected scalar|optimised)" >&2
            return 1
            ;;
    esac

    rm -rf -- "$build_dir"

    "$ROOT/scripts/build.sh" \
        --build-dir "$build_dir" \
        --build-type Release \
        -D3DPE_BUILD_TESTS=OFF \
        -D3DPE_ENABLE_COVERAGE=OFF \
        "-DCMAKE_CXX_FLAGS_RELEASE=${flags}"
}

# build_dir_for <scalar|optimised> -> default build directory for that profile
build_dir_for() {
    echo "$ROOT/build/benchmark_$1"
}

# performance_binary <scalar|optimised> -> path to the Performance benchmark binary
# (built from benchmarks/performance/benchmark.cpp - see benchmarks/performance/README.md)
performance_binary() {
    echo "$(build_dir_for "$1")/benchmarks/Performance"
}
