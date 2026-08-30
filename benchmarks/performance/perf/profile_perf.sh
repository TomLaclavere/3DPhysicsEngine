#!/usr/bin/env bash
set -euo pipefail

# perf/profile_perf.sh - hardware counters + sampling profile for one representative workload.
#
# Runs perf stat (grouped events, repeated) + a Top-Down pass + perf record/flamegraph
# against benchmarks/Performance at one fixed (solver, dt, n_objects), for one or both
# build profiles (scalar/optimised). Deliberately NOT the full solver x N x dt sweep -
# see benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md Part 0.4 for why.
#
# Read benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md Part 1 first if you haven't - this
# script assumes you already understand what each flag/metric below is for. This directory's
# perf.md is the quick tool-specific reference once you have.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# shellcheck source=../profiles.sh
source "$ROOT/benchmarks/performance/profiles.sh"

# benchmark.cpp hardcodes its CSV output path as the *relative* string
# "benchmarks/performance/results" — everything below must run with cwd == repo root or the
# binary crashes trying to create that path (found while testing profile_maqao.sh/malt.sh).
cd "$ROOT"

RESULTS_DIR="$SCRIPT_DIR/results"

# Representative, fixed workload (override via flags below). dt/dur are deliberately much
# finer/longer than the CLI defaults' 1e-3/2.0 (2 000 steps, ~150us) - perf record needs
# enough wall-clock time to collect a meaningful number of samples at 999 Hz; ~600 000 steps
# gives tens of milliseconds even at n_objects=1.
SOLVER="Verlet"
DT="5e-5"
DUR="30"
N_OBJECTS=1

PROFILES=("scalar" "optimised")
REPEAT=5           # perf stat -r: repeat N times, report mean +- stddev
FREQ=999           # perf record sampling frequency (Hz) - avoids aliasing w/ 1 kHz kernel timers
CALL_GRAPH="dwarf"
SKIP_BUILD=false
SKIP_RECORD=false

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./benchmarks/performance/perf/profile_perf.sh [options]

Options:
  --profile NAME     scalar | optimised | both   (default: both)
  --solver NAME      solver for the representative run   (default: Verlet)
  --dt VALUE         timestep for the representative run  (default: 5e-5)
  --dur VALUE        simulated duration in s               (default: 30)
  --n-objects N      object count for the representative run (default: 1)
  --repeat N         perf stat repeat count (-r)          (default: 5)
  --freq N           perf record sampling frequency (Hz)  (default: 999)
  --fp               use frame-pointer call graph instead of dwarf (faster, shallower)
  --skip-build       reuse existing binaries (fail if missing)
  --skip-record      skip perf record / flamegraph, only run perf stat
  -h, --help         show this help

Output: benchmarks/performance/perf/results/
  perf_stat_<profile>.txt       grouped hardware counters, repeated (-r)
  perf_topdown_<profile>.txt    Top-Down microarchitecture breakdown
  perf_<profile>.data/.script   raw perf record data + resolved call stacks
  flame_<profile>.svg           flamegraph (if stackcollapse-perf.pl/flamegraph.pl on PATH)

Note: benchmarks/Performance always writes/appends to benchmarks/performance/results/
benchmark.csv (hardcoded in benchmark.cpp) and truncates energy_drift.csv on every
invocation - running this script adds a harmless extra row to the shared sweep CSV.
EOF
}

# =====================================================================
# Argument parsing
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --profile)
            case "$2" in
                both) PROFILES=("scalar" "optimised") ;;
                scalar|optimised) PROFILES=("$2") ;;
                *) echo "Unknown profile: $2 (expected scalar|optimised|both)" >&2; exit 1 ;;
            esac
            shift 2 ;;
        --solver)      SOLVER="$2";      shift 2 ;;
        --dt)          DT="$2";          shift 2 ;;
        --dur)         DUR="$2";         shift 2 ;;
        --n-objects)   N_OBJECTS="$2";   shift 2 ;;
        --repeat)      REPEAT="$2";      shift 2 ;;
        --freq)        FREQ="$2";        shift 2 ;;
        --fp)          CALL_GRAPH="fp";  shift ;;
        --skip-build)  SKIP_BUILD=true;  shift ;;
        --skip-record) SKIP_RECORD=true; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

# =====================================================================
# Prerequisites
# =====================================================================
command -v perf >/dev/null 2>&1 || {
    cat >&2 <<'EOF'
Error: perf not found on PATH.

Install:
  # Arch:
  sudo pacman -S perf

  # Ubuntu/Debian:
  sudo apt install linux-tools-common linux-tools-generic

See benchmarks/performance/perf/perf.md for the full walkthrough.
EOF
    exit 1
}

paranoid="$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo "")"
if [[ -n "$paranoid" && "$paranoid" -gt 1 ]]; then
    echo "Warning: kernel.perf_event_paranoid=${paranoid} may block hardware counters for a"
    echo "         non-root user. If perf stat fails below, run:"
    echo "           sudo sysctl kernel.perf_event_paranoid=1"
fi

# =====================================================================
# Hybrid P-core/E-core handling
#
# On a hybrid Intel CPU (P-cores + E-cores), perf exposes two separate PMUs (cpu_core,
# cpu_atom). A bare event name like "cycles" silently expands to both; whichever PMU the
# process didn't run on reports <not counted>/<not supported> for every line, which is noise,
# not a real problem - but on some CPU generations (this one included) it also means the
# legacy stalled-cycles-frontend/backend aliases aren't supported on either PMU at all (see
# the stall-event probe below; Top-Down supersedes them anyway). Pinning to one P-core and
# qualifying every event with cpu_core/.../ keeps the report to one PMU's real numbers.
# =====================================================================
PMU_PREFIX=""
PIN_CORE=0
if [[ -d /sys/devices/cpu_core ]]; then
    PMU_PREFIX="cpu_core/"
    PIN_CORE="$(cut -d- -f1 /sys/devices/cpu_core/cpus)"
    echo "[perf] Hybrid CPU detected (cpu_core + cpu_atom) - pinning to core ${PIN_CORE} (a P-core)"
fi

qevt() {  # qualify a bare event name for the detected PMU, e.g. cycles -> cpu_core/cycles/
    if [[ -n "$PMU_PREFIX" ]]; then
        echo "${PMU_PREFIX}${1}/"
    else
        echo "$1"
    fi
}

event_group() {  # event_group name1 name2 ... -> "{q(name1),q(name2),...}"
    local out="" e
    for e in "$@"; do out+="$(qevt "$e"),"; done
    echo "{${out%,}}"
}

pinned() { taskset -c "$PIN_CORE" "$@"; }

# Probe once whether stalled-cycles-frontend/backend exist on this CPU at all - not every
# generation supports them (superseded by Top-Down on recent Intel parts).
STALL_GROUP=""
if pinned perf stat -e "$(qevt stalled-cycles-frontend)" -- true >/dev/null 2>&1; then
    STALL_GROUP="$(event_group stalled-cycles-frontend stalled-cycles-backend)"
else
    echo "[perf] stalled-cycles-frontend/backend not supported on this CPU - skipping"
    echo "       (the Top-Down pass below covers the same question: Front-End vs Back-End Bound)"
fi

mkdir -p "$RESULTS_DIR"

# =====================================================================
# Build (if needed)
# =====================================================================
ensure_binary() {
    local profile="$1" bin
    bin="$(performance_binary "$profile")"
    if [[ ! -x "$bin" ]]; then
        if [[ "$SKIP_BUILD" == true ]]; then
            echo "Error: binary not found: ${bin} (and --skip-build was given)" >&2
            exit 1
        fi
        echo ""
        echo "======================================================"
        echo " Build - profile: ${profile}"
        echo "======================================================"
        build_profile "$profile" "$(build_dir_for "$profile")"
    fi
}

# =====================================================================
# perf stat - grouped + repeated hardware counters, plus a Top-Down pass
# =====================================================================
run_perf_stat() {
    local profile="$1" bin
    bin="$(performance_binary "$profile")"
    local out="${RESULTS_DIR}/perf_stat_${profile}.txt"
    local topdown_out="${RESULTS_DIR}/perf_topdown_${profile}.txt"

    echo ""
    echo "[perf stat] profile=${profile} solver=${SOLVER} dt=${DT} n_objects=${N_OBJECTS} (x${REPEAT})"

    local stat_args=(
        -e "$(event_group cycles instructions)"
        -e "$(event_group branches branch-misses)"
        -e "$(event_group L1-dcache-loads L1-dcache-load-misses)"
        -e "$(event_group LLC-loads LLC-load-misses)"
    )
    [[ -n "$STALL_GROUP" ]] && stat_args+=(-e "$STALL_GROUP")

    if ! pinned perf stat -r "$REPEAT" "${stat_args[@]}" -- \
            "$bin" --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
                   --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --append \
            > "$out" 2>&1
    then
        echo "perf stat failed - see ${out}" >&2
        cat "$out" >&2
        return 1
    fi
    echo "  -> ${out}"

    # Top-Down uses its own internal counter grouping - run as a separate pass rather than
    # mixed into the -e list above (see BENCHMARK_TOOLS_GUIDE.md §1.5).
    if ! pinned perf stat -M TopdownL1 -- \
            "$bin" --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
                   --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --append \
            > "$topdown_out" 2>&1
    then
        echo "Top-Down pass failed - see ${topdown_out}" >&2
        echo "(older perf/kernel: try '-M TopdownL1' -> '--topdown' in this script)" >&2
        cat "$topdown_out" >&2
        return 1
    fi
    echo "  -> ${topdown_out}"
}

# =====================================================================
# perf record -> flamegraph
# =====================================================================
run_perf_record() {
    local profile="$1" bin
    bin="$(performance_binary "$profile")"
    local data="${RESULTS_DIR}/perf_${profile}.data"
    local script_out="${RESULTS_DIR}/perf_${profile}.script"
    local flame="${RESULTS_DIR}/flame_${profile}.svg"

    echo ""
    echo "[perf record] profile=${profile} -> ${data}"

    pinned perf record -F "$FREQ" -g --call-graph "$CALL_GRAPH" -o "$data" -- \
        "$bin" --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
               --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --append

    perf script -i "$data" > "$script_out"
    echo "  -> ${data}"
    echo "  -> ${script_out}"

    local stackcollapse flamegraph_bin
    stackcollapse=$(command -v stackcollapse-perf.pl || true)
    flamegraph_bin=$(command -v flamegraph.pl || true)
    if [[ -n "$stackcollapse" && -n "$flamegraph_bin" ]]; then
        "$stackcollapse" "$script_out" | "$flamegraph_bin" > "$flame"
        echo "  -> ${flame}"
    else
        echo "  (flamegraph tools not on PATH - install Brendan Gregg's FlameGraph repo for ${flame})"
    fi
}

# =====================================================================
# Orchestration
# =====================================================================
for profile in "${PROFILES[@]}"; do
    ensure_binary "$profile"
    run_perf_stat "$profile"
    [[ "$SKIP_RECORD" == false ]] && run_perf_record "$profile"
done

echo ""
echo "Done."
if [[ "${#PROFILES[@]}" -eq 2 ]]; then
    echo "Compare profiles: diff ${RESULTS_DIR}/perf_stat_scalar.txt ${RESULTS_DIR}/perf_stat_optimised.txt"
fi
