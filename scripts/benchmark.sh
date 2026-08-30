#!/usr/bin/env bash
set -euo pipefail

# Benchmark sweep : timing + analytic FLOP/s across solver x n_objects x dt.
#
# This script's only job is: build the two reference profiles (scalar / optimised, see
# benchmarks/performance/profiles.sh), run benchmarks/Performance across a parameter sweep for
# each, and hand the resulting CSVs to the Python analysis step.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# shellcheck source=../benchmarks/performance/profiles.sh
source "$ROOT/benchmarks/performance/profiles.sh"

REPO="${ROOT}/benchmarks/performance"
RESULTS_DIR="${REPO}/results"
ANALYSE_PY="${REPO}/benchmark_analysis.py"

# Solvers, dt sweep, and object-count sweep
SOLVERS=("Euler" "Verlet" "RK4")
N_OBJECTS=(1 2 5 10 20 50 100)
DT_MIN="1e-5"
DT_MAX="1e-2"
DT_N=5

# Simulation parameters
DUR="22.0"
N_WARMUP=1
N_RUNS=5

# Pipeline control
SKIP_BUILD=false
SKIP_ANALYSIS=false

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./scripts/benchmark.sh [options]

Builds the scalar and optimised reference profiles and runs the solver x n_objects x dt
sweep for each, writing benchmarks/performance/results/benchmark.csv.

Options:
  --solvers LIST     space-separated solver names  (default: "Euler Verlet RK4")
  --n-objects LIST   space-separated object counts  (default: "1 2 5 10 20 50 100")
  --dt-min VALUE     smallest timestep  (default: 1e-5)
  --dt-max VALUE     largest timestep   (default: 1e-2)
  --dt-n N           number of log-spaced dt values  (default: 5)
  --dur VALUE        simulated duration in s  (default: 22.0)
  --n-warmup N       discarded warmup runs  (default: 1)
  --n-runs N         timed runs  (default: 5)
  --skip-build       reuse existing binaries
  --skip-analysis    skip the Python analysis step
  -h, --help         show this help

Output:
  benchmarks/performance/results/
    benchmark.csv              timing + FLOP/s for all (solver, dt, n_objects) combinations
    energy_drift.csv
    benchmark_report.pdf, plot_*.png   (from benchmark_analysis.py)

Next steps (not run by this script):
  scripts/profile.sh                               all three below, in one command
  benchmarks/performance/perf/profile_perf.sh      hardware counters + flamegraph
  benchmarks/performance/maqao/profile_maqao.sh    vectorisation + roofline
  benchmarks/performance/malt/profile_malt.sh      allocation profiling
EOF
}

# =====================================================================
# Argument parsing
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --solvers)       read -ra SOLVERS    <<< "$2"; shift 2 ;;
        --n-objects)     read -ra N_OBJECTS  <<< "$2"; shift 2 ;;
        --dt-min)        DT_MIN="$2";              shift 2 ;;
        --dt-max)        DT_MAX="$2";              shift 2 ;;
        --dt-n)          DT_N="$2";                shift 2 ;;
        --dur)           DUR="$2";                 shift 2 ;;
        --n-warmup)      N_WARMUP="$2";            shift 2 ;;
        --n-runs)        N_RUNS="$2";              shift 2 ;;
        --skip-build)    SKIP_BUILD=true;          shift ;;
        --skip-analysis) SKIP_ANALYSIS=true;       shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

# =====================================================================
# Generate dt array (log-spaced via python3)
# =====================================================================
build_dt_array() {
    mapfile -t DT_VALUES < <(python3 -c "
import numpy as np
for x in np.geomspace($DT_MIN, $DT_MAX, $DT_N):
    print(f'{x:.6e}')
")
    echo "[dt] ${#DT_VALUES[@]} values from ${DT_MIN} to ${DT_MAX}"
}

# =====================================================================
# Run benchmark - solver x n_objects x dt triple loop
# =====================================================================
run_benchmark() {
    local profile="$1"
    local bin
    bin="$(performance_binary "$profile")"

    echo ""
    echo "======================================================"
    echo " Run - profile: ${profile}"
    echo "   solvers: ${SOLVERS[*]}"
    echo "   dt: ${DT_VALUES[0]} ... ${DT_VALUES[-1]}  (${#DT_VALUES[@]} values)"
    echo "======================================================"

    if [[ ! -x "$bin" ]]; then
        echo "Error: binary not found or not executable: ${bin}" >&2
        echo "       (build it first, or drop --skip-build)" >&2
        exit 1
    fi

    cd "$ROOT"

    local first=true
    for solver in "${SOLVERS[@]}"; do
        for n_obj in "${N_OBJECTS[@]}"; do
            echo ""
            echo "  Solver: ${solver}  n_objects: ${n_obj}"
            for dt in "${DT_VALUES[@]}"; do
                local append_flag=""
                $first || append_flag="--append"
                first=false

                "$bin" \
                    --solver     "$solver"   \
                    --dt         "$dt"       \
                    --dur        "$DUR"      \
                    --n_warmup   "$N_WARMUP" \
                    --n_runs     "$N_RUNS"   \
                    --n_objects  "$n_obj"    \
                    $append_flag
            done
        done
    done
}

# =====================================================================
# Python analysis
# =====================================================================
run_analysis() {
    echo ""
    echo "======================================================"
    echo " Analysis"
    echo "======================================================"

    if ! command -v python3 &>/dev/null; then
        echo "Warning: python3 not found - skipping analysis" >&2
        return
    fi

    python3 "$ANALYSE_PY"
}

# =====================================================================
# Orchestration
# =====================================================================
run_mode() {
    local profile="$1"

    mkdir -p "${RESULTS_DIR}"

    if [[ "$SKIP_BUILD" == false ]]; then
        echo ""
        echo "======================================================"
        echo " Build - profile: ${profile}"
        echo "======================================================"
        build_profile "$profile" "$(build_dir_for "$profile")"
    fi

    run_benchmark "$profile"

    # Archive scalar results before the optimised run overwrites results/
    if [[ "$profile" == "scalar" ]]; then
        rm -rf "${REPO}/results_scalar"
        cp -r  "${RESULTS_DIR}" "${REPO}/results_scalar"
        echo "Results archived -> ${REPO}/results_scalar/"
    fi
}

# =====================================================================
# Main - always scalar then optimised, no --mode flag
# =====================================================================
build_dt_array
run_mode scalar
run_mode optimised

[[ "$SKIP_ANALYSIS" == false ]] && run_analysis

echo ""
echo "solvers=${SOLVERS[*]}  n_objects=${N_OBJECTS[*]}  dt_n=${DT_N}  dt=${DT_MIN}..${DT_MAX}  dur=${DUR}"
echo ""
echo "Sweep done. For hardware-counter/vectorisation/memory analysis on a representative"
echo "workload, run scripts/profile.sh (all three tools) or the individual scripts under"
echo "benchmarks/performance/{perf,maqao,malt}/."
