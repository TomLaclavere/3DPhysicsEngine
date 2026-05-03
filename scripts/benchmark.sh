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
# Perf parameters
# =====================================================================
TARGET_REL="benchmarks/Performance"   # path relative to build/
OPEN_HOTSPOT=false
MEASURE_MFLOPS=true
FREQ=999
CALL_GRAPH="dwarf"
BIN_ARGS=()   # extra args forwarded to the profiled binary (after --)

REPO="${ROOT}/benchmarks/performance"
RESULTS_DIR="${REPO}/results"
PERF_DATA="${RESULTS_DIR}/perf.data"
PERF_SCRIPT="${RESULTS_DIR}/perf.script"
FLAME_SVG="${RESULTS_DIR}/flame.svg"
ANALYSE_PY="${REPO}/analyse_perf.py"

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

    # Perf
    # while [[ $# -gt 0 ]]; do
    #     case "$1" in
    #         --target)   TARGET_REL="$2"; shift 2 ;;
    #         --hotspot)  OPEN_HOTSPOT=true; shift ;;
    #         --freq)     FREQ="$2"; shift 2 ;;
    #         --fp)       CALL_GRAPH="fp"; shift ;;
    #         -h|--help)  usage; exit 0 ;;
    #         --)         shift; BIN_ARGS=("$@"); break ;;
    #         *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    #     esac
    # done

    BINARY="${ROOT}/build/${TARGET_REL}"

    # Preflight checks
    if ! command -v perf &>/dev/null; then
        echo "Error: 'perf' not found on PATH." >&2
        exit 1
    fi

    if [[ ! -x "${BINARY}" ]]; then
        echo "Error: binary not found or not executable: ${BINARY}" >&2
        echo "Build the project first: ./scripts/build.sh" >&2
        exit 1
    fi

    STACKCOLLAPSE=$(command -v stackcollapse-perf.pl 2>/dev/null || true)
    FLAMEGRAPH=$(command -v flamegraph.pl 2>/dev/null || true)
    HAVE_FLAMEGRAPH=false
    if [[ -x "${STACKCOLLAPSE}" && -x "${FLAMEGRAPH}" ]]; then
        HAVE_FLAMEGRAPH=true
    fi

    mkdir -p "${RESULTS_DIR}"
    

    # Step 1: Record
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Profiling: ${BINARY}"
    [[ ${#BIN_ARGS[@]} -gt 0 ]] && echo "  Args:      ${BIN_ARGS[*]}"
    echo "  Frequency: ${FREQ} Hz   Call graph: ${CALL_GRAPH}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    cd "${ROOT}"

    perf record -F "${FREQ}" -g --call-graph "${CALL_GRAPH}" \
        -o "${PERF_DATA}" \
        "${BINARY}" "${BIN_ARGS[@]}"

    echo ""
    echo "✓ Raw data  →  ${PERF_DATA}"

    # Step 2: Convert to text script
    echo "Converting to perf.script …"
    perf script -i "${PERF_DATA}" > "${PERF_SCRIPT}"
    echo "✓ Script    →  ${PERF_SCRIPT}"

    # Step 3: Flamegraph
    if ${HAVE_FLAMEGRAPH}; then
        echo "Generating flamegraph …"
        "${STACKCOLLAPSE}" "${PERF_SCRIPT}" | "${FLAMEGRAPH}" > "${FLAME_SVG}"
        echo "✓ Flamegraph →  ${FLAME_SVG}"
    else
        echo "⚠  stackcollapse-perf.pl / flamegraph.pl not found — skipping flamegraph."
    fi

    # Step 4: Python analysis
    echo ""
    if [[ -f "${ANALYSE_PY}" ]] && command -v python3 &>/dev/null; then
        python3 "${ANALYSE_PY}" \
            --input  "${PERF_SCRIPT}" \
            --output "${RESULTS_DIR}/hotspots.png"
    else
        echo "⚠  analyze.py not found or python3 unavailable — skipping automated report."
    fi

    # Step 5: Summary
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Output files:"
    echo "    ${PERF_DATA}"
    echo "    ${PERF_SCRIPT}"
    ${HAVE_FLAMEGRAPH} && echo "    ${FLAME_SVG}"
    [[ -f "${RESULTS_DIR}/hotspots.png" ]] && echo "    ${RESULTS_DIR}/hotspots.png"
    echo ""
    echo "  Next steps:"
    echo "    perf report -i ${PERF_DATA}"
    [[ -f "${FLAME_SVG}" ]] && echo "    xdg-open ${FLAME_SVG}"
    echo "    hotspot ${PERF_DATA}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    # Step 6: Optional hotspot GUI
    if ${OPEN_HOTSPOT}; then
        if command -v hotspot &>/dev/null; then
            echo "Opening hotspot …"
            hotspot "${PERF_DATA}" &
        else
            echo "⚠  hotspot not found on PATH." >&2
        fi
    fi


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

    # if [[ "$SKIP_ANALYSIS" == false ]]; then
    #     run_analysis
    # fi
    echo ""
    if [[ -f "${ANALYSE_PY}" ]] && command -v python3 &>/dev/null; then
        python3 "${ANALYSE_PY}" \
            --input  "${PERF_SCRIPT}" \
            --output "${RESULTS_DIR}/hotspots.png"
    else
        echo "⚠  analyze.py not found or python3 unavailable — skipping automated report."
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
