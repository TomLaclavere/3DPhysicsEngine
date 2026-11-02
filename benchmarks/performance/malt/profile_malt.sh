#!/usr/bin/env bash
set -euo pipefail

# malt/profile_malt.sh — allocation profiling for one representative workload.
#
# Unlike perf/MAQAO, this only needs to run once, against the optimised profile — allocation
# *behaviour* doesn't change between build profiles (same new/push_back call sites either
# way; only how the arithmetic around them is compiled changes). See
# benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md for what MALT measures and malt/malt.md for
# the tool reference.
#
# Uses the `malt` command — a wrapper MALT installs that preloads its allocation-tracking
# library internally — rather than a raw manual LD_PRELOAD.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# shellcheck source=../profiles.sh
source "$ROOT/benchmarks/performance/profiles.sh"

cd "$ROOT"

RESULTS_DIR="$SCRIPT_DIR/results"

# Same representative workload as perf/profile_perf.sh and maqao/profile_maqao.sh — keep
# these in sync so all three reports describe the same run.
SOLVER="Verlet"
DT="5e-5"
DUR="30"
N_OBJECTS=10
PROFILE="optimised"   # see the header comment: MALT only needs one profile, not both

SKIP_BUILD=false

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./benchmarks/performance/malt/profile_malt.sh [options]

Options:
  --profile NAME     scalar | optimised   (default: optimised — see header comment)
  --solver NAME      solver for the representative run   (default: Verlet)
  --dt VALUE         timestep for the representative run  (default: 5e-5)
  --dur VALUE        simulated duration in s               (default: 30)
  --n-objects N      object count for the representative run (default: 10)
  --skip-build       reuse the existing binary instead of an (incremental) rebuild
  -h, --help         show this help

Requires MALT installed (`malt` on PATH) — see malt/malt.md's Install section, no package
for most distros, needs a from-source build.

Output: benchmarks/performance/malt/results/malt_<profile>/
  malt-Performance-<pid>.json   MALT's report, default naming
  csv/                          the benchmark binary's own CSV for this run

Plot the results with: python3 benchmarks/performance/profiling_analysis.py
EOF
}

# =====================================================================
# Argument parsing
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --profile)
            case "$2" in
                scalar|optimised) PROFILE="$2" ;;
                *) echo "Unknown profile: $2 (expected scalar|optimised)" >&2; exit 1 ;;
            esac
            shift 2 ;;
        --solver)     SOLVER="$2";       shift 2 ;;
        --dt)         DT="$2";           shift 2 ;;
        --dur)        DUR="$2";          shift 2 ;;
        --n-objects)  N_OBJECTS="$2";    shift 2 ;;
        --skip-build) SKIP_BUILD=true;   shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

# =====================================================================
# Prerequisites
# =====================================================================
command -v malt >/dev/null 2>&1 || {
    cat >&2 <<'EOF'
Error: malt not found on PATH.

No AUR package exists — build from source:
  # Arch:
  sudo pacman -S --needed base-devel cmake gcc make openssl libunwind elfutils \
                           nodejs npm nlohmann-json graphviz python curl
  # Ubuntu/Debian:
  sudo apt install cmake g++ make libssl-dev libunwind-dev libelf-dev nodejs npm \
                    nlohmann-json3-dev graphviz python3-dev curl

  git clone https://github.com/memtt/malt.git && cd malt
  mkdir build && cd build && ../configure --prefix=/usr/local
  make && sudo make install

See benchmarks/performance/malt/malt.md for the full walkthrough.
EOF
    exit 1
}

mkdir -p "$RESULTS_DIR"
CSV_DIR="$RESULTS_DIR/csv"

# =====================================================================
# Build (if needed)
# =====================================================================
ensure_binary() {
    local profile="$1" bin
    bin="$(performance_binary "$profile")"
    if [[ "$SKIP_BUILD" == true ]]; then
        [[ -x "$bin" ]] || { echo "Error: binary not found: ${bin} (and --skip-build was given)" >&2; exit 1; }
        return
    fi
    echo ""
    echo "======================================================"
    echo " Build — profile: ${profile}"
    echo "======================================================"
    build_profile "$profile" "$(build_dir_for "$profile")" > "${RESULTS_DIR}/build_${profile}.log" 2>&1 \
        || { echo "Build failed — see ${RESULTS_DIR}/build_${profile}.log" >&2; exit 1; }
}

# =====================================================================
# MALT run
# =====================================================================
run_malt() {
    local profile="$1" bin out marker
    bin="$(performance_binary "$profile")"
    out="${RESULTS_DIR}/malt_${profile}"
    mkdir -p "$out"

    echo ""
    echo "[malt] profile=${profile} solver=${SOLVER} dt=${DT} n_objects=${N_OBJECTS}"

    # `malt <program> [args...]` — args after the binary path are forwarded straight through
    # to it. MALT drops its report (malt-Performance-<pid>.json) in the current directory,
    # $ROOT — relocate it into `out` afterward. `-newer "$marker"` scopes the move to files
    # this run actually created, not stale reports from an earlier run. Older reports in `out`
    # are removed first so the plotting script always reads this run's report.
    rm -f "${out}"/malt-Performance-*.json
    marker="$(mktemp)"
    malt "$bin" \
        --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
        --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --out "$CSV_DIR"
    find "$ROOT" -maxdepth 1 -name 'malt-Performance-*.json' -newer "$marker" -exec mv -t "$out" {} +
    rm -f "$marker"
    echo "  -> ${out}/"
}

ensure_binary "$PROFILE"
run_malt "$PROFILE"

echo ""
echo "Done. Plot the results with: python3 benchmarks/performance/profiling_analysis.py"
echo "Interactive view:"
echo "  malt-webview ${RESULTS_DIR}/malt_${PROFILE}/malt-Performance-*.json"
echo "(if that fails with 'cannot open shared object file: libmalt-reader.so.1', run"
echo " 'sudo ldconfig' once after install, or prefix the command with LD_LIBRARY_PATH=/usr/local/lib)"
