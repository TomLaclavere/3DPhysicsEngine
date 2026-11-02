#!/usr/bin/env bash
set -uo pipefail

# scripts/full_report.sh : run the benchmark sweep, then the profiling pipeline, one after the
# other (never in parallel: profilers slow the program down, which would corrupt the timings).

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./scripts/full_report.sh

Runs scripts/benchmark.sh (~40 min), then scripts/profile.sh (~3 min), with their default
settings, and prints where the two reports are. The profiling step runs even if the sweep
failed. For other settings, run the two scripts directly.

Do not build, clean or test the project while it runs (scripts/clean.sh, tests.sh and
coverage.sh delete or rebuild build/, which removes the binaries being measured).

Output:
  benchmarks/performance/results/benchmark_report.pdf               benchmark sweep
  benchmarks/performance/profiling_report/profiling_report.pdf      profiling
EOF
}

case "${1:-}" in
    -h|--help) usage; exit 0 ;;
    "") ;;
    *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
esac

# =====================================================================
# Orchestration
# =====================================================================
"$SCRIPT_DIR/benchmark.sh"
bench_status=$?

"$SCRIPT_DIR/profile.sh"
profile_status=$?

echo ""
echo "######################################################"
echo "# Full report"
echo "######################################################"
echo "  benchmark sweep: $([[ $bench_status -eq 0 ]] && echo OK || echo "FAILED (exit $bench_status)")"
echo "  profiling:       $([[ $profile_status -eq 0 ]] && echo OK || echo "FAILED (exit $profile_status)")"
echo ""
echo "  benchmarks/performance/results/benchmark_report.pdf"
echo "  benchmarks/performance/profiling_report/profiling_report.pdf"

(( bench_status == 0 && profile_status == 0 ))
