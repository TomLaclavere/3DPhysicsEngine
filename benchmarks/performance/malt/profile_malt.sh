#!/usr/bin/env bash
set -euo pipefail

# malt/profile_malt.sh — allocation profiling for one representative workload.
#
# Unlike perf/MAQAO, this only needs to run once, against the optimised profile — allocation
# *behaviour* doesn't change between build profiles (same new/push_back call sites either
# way; only how the arithmetic around them is compiled changes). See
# benchmarks/performance/BENCHMARK_TOOLS_GUIDE.md §3.5 for why.
#
# Read BENCHMARK_TOOLS_GUIDE.md Part 3 first — this script assumes you already understand
# what MALT measures and why. malt/malt.md is the tool-specific quick reference once you do.
#
# Uses the `malt` command — a wrapper MALT installs that preloads its allocation-tracking
# library internally, confirmed against MALT's official README (github.com/memtt/malt) —
# rather than a raw manual LD_PRELOAD. Still not run for real in this repo (MALT isn't
# installed here), so double-check `malt --help` on your build if anything below errors.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# shellcheck source=../profiles.sh
source "$ROOT/benchmarks/performance/profiles.sh"

# benchmark.cpp hardcodes its CSV output path as the *relative* string
# "benchmarks/performance/results" — everything below must run with cwd == repo root or the
# binary crashes trying to create that path (confirmed while testing this script's earlier
# `cd "$out"` approach against a real maqao run hitting the same issue).
cd "$ROOT"

RESULTS_DIR="$SCRIPT_DIR/results"

# Same representative workload as perf/profile_perf.sh and maqao/profile_maqao.sh — keep
# these in sync so all three reports describe the same run.
SOLVER="Verlet"
DT="5e-5"
DUR="30"
N_OBJECTS=1
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
  --n-objects N      object count for the representative run (default: 1)
  --skip-build       reuse existing binary (fail if missing)
  -h, --help         show this help

Requires MALT installed (`malt` on PATH) — see malt/malt.md's Install section, no package
for most distros, needs a from-source build.

Output: benchmarks/performance/malt/results/malt_<profile>/
  malt-Performance-<pid>.json   MALT's report, default naming

Note: benchmarks/Performance always writes/appends to benchmarks/performance/results/
benchmark.csv (hardcoded in benchmark.cpp) and truncates energy_drift.csv on every
invocation — running this script adds a harmless extra row to the shared sweep CSV.
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
    # to it (matching MALT's documented `malt {YOUR_PROGRAM} [OPTIONS]`). Must run from $ROOT
    # (see header note), so the report (malt-Performance-<pid>.json, dropped in cwd by
    # default) lands in $ROOT too — relocate it into `out` afterward. `-newer "$marker"` scopes
    # the move to files this run actually created, not stale reports from an earlier run.
    marker="$(mktemp)"
    malt "$bin" \
        --solver "$SOLVER" --dt "$DT" --dur "$DUR" \
        --n_warmup 1 --n_runs 1 --n_objects "$N_OBJECTS" --append
    find "$ROOT" -maxdepth 1 -name 'malt-Performance-*.json' -newer "$marker" -exec mv -t "$out" {} +
    rm -f "$marker"
    echo "  -> ${out}/"
}

ensure_binary "$PROFILE"
run_malt "$PROFILE"

echo ""
echo "Done. View with:"
echo "  malt-webview ${RESULTS_DIR}/malt_${PROFILE}/malt-Performance-*.json"
echo "(if that fails with 'cannot open shared object file: libmalt-reader.so.1', run"
echo " 'sudo ldconfig' once after install, or prefix the command with LD_LIBRARY_PATH=/usr/local/lib)"
