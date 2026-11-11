#!/usr/bin/env bash
set -euo pipefail

# Benchmark sweeps : timing + analytic FLOP/s, in two independent sweeps:
#   - n_objects sweep: every broad phase, one solver, fixed dt (how each broad phase scales with N),
#   - dt sweep: every solver, one broad phase, fixed n_objects (how each solver's step cost behaves with dt).
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

# n_objects sweep: BROADS compared with solver N_SOLVER, at DT_FIXED
BROADS=("Brut" "Grid" "SweepPrune")
N_SOLVER="Verlet"
N_OBJECTS=(1 2 5 10 20 50 100 200 500 1000 2000 5000 10000)
DT_FIXED="1e-3"
# Brut is O(N²) (~70 ms per step at N=5000): above BRUT_N_MAX it would take hours, so it is skipped
BRUT_N_MAX=2000
# dt sweep: SOLVERS compared with broad phase DT_BROAD, at N_FIXED. Brut is the reference: its
# cost does not depend on where the objects are
SOLVERS=("Euler" "Verlet" "RK4")
DT_BROAD="Brut"
N_FIXED=100
DT_MIN="1e-5"
DT_MAX="1e-2"
DT_N=5

# Simulation parameters. The per-step cost does not depend on the simulated duration, so a
# short one keeps the whole run around 1 hour (estimated: N=5000-10000 and the finest dt dominate).
DUR="1.0"
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

Builds the scalar and optimised reference profiles and, for each, runs two sweeps,
writing benchmarks/performance/results/benchmark.csv:
  - n_objects sweep: every broad phase, one solver, fixed dt,
  - dt sweep: every solver, one broad phase, fixed n_objects.

Options:
  --broads LIST      broad phases of the n_objects sweep  (default: "Brut Grid SweepPrune")
  --n-solver NAME    solver of the n_objects sweep  (default: Verlet)
  --n-objects LIST   object counts of the n_objects sweep  (default: "1 2 5 ... 5000 10000")
  --brut-n-max N     largest object count run with Brut  (default: 2000)
  --dt VALUE         timestep of the n_objects sweep  (default: 1e-3)
  --solvers LIST     solvers of the dt sweep  (default: "Euler Verlet RK4")
  --dt-broad NAME    broad phase of the dt sweep  (default: Brut)
  --n-fixed N        object count of the dt sweep  (default: 100)
  --dt-min VALUE     smallest timestep of the dt sweep  (default: 1e-5)
  --dt-max VALUE     largest timestep of the dt sweep   (default: 1e-2)
  --dt-n N           number of log-spaced dt values  (default: 5)
  --dur VALUE        simulated duration in s  (default: 1.0)
  --n-warmup N       discarded warmup runs  (default: 1)
  --n-runs N         timed runs  (default: 5)
  --skip-build       reuse existing binaries instead of an (incremental) rebuild
  --skip-analysis    skip the Python analysis step
  -h, --help         show this help

Every run is pinned to one P-core, and the CPU governor is set to 'performance' for the whole
run (sudo password asked once, previous governor restored at exit; see
benchmarks/performance/profiles.sh). With the default
sweeps, expect about 1 hour (estimate), dominated by N=5000-10000 and by the dt sweep at
dt=1e-5 (drop them with --n-objects or --dt-min 1e-4).

Output:
  benchmarks/performance/results_scalar/benchmark.csv   scalar profile
  benchmarks/performance/results/benchmark.csv          optimised profile
  benchmarks/performance/results/plot_*.png, benchmark_report.pdf   (benchmark_analysis.py)

Next steps (not run by this script):
  scripts/profile.sh                               all three below, in one command
  benchmarks/performance/perf/profile_perf.sh      hardware counters + flamegraph
  benchmarks/performance/maqao/profile_maqao.sh    vectorisation per loop
  benchmarks/performance/malt/profile_malt.sh      allocation profiling
EOF
}

# =====================================================================
# Argument parsing
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --solvers)       read -ra SOLVERS    <<< "$2"; shift 2 ;;
        --broads)        read -ra BROADS     <<< "$2"; shift 2 ;;
        --n-solver)      N_SOLVER="$2";            shift 2 ;;
        --dt-broad)      DT_BROAD="$2";            shift 2 ;;
        --n-objects)     read -ra N_OBJECTS  <<< "$2"; shift 2 ;;
        --dt)            DT_FIXED="$2";            shift 2 ;;
        --brut-n-max)    BRUT_N_MAX="$2";          shift 2 ;;
        --n-fixed)       N_FIXED="$2";             shift 2 ;;
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
# Run benchmark - solver x broad phase x n_objects x dt loop
# =====================================================================
run_benchmark() {
    local profile="$1" out_dir="$2"
    local bin cpu
    bin="$(performance_binary "$profile")"
    cpu="$(bench_cpu)"

    echo ""
    echo "======================================================"
    echo " Run - profile: ${profile}"
    echo "   n_objects sweep: ${N_OBJECTS[*]}  (broad phases: ${BROADS[*]}, solver = ${N_SOLVER}, dt = ${DT_FIXED})"
    echo "   dt sweep: ${DT_VALUES[0]} ... ${DT_VALUES[-1]}  (${#DT_VALUES[@]} values, solvers: ${SOLVERS[*]}, broad = ${DT_BROAD}, n_objects = ${N_FIXED})"
    echo "======================================================"

    if [[ ! -x "$bin" ]]; then
        echo "Error: binary not found or not executable: ${bin}" >&2
        echo "       (build it first, or drop --skip-build)" >&2
        exit 1
    fi

    cd "$ROOT"

    local first=true
    run_point() {
        local solver="$1" broad="$2" n_obj="$3" dt="$4"
        local append_flag=""
        $first || append_flag="--append"
        first=false

        taskset -c "$cpu" "$bin" \
            --solver     "$solver"   \
            --broad      "$broad"    \
            --dt         "$dt"       \
            --dur        "$DUR"      \
            --n_warmup   "$N_WARMUP" \
            --n_runs     "$N_RUNS"   \
            --n_objects  "$n_obj"    \
            --out        "$out_dir"  \
            $append_flag
    }

    for broad in "${BROADS[@]}"; do
        echo ""
        echo "  n_objects sweep - broad: ${broad}  (solver = ${N_SOLVER}, dt = ${DT_FIXED})"
        for n_obj in "${N_OBJECTS[@]}"; do
            if [[ "$broad" == "Brut" ]] && (( n_obj > BRUT_N_MAX )); then
                echo "  skip Brut at n_objects=${n_obj} (> --brut-n-max ${BRUT_N_MAX})"
                continue
            fi
            run_point "$N_SOLVER" "$broad" "$n_obj" "$DT_FIXED"
        done
    done

    for solver in "${SOLVERS[@]}"; do
        echo ""
        echo "  dt sweep - solver: ${solver}  (broad = ${DT_BROAD}, n_objects = ${N_FIXED})"
        for dt in "${DT_VALUES[@]}"; do
            # Already measured by the n_objects sweep
            if [[ "$solver" == "$N_SOLVER" && " ${BROADS[*]} " == *" ${DT_BROAD} "* \
                  && " ${N_OBJECTS[*]} " == *" ${N_FIXED} "* ]] \
                && awk "BEGIN { exit !($dt == $DT_FIXED) }"; then
                continue
            fi
            run_point "$solver" "$DT_BROAD" "$N_FIXED" "$dt"
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
    local profile="$1" out_dir="$2"

    if [[ "$SKIP_BUILD" == false ]]; then
        echo ""
        echo "======================================================"
        echo " Build - profile: ${profile}"
        echo "======================================================"
        build_profile "$profile" "$(build_dir_for "$profile")"
    fi

    run_benchmark "$profile" "$out_dir"
}

# =====================================================================
# Main - always scalar then optimised, no --mode flag
# =====================================================================
check_cpu_setup
build_dt_array
run_mode scalar    "${REPO}/results_scalar"
run_mode optimised "${RESULTS_DIR}"

[[ "$SKIP_ANALYSIS" == false ]] && run_analysis

echo ""
echo "n_objects sweep: broads=${BROADS[*]} solver=${N_SOLVER} n_objects=${N_OBJECTS[*]} dt=${DT_FIXED}"
echo "dt sweep: solvers=${SOLVERS[*]} broad=${DT_BROAD} n_objects=${N_FIXED} dt=${DT_MIN}..${DT_MAX} x${DT_N}"
echo "dur=${DUR}"
echo ""
echo "Sweep done. For hardware-counter/vectorisation/memory analysis on a representative"
echo "workload, run scripts/profile.sh (all three tools) or the individual scripts under"
echo "benchmarks/performance/{perf,maqao,malt}/."
