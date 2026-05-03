#!/usr/bin/env bash
# profile.sh — Record a perf profile, generate a flamegraph, and run analysis.
#
# Usage:
#   ./profiling/perf/profile.sh
#   ./profiling/perf/profile.sh -- --solver RK4 --dt 1e-4
#   ./profiling/perf/profile.sh --target benchmarks/Contact_Forces
#   ./profiling/perf/profile.sh --hotspot

set -euo pipefail

# Locate repo root (parent of this script's directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Defaults
TARGET_REL="profiling/perf_target"   # path relative to build/
OPEN_HOTSPOT=false
MEASURE_MFLOPS=true
FREQ=999
CALL_GRAPH="dwarf"
BIN_ARGS=()   # extra args forwarded to the profiled binary (after --)

RESULTS_DIR="${REPO_ROOT}/profiling/perf/results"
PERF_DATA="${RESULTS_DIR}/perf.data"
PERF_SCRIPT="${RESULTS_DIR}/perf.script"
FLAME_SVG="${RESULTS_DIR}/flame.svg"
MFLOPS_JSON="${RESULTS_DIR}/mflops_stats.json"
ANALYZE_PY="${SCRIPT_DIR}/analyse_perf.py"

# Argument parsing
usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [-- BINARY_ARGS...]

Options:
  --target PATH   Binary to profile, relative to build/ (default: profiling/perf_target)
  --hotspot       Open hotspot GUI after profiling
  --mflops        Measure MFLOP/s (floating-point operations per second)
  --freq N        Sampling frequency in Hz (default: 999)
  --fp            Use frame-pointer call graph instead of DWARF (faster, shallower stacks)
  -h, --help      Show this help

Binary args (passed after --):
  --solver {Euler|Verlet|RK4}   Integrator (default: Verlet)
  --dt SECONDS                  Timestep   (default: 5e-5)
  --dur SECONDS                 Simulation duration (default: 30)
  --contact-forces              Use spring-damper contact instead of impulse

Examples:
  $(basename "$0")
  $(basename "$0") -- --solver RK4 --dt 1e-4
  $(basename "$0") -- --solver Euler --dur 10
  $(basename "$0") --target benchmarks/Contact_Forces
  $(basename "$0") --target examples/Bouncing --hotspot
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --target)   TARGET_REL="$2"; shift 2 ;;
        --hotspot)  OPEN_HOTSPOT=true; shift ;;
        --mflops)   MEASURE_MFLOPS=true; shift ;;
        --freq)     FREQ="$2"; shift 2 ;;
        --fp)       CALL_GRAPH="fp"; shift ;;
        -h|--help)  usage; exit 0 ;;
        --)         shift; BIN_ARGS=("$@"); break ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

BINARY="${REPO_ROOT}/build/${TARGET_REL}"

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

# MFLOP/s measurement function
measure_mflops() {
    local binary="$1"
    shift
    local temp_json=$(mktemp)
    trap "rm -f ${temp_json}" RETURN
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Measuring MFLOP/s…"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    perf stat -e cycles,instructions,fp_arith_inst_retired:u -j \
        -o "${temp_json}" \
        "${binary}" "$@" 2>&1 | tail -20
    
    # Parse JSON and calculate MFLOP/s
    if [[ -f "${temp_json}" ]]; then
        python3 << 'PYTHON_EOF' "${temp_json}" "${MFLOPS_JSON}"
import json
import sys

try:
    with open(sys.argv[1]) as f:
        data = json.load(f)
    
    fp_ops = 0
    elapsed_ns = 0
    
    for entry in data:
        if entry.get('event') == 'fp_arith_inst_retired:u':
            fp_ops = entry.get('value', 0)
        if entry.get('unit') == 'msec':
            elapsed_ns = int(entry.get('value', 0) * 1e6)  # ms to ns
    
    if fp_ops > 0 and elapsed_ns > 0:
        elapsed_s = elapsed_ns / 1e9
        mflops = (fp_ops / elapsed_s) / 1e6
        print(f"\n✓ MFLOP/s: {mflops:,.2f}")
        print(f"  Total FP operations: {fp_ops:,}")
        print(f"  Elapsed time: {elapsed_s:.3f}s")
        with open(sys.argv[2], 'w') as out:
            json.dump({
                'mflops': mflops,
                'fp_operations': fp_ops,
                'elapsed_seconds': elapsed_s
            }, out, indent=2)
    else:
        print("⚠  Could not extract FP operation count or timing.")
        print("  Ensure perf has access to fp_arith_inst_retired event.")
except Exception as e:
    print(f"⚠  Error calculating MFLOP/s: {e}", file=sys.stderr)
PYTHON_EOF
    fi
}

# Step 1: Record
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Profiling: ${BINARY}"
[[ ${#BIN_ARGS[@]} -gt 0 ]] && echo "  Args:      ${BIN_ARGS[*]}"
echo "  Frequency: ${FREQ} Hz   Call graph: ${CALL_GRAPH}"
${MEASURE_MFLOPS} && echo "  MFLOP/s:   enabled"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

cd "${REPO_ROOT}"

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

# Step 4: MFLOP/s measurement (optional)
if ${MEASURE_MFLOPS}; then
    echo ""
    measure_mflops "${BINARY}" "${BIN_ARGS[@]}"
fi

# Step 5: Python analysis
echo ""
if [[ -f "${ANALYZE_PY}" ]] && command -v python3 &>/dev/null; then
    python3 "${ANALYZE_PY}" \
        --input  "${PERF_SCRIPT}" \
        --output "${RESULTS_DIR}/hotspots.png"
else
    echo "⚠  analyze.py not found or python3 unavailable — skipping automated report."
fi

# Step 6: Summary
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Output files:"
echo "    ${PERF_DATA}"
echo "    ${PERF_SCRIPT}"
${HAVE_FLAMEGRAPH} && echo "    ${FLAME_SVG}"
[[ -f "${RESULTS_DIR}/hotspots.png" ]] && echo "    ${RESULTS_DIR}/hotspots.png"
${MEASURE_MFLOPS} && [[ -f "${MFLOPS_JSON}" ]] && echo "    ${MFLOPS_JSON}"
echo ""
echo "  Next steps:"
echo "    perf report -i ${PERF_DATA}"
[[ -f "${FLAME_SVG}" ]] && echo "    xdg-open ${FLAME_SVG}"
echo "    hotspot ${PERF_DATA}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Step 7: Optional hotspot GUI
if ${OPEN_HOTSPOT}; then
    if command -v hotspot &>/dev/null; then
        echo "Opening hotspot …"
        hotspot "${PERF_DATA}" &
    else
        echo "⚠  hotspot not found on PATH." >&2
    fi
fi
