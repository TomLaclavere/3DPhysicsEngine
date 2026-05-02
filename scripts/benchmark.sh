#!/usr/bin/env bash
set -euo pipefail

# Benchmark pipeline following docs/benchmark.md.
#
# Two baseline modes:
#   scalar    — -O2 -fno-tree-vectorize -fno-inline  (scalar reference, no SIMD)
#   optimised — -O3 -march=native                    (full compiler optimisations, single-core)
#
# Steps per mode:
#   1. Recompile (Release, tests/coverage disabled, with the appropriate flags)
#   2. Run benchmark executable(s) → CSV written under benchmarks/performance
#   3. Run Python analysis 

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_SCRIPT="$SCRIPT_DIR/build.sh"

# =====================================================================
# Defaults
# =====================================================================
MODE="both"           # scalar | optimised | both
BENCHMARK="performance" 
SKIP_BUILD=false
SKIP_ANALYSIS=false

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./scripts/benchmark.sh [options]

Options:
  --mode MODE        Compiler mode: scalar | optimised | both (default: both)
  --skip-build       Reuse existing binaries, do not recompile
  --skip-analysis    Skip the Python analysis step
  -h, --help         Show this help message

Modes:
  scalar      -O2 -fno-tree-vectorize -fno-inline  (scalar reference baseline)
  optimised   -O3 -march=native                    (compiler-optimised single-core baseline)
  both        Run scalar first, then optimised

Output:
  Results are written to benchmarks/performance/results/.
  When running both modes, each profile's results are also archived to
  benchmarks/performance/results/ before the next run overwrites them.

Examples:
  ./scripts/benchmark.sh
  ./scripts/benchmark.sh --mode optimised
  ./scripts/benchmark.sh --mode scalar
  ./scripts/benchmark.sh --skip-build --skip-analysis
EOF
}

# =====================================================================
# Parse arguments
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --mode)       MODE="$2";      shift 2;;
        --skip-build)    SKIP_BUILD=true;    shift;;
        --skip-analysis) SKIP_ANALYSIS=true; shift;;
        -h|--help) usage; exit 0;;
        *) echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

case "$MODE" in
    scalar|optimised|both) ;;
    *) echo "Error: invalid --mode '$MODE'"; usage; exit 1;;
esac

# =====================================================================
# Build
# =====================================================================
build_profile() {
    local profile="$1"
    local build_dir="$ROOT/build/benchmark_${profile}"

    local release_flags
    case "$profile" in
        scalar)    release_flags="-O2 -fno-tree-vectorize -fno-inline -DNDEBUG";;
        optimised) release_flags="-O3 -march=native -DNDEBUG";;
    esac

    echo ""
    echo "======================================================"
    echo " Build — profile: ${profile}"
    echo "   flags: ${release_flags}"
    echo "======================================================"

    "$BUILD_SCRIPT" \
        --build-dir  "$build_dir" \
        --build-type Release \
        "-D3DPE_BUILD_TESTS=OFF" \
        "-D3DPE_ENABLE_COVERAGE=OFF" \
        "-DCMAKE_CXX_FLAGS_RELEASE=${release_flags}"
}

# =====================================================================
# Run benchmarks
# =====================================================================
run_benchmarks() {
    local profile="$1"
    local archive="$2"   # true | false — whether to copy results to results_<profile>/
    local bin_dir="$ROOT/build/benchmark_${profile}/benchmarks"

    echo ""
    echo "======================================================"
    echo " Run — profile: ${profile}"
    echo "======================================================"

    cd "$ROOT"
    "$bin_dir/Performance"
    if [[ "$archive" == true ]]; then
        rm -rf "benchmarks/performance/results_${profile}"
        cp -r  "benchmarks/performance/results" \
                "benchmarks/performance/results_${profile}"
        echo "[run] Results archived to benchmarks/performance/results_${profile}/"
    fi

}

# =====================================================================
# Analysis
# =====================================================================
run_analysis() {
    echo ""
    echo "======================================================"
    echo " Analysis"
    echo "======================================================"

    cd "$ROOT/benchmarks/performance"
    python3 benchmark_analysis.py
    cd "$ROOT"
    
}

# =====================================================================
# Main
# =====================================================================
run_mode() {
    local profile="$1"
    local archive="$2"

    if [[ "$SKIP_BUILD" == false ]]; then
        build_profile "$profile"
    fi

    run_benchmarks "$profile" "$archive"

    if [[ "$SKIP_ANALYSIS" == false ]]; then
        run_analysis
    fi
}

case "$MODE" in
    scalar)
        run_mode scalar false
        ;;
    optimised)
        run_mode optimised false
        ;;
    both)
        # Archive scalar results so the optimised run doesn't overwrite them.
        run_mode scalar    true
        run_mode optimised false
        ;;
esac

echo ""
echo "[DONE] Benchmark pipeline completed  (mode=${MODE}, benchmark=${BENCHMARK})"
