#!/usr/bin/env bash
set -uo pipefail

# scripts/profile.sh : run the perf + MAQAO + MALT profiling scripts in one command.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

RUN_PERF=true
RUN_MAQAO=true
RUN_MALT=true
PROFILE_ARG=()   # forwarded to perf/MAQAO's --profile; MALT only ever runs optimised

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./scripts/profile.sh [options]

Runs, in order: perf/profile_perf.sh, maqao/profile_maqao.sh, malt/profile_malt.sh.
A tool that isn't installed fails that one step (with install instructions) without
stopping the others - see the summary printed at the end.

Options:
  --profile NAME   scalar | optimised | both  - forwarded to perf/MAQAO (default: both)
  --skip-perf      don't run the perf step
  --skip-maqao     don't run the MAQAO step
  --skip-malt      don't run the MALT step
  -h, --help       show this help

MALT needs the `malt` command installed (see benchmarks/performance/malt/malt.md) - if it
isn't found, that step fails with instructions, same as a missing perf/maqao.

Each script also takes its own --solver/--dt/--dur/--n-objects etc. - run it directly
(e.g. ./benchmarks/performance/perf/profile_perf.sh --help) for finer control than this
orchestrator offers.
EOF
}

# =====================================================================
# Argument parsing
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --profile)     PROFILE_ARG=(--profile "$2"); shift 2 ;;
        --skip-perf)   RUN_PERF=false;   shift ;;
        --skip-maqao)  RUN_MAQAO=false;  shift ;;
        --skip-malt)   RUN_MALT=false;   shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

# =====================================================================
# Orchestration
# =====================================================================
STEPS=()   # each entry: "<name>: OK" or "<name>: FAILED (exit N)" or "<name>: skipped"

run_step() {
    local name="$1" script="$2"; shift 2
    echo ""
    echo "######################################################"
    echo "# $name"
    echo "######################################################"
    if "$script" "$@"; then
        STEPS+=("${name}: OK")
    else
        STEPS+=("${name}: FAILED (exit $?)")
    fi
}

if [[ "$RUN_PERF" == true ]]; then
    run_step "perf" "$ROOT/benchmarks/performance/perf/profile_perf.sh" "${PROFILE_ARG[@]}"
else
    STEPS+=("perf: skipped")
fi

if [[ "$RUN_MAQAO" == true ]]; then
    run_step "MAQAO" "$ROOT/benchmarks/performance/maqao/profile_maqao.sh" "${PROFILE_ARG[@]}"
else
    STEPS+=("MAQAO: skipped")
fi

if [[ "$RUN_MALT" == true ]]; then
    run_step "MALT" "$ROOT/benchmarks/performance/malt/profile_malt.sh"
else
    STEPS+=("MALT: skipped")
fi

echo ""
echo "######################################################"
echo "# Summary"
echo "######################################################"
printf '  %s\n' "${STEPS[@]}"

# Exit non-zero if anything actually failed (not just skipped), so this is CI-friendly.
if printf '%s\n' "${STEPS[@]}" | grep -q "FAILED"; then
    exit 1
fi
