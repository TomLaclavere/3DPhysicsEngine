#!/usr/bin/env bash

# profiles.sh - shared definitions for the benchmark/profiling pipeline: build profiles,
# binary paths, and the CPU setup that keeps measurements reproducible.
#
# Requires ROOT to already be set by the sourcing script (repo root, absolute path).

SCALAR_PROFILE_FLAGS="-O2 -fno-tree-vectorize -fno-inline -DNDEBUG"
OPTIMISED_PROFILE_FLAGS="-O3 -march=native -DNDEBUG"

# build_profile <scalar|optimised> <build_dir>
#
# Configures and builds the project (Release, tests/coverage/examples off) with the compiler
# flags for the given profile. The build is incremental: CMake picks up both source changes
# and flag changes, so the binary always matches the current code and profile.sh flags. See
# docs/benchmark.md for why these two flag sets are the reference baselines.
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

    (cd "$ROOT" && "$ROOT/scripts/build.sh" \
        --build-dir "$build_dir" \
        --build-type Release \
        -D3DPE_BUILD_TESTS=OFF \
        -D3DPE_BUILD_EXAMPLES=OFF \
        -D3DPE_ENABLE_COVERAGE=OFF \
        "-DCMAKE_CXX_FLAGS_RELEASE=${flags}") || return 1

    # Guard: a CMake cache reset silently drops these settings, and every number measured
    # afterwards would come from an unoptimised build. Refuse to continue if that happened.
    local cache="$build_dir/CMakeCache.txt"
    if ! grep -qx "CMAKE_BUILD_TYPE:STRING=Release" "$cache" \
        || ! grep -qxF "CMAKE_CXX_FLAGS_RELEASE:STRING=${flags}" "$cache"; then
        echo "build_profile: $build_dir is not configured as Release with '${flags}'." >&2
        echo "               Delete that directory and run again." >&2
        return 1
    fi
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

# bench_cpu -> logical CPU id every measured run is pinned to (with taskset)
#
# Pinning stops the scheduler from migrating the process mid-run. On a hybrid Intel CPU it
# also guarantees a P-core: an E-core runs this code ~40% slower, so a run that lands on one
# (or migrates to one) silently corrupts the timing. The second physical P-core is used rather
# than CPU 0, which handles more interrupts. Override with the BENCH_CPU environment variable.
bench_cpu() {
    if [[ -n "${BENCH_CPU:-}" ]]; then
        echo "$BENCH_CPU"
        return
    fi
    local first=0 ncpu
    if [[ -r /sys/devices/cpu_core/cpus ]]; then
        first="$(cut -d- -f1 < /sys/devices/cpu_core/cpus | cut -d, -f1)"
    fi
    ncpu="$(nproc)"
    if (( first + 2 < ncpu )); then
        echo $((first + 2))
    else
        echo "$first"
    fi
}

# check_cpu_setup - warns (never changes anything) when the CPU frequency policy is likely to
# add noise to the measurements.
check_cpu_setup() {
    local gov_file=/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    if [[ -r "$gov_file" ]]; then
        local gov
        gov="$(cat "$gov_file")"
        if [[ "$gov" != "performance" ]]; then
            echo "[cpu] Warning: CPU governor is '${gov}', not 'performance' - the clock speed can"
            echo "      vary between runs. For the most stable numbers, before benchmarking run:"
            echo "        sudo cpupower frequency-set -g performance"
            echo "      and afterwards restore it with:"
            echo "        sudo cpupower frequency-set -g ${gov}"
        fi
    fi
    if ! command -v taskset >/dev/null 2>&1; then
        echo "Error: taskset not found (package util-linux) - needed to pin runs to one core." >&2
        return 1
    fi
    echo "[cpu] Measured runs are pinned to CPU $(bench_cpu)"
}
