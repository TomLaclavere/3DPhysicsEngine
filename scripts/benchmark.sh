#!/usr/bin/env bash
set -euo pipefail

# Benchmark pipeline
#
# Two compiler profiles:
#   scalar     -O2 -fno-tree-vectorize -fno-inline  (scalar reference baseline)
#   optimised  -O3 -march=native                    (full optimisations, single-core)
#
# Steps per profile:
#   1. Build   — recompile with the appropriate flags
#   2. Run     — for each solver × dt: execute benchmark binary → append to benchmark.csv
#   3. Stat    — perf stat per solver at DT_MIN (finest dt, most iterations)
#   4. Record  — perf record → flamegraph (first solver, DT_MIN)
#   5. Analyse — Python post-processing

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_SCRIPT="$SCRIPT_DIR/build.sh"

REPO="${ROOT}/benchmarks/performance"
RESULTS_DIR="${REPO}/results"
ANALYSE_PY="${REPO}/benchmark_analysis.py"

# perf record output files (overwritten per profile)
PERF_DATA="${RESULTS_DIR}/perf.data"
PERF_SCRIPT_OUT="${RESULTS_DIR}/perf.script"
FLAME_SVG="${RESULTS_DIR}/flame.svg"

# perf record settings
FREQ=999
CALL_GRAPH="dwarf"

# Solvers, dt sweep, and object-count sweep
SOLVERS=("Euler" "Verlet" "RK4")
N_OBJECTS=(1 2 5 10 20)
DT_MIN="1e-5"
DT_MAX="1e-2"
DT_N=5
PERF_DT=""          # empty = use DT_MIN for perf stat (finest dt → most stable counters)
PERF_N_OBJ=""       # empty = use N_OBJECTS[0] (smallest count → cleanest signal) for perf stat

# Simulation parameters
DUR="22.0"
N_WARMUP=1
N_RUNS=5

# Pipeline control
SKIP_BUILD=false
SKIP_PERF=false
SKIP_ANALYSIS=false

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./scripts/benchmark.sh [options]

Options:
  --solvers LIST     space-separated solver names  (default: "Euler Verlet RK4")
  --n-objects LIST   space-separated object counts  (default: "1 5 10 25 50")
  --dt-min VALUE     smallest timestep  (default: 1e-5)
  --dt-max VALUE     largest timestep   (default: 1e-2)
  --dt-n N           number of log-spaced dt values  (default: 5)
  --perf-dt VALUE    dt used for perf stat  (default: DT_MIN)
  --perf-n-obj N     object count used for perf stat  (default: N_OBJECTS[0])
  --dur VALUE        simulated duration in s  (default: 22.0)
  --n-warmup N       discarded warmup runs  (default: 1)
  --n-runs N         timed runs  (default: 5)
  --skip-build       reuse existing binaries
  --skip-perf        skip perf stat and perf record
  --skip-analysis    skip Python analysis step
  -h, --help         show this help

Output:
  benchmarks/performance/results/
    benchmark.csv              timing + FLOP/s for all (solver, dt) combinations
    perf_stat_<p>_<s>.txt     perf stat per profile+solver at DT_MIN
    perf.data / perf.script    raw perf record data
    flame.svg                  flamegraph (requires flamegraph.pl)
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
        --perf-dt)       PERF_DT="$2";             shift 2 ;;
        --perf-n-obj)    PERF_N_OBJ="$2";          shift 2 ;;
        --dur)           DUR="$2";                 shift 2 ;;
        --n-warmup)      N_WARMUP="$2";            shift 2 ;;
        --n-runs)        N_RUNS="$2";              shift 2 ;;
        --skip-build)    SKIP_BUILD=true;          shift ;;
        --skip-perf)     SKIP_PERF=true;           shift ;;
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
# Build
# =====================================================================
build_profile() {
    local profile="$1"
    local build_dir="$ROOT/build/benchmark_${profile}"

    local release_flags
    case "$profile" in
        scalar)    release_flags="-O2 -fno-tree-vectorize -fno-inline -DNDEBUG" ;;
        optimised) release_flags="-O3 -march=native -DNDEBUG" ;;
    esac

    echo ""
    echo "======================================================"
    echo " Build — profile: ${profile}"
    echo "   flags: ${release_flags}"
    echo "======================================================"

    "$BUILD_SCRIPT" \
        --clean \
        --build-dir  "$build_dir" \
        --build-type Release \
        "-D3DPE_BUILD_TESTS=OFF" \
        "-D3DPE_ENABLE_COVERAGE=OFF" \
        "-DCMAKE_CXX_FLAGS_RELEASE=${release_flags}"
}

# =====================================================================
# Run benchmark — solver × dt double loop, append after first row
# =====================================================================
run_benchmark() {
    local profile="$1"
    local bin="$ROOT/build/benchmark_${profile}/benchmarks/Performance"

    echo ""
    echo "======================================================"
    echo " Run — profile: ${profile}"
    echo "   solvers: ${SOLVERS[*]}"
    echo "   dt: ${DT_VALUES[0]} … ${DT_VALUES[-1]}  (${#DT_VALUES[@]} values)"
    echo "======================================================"

    if [[ ! -x "$bin" ]]; then
        echo "Error: binary not found or not executable: ${bin}" >&2
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
# perf stat — per solver at representative dt
# =====================================================================
run_perf_stat() {
    local profile="$1"
    local bin="$ROOT/build/benchmark_${profile}/benchmarks/Performance"
    local rep_dt="${PERF_DT:-$DT_MIN}"

    echo ""
    echo "======================================================"
    echo " perf stat — profile: ${profile}  dt=${rep_dt}"
    echo "======================================================"

    cd "$ROOT"

    local rep_n_obj="${PERF_N_OBJ:-${N_OBJECTS[0]}}"

    for solver in "${SOLVERS[@]}"; do
        local out="${RESULTS_DIR}/perf_stat_${profile}_${solver}.txt"
        echo ""
        echo "  solver: ${solver}  n_objects: ${rep_n_obj} → ${out}"

        perf stat \
            -e cycles,instructions \
            -e L1-dcache-loads,L1-dcache-load-misses \
            -e LLC-loads,LLC-load-misses \
            -e branches,branch-misses \
            -e stalled-cycles-frontend,stalled-cycles-backend \
            "$bin" \
                --solver    "$solver"    \
                --dt        "$rep_dt"    \
                --dur       "$DUR"       \
                --n_warmup  0            \
                --n_runs    1            \
                --n_objects "$rep_n_obj" \
                --append                 \
            2>&1 | tee "$out"
    done
}

# =====================================================================
# perf record → flamegraph (first solver, finest dt)
# =====================================================================
run_perf_record() {
    local profile="$1"
    local bin="$ROOT/build/benchmark_${profile}/benchmarks/Performance"
    local rep_solver="${SOLVERS[0]}"
    local rep_dt="${PERF_DT:-$DT_MIN}"

    echo ""
    echo "======================================================"
    echo " perf record — profile: ${profile}  solver=${rep_solver}  dt=${rep_dt}"
    echo "======================================================"

    cd "$ROOT"

    local rep_n_obj="${PERF_N_OBJ:-${N_OBJECTS[0]}}"

    perf record -F "${FREQ}" -g --call-graph "${CALL_GRAPH}" \
        -o "${PERF_DATA}" \
        "$bin" \
            --solver    "$rep_solver" \
            --dt        "$rep_dt"     \
            --dur       "$DUR"        \
            --n_warmup  0             \
            --n_runs    1             \
            --n_objects "$rep_n_obj"  \
            --append

    perf script -i "${PERF_DATA}" > "${PERF_SCRIPT_OUT}"
    echo "perf record → ${PERF_DATA}"
    echo "perf script → ${PERF_SCRIPT_OUT}"

    local stackcollapse flamegraph_bin
    stackcollapse=$(command -v stackcollapse-perf.pl 2>/dev/null || true)
    flamegraph_bin=$(command -v flamegraph.pl 2>/dev/null || true)

    if [[ -x "${stackcollapse}" && -x "${flamegraph_bin}" ]]; then
        "${stackcollapse}" "${PERF_SCRIPT_OUT}" | "${flamegraph_bin}" > "${FLAME_SVG}"
        echo "flamegraph  → ${FLAME_SVG}"
    else
        echo "⚠  flamegraph tools not found — skipping (install FlameGraph from Brendan Gregg)"
    fi
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
        echo "⚠  python3 not found — skipping analysis"
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

    [[ "$SKIP_BUILD" == false ]] && build_profile "$profile"

    run_benchmark "$profile"

    if [[ "$SKIP_PERF" == false ]]; then
        if command -v perf &>/dev/null; then
            run_perf_stat   "$profile"
            run_perf_record "$profile"
        else
            echo "⚠  perf not found on PATH — skipping hardware counters"
        fi
    fi

    # Archive scalar results before the optimised run overwrites results/
    if [[ "$profile" == "scalar" ]]; then
        rm -rf "${REPO}/results_scalar"
        cp -r  "${RESULTS_DIR}" "${REPO}/results_scalar"
        echo "Results archived → ${REPO}/results_scalar/"
    fi
}

# =====================================================================
# Main — always scalar then optimised, no --mode flag
# =====================================================================
build_dt_array
run_mode scalar
run_mode optimised

[[ "$SKIP_ANALYSIS" == false ]] && run_analysis

[[ -f "${RESULTS_DIR}/benchmark_report.pdf" ]] && \
    cp "${RESULTS_DIR}/benchmark_report.pdf" "${REPO}/"

echo ""
echo "solvers=${SOLVERS[*]}  n_objects=${N_OBJECTS[*]}  dt_n=${DT_N}  dt=${DT_MIN}..${DT_MAX}  dur=${DUR}"
