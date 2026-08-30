#!/usr/bin/env bash
set -euo pipefail

# maqao/profile_maqao.sh — vectorisation (CQA) + dynamic hotspot (Lprof) + combined report
# (ONE-View) for one representative workload, against one or both build profiles.
#
# Read benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md Part 2 first — this script assumes you
# already understand what CQA/Lprof measure and why. maqao/maqao.md is the tool-specific quick
# reference once you do.
#
# Verified against a real MAQAO 2025.1.0 install on this machine (all three commands run
# successfully end to end) — see maqao/maqao.md's worked example for real output. Two things
# that don't match what you'd guess from the module names:
#   - There is no standalone "roofline" module in this MAQAO version (`maqao --list-modules`
#     only lists analyze/cqa/lprof/madras/oneview). The "% of peak FLOP/cycle" number CQA
#     reports per loop, next to Lprof's dynamic throughput, is the roofline-relevant data —
#     ONE-View (`oneview -R1`) is the closest thing to a combined dashboard, not a dedicated
#     roofline chart.
#   - Every maqao invocation below MUST run with the repo root as the working directory:
#     benchmark.cpp hardcodes its CSV output path as the *relative* string
#     "benchmarks/performance/results", so if MAQAO (or anything else) changes directory
#     before running the binary, it crashes with a filesystem error. This bit both this
#     script and profile_malt.sh during testing — fixed here with an explicit `cd "$ROOT"`.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# shellcheck source=../profiles.sh
source "$ROOT/benchmarks/performance/profiles.sh"

RESULTS_DIR="$SCRIPT_DIR/results"

# Same representative workload as perf/profile_perf.sh — keep these in sync so perf and MAQAO
# reports describe the same run (see BENCHMARK_TOOLS_GUIDE.md §0.4).
SOLVER="Verlet"
DT="5e-5"
DUR="30"
N_OBJECTS=1

# Lua-pattern function filter for CQA (comma-separated = OR, not regex alternation — MAQAO's
# own convention, see `maqao cqa --help`). Scoped to this project's own code so the report
# isn't buried under libc/gcov noise; widen to ".*" for a first, unscoped look at a binary you
# don't know yet.
CQA_FUNCTIONS="PhysicsWorld,NarrowCollision,BroadCollision,Sphere,Plane,Vector3D,simulation"

PROFILES=("scalar" "optimised")
SKIP_BUILD=false
RUN_CQA=true
RUN_LPROF=true
RUN_ONEVIEW=true

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./benchmarks/performance/maqao/profile_maqao.sh [options]

Options:
  --profile NAME     scalar | optimised | both   (default: both)
  --solver NAME      solver for the representative run   (default: Verlet)
  --dt VALUE         timestep for the representative run  (default: 5e-5)
  --dur VALUE        simulated duration in s               (default: 30)
  --n-objects N      object count for the representative run (default: 1)
  --cqa-functions P  Lua-pattern function filter for CQA, comma-separated for OR
                     (default: "PhysicsWorld,NarrowCollision,BroadCollision,Sphere,Plane,
                     Vector3D,simulation"; use ".*" to analyse every function in the binary)
  --skip-build       reuse existing binaries (fail if missing)
  --skip-cqa         skip the static CQA pass
  --skip-lprof       skip the dynamic Lprof pass
  --skip-oneview     skip the ONE-View combined-report pass
  -h, --help         show this help

Output: benchmarks/performance/maqao/results/
  cqa_<profile>/cqa.txt     CQA static loop analysis (vectorisation ratio, VER, bottleneck)
  lprof_<profile>/          Lprof dynamic hotspot profile (functions.txt, loops.txt)
  oneview_<profile>/        ONE-View combined HTML report (RESULTS/Performance_one_html/index.html)

Note: benchmarks/Performance always writes/appends to benchmarks/performance/results/
benchmark.csv (hardcoded in benchmark.cpp) and truncates energy_drift.csv on every
invocation — running this script (via Lprof/ONE-View, which execute the binary) adds a
harmless extra row to the shared sweep CSV.
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
        --solver)        SOLVER="$2";        shift 2 ;;
        --dt)            DT="$2";            shift 2 ;;
        --dur)           DUR="$2";           shift 2 ;;
        --n-objects)     N_OBJECTS="$2";     shift 2 ;;
        --cqa-functions) CQA_FUNCTIONS="$2"; shift 2 ;;
        --skip-build)    SKIP_BUILD=true;    shift ;;
        --skip-cqa)      RUN_CQA=false;      shift ;;
        --skip-lprof)    RUN_LPROF=false;    shift ;;
        --skip-oneview)  RUN_ONEVIEW=false;  shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

# =====================================================================
# Prerequisites
# =====================================================================
command -v maqao >/dev/null 2>&1 || {
    cat >&2 <<'EOF'
Error: maqao not found on PATH.

Install, two equivalent options:
  # Arch (this machine):
  yay -S maqao-bin

  # Any distro (generic binary release, no package needed):
  curl -LO http://www.maqao.org/maqao_archive/maqao.x86_64.2025.1.0.tar.xz
  tar xf maqao.x86_64.2025.1.0.tar.xz
  export PATH="$PWD/maqao.x86_64.2025.1.0:$PATH"
  (check maqao.org/download.html for the current version before pinning it elsewhere)

See benchmarks/performance/maqao/maqao.md for the full walkthrough.
EOF
    exit 1
}

mkdir -p "$RESULTS_DIR"

# All maqao invocations below need cwd == repo root — see the header note.
cd "$ROOT"

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
        echo " Build — profile: ${profile}"
        echo "======================================================"
        build_profile "$profile" "$(build_dir_for "$profile")"
    fi
}

# =====================================================================
# CQA — static loop analysis (no execution)
# =====================================================================
run_cqa() {
    local profile="$1" bin out
    bin="$(performance_binary "$profile")"
    out="${RESULTS_DIR}/cqa_${profile}"
    mkdir -p "$out"

    echo ""
    echo "[maqao cqa] profile=${profile} functions=${CQA_FUNCTIONS}"
    maqao cqa "$bin" "fct-loops=${CQA_FUNCTIONS}" \
        > "${out}/cqa.txt" 2>&1 || { echo "cqa failed — see ${out}/cqa.txt" >&2; return 1; }
    echo "  -> ${out}/cqa.txt"
}

# =====================================================================
# Lprof — dynamic hotspot profile (executes the binary)
# =====================================================================
run_lprof() {
    local profile="$1" bin out xp
    bin="$(performance_binary "$profile")"
    out="${RESULTS_DIR}/lprof_${profile}"
    xp="${out}/run"
    mkdir -p "$out"
    rm -rf "$xp"

    echo ""
    echo "[maqao lprof] profile=${profile} solver=${SOLVER} dt=${DT} n_objects=${N_OBJECTS}"
    maqao lprof "-xp=${xp}" -- \
        "$bin" --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
               --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --append \
        > "${out}/collect.log" 2>&1 || { echo "lprof collect failed — see ${out}/collect.log" >&2; return 1; }

    maqao lprof -df "xp=${xp}" > "${out}/functions.txt" 2>&1
    maqao lprof -dl "xp=${xp}" > "${out}/loops.txt" 2>&1
    echo "  -> ${out}/{functions,loops}.txt"
}

# =====================================================================
# ONE-View — combined report (drives CQA + Lprof + topology together)
# =====================================================================
run_oneview() {
    local profile="$1" bin out
    bin="$(performance_binary "$profile")"
    out="${RESULTS_DIR}/oneview_${profile}"
    rm -rf "$out"

    echo ""
    echo "[maqao oneview] profile=${profile} solver=${SOLVER} dt=${DT} n_objects=${N_OBJECTS}"
    # -R1 = --create-report=one. The binary must be the first token after `--` (not passed via
    # --executable=, which does something different — see the header note if this ever errors
    # with "binary <something odd> does not exist", that's the symptom of getting this wrong).
    maqao oneview -R1 "-xp=${out}" --replace -- \
        "$bin" --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
               --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --append \
        > "${out}.log" 2>&1 || { echo "oneview failed — see ${out}.log" >&2; return 1; }
    echo "  -> ${out}/RESULTS/Performance_one_html/index.html"
}

# =====================================================================
# Orchestration
# =====================================================================
for profile in "${PROFILES[@]}"; do
    ensure_binary "$profile"
    [[ "$RUN_CQA"     == true ]] && run_cqa "$profile"
    [[ "$RUN_LPROF"   == true ]] && run_lprof "$profile"
    [[ "$RUN_ONEVIEW" == true ]] && run_oneview "$profile"
done

echo ""
echo "Done. Compare the scalar and optimised CQA reports' vectorisation ratio/VER for the"
echo "hottest loop — the optimised build's should be visibly higher (BENCHMARK_TOOLS_GUIDE.md §2.3)."
