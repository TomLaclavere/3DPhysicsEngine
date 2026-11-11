#!/usr/bin/env bash
set -euo pipefail

# Physics validation benchmarks: run each scenario of benchmarks/physics/ (solver accuracy, energy
# conservation, stability), then execute its notebook to produce the figures and the PDF report.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$ROOT/build/benchmark_physics"
SCENARIOS=("Free_Fall" "Bouncing" "Bouncing_Conservative" "Contact_Forces")
SKIP_BUILD=false
SKIP_ANALYSIS=false

# =====================================================================
# Help
# =====================================================================
usage() {
    cat <<'EOF'
Usage: ./scripts/physics_benchmark.sh [options]

Runs the physics validation benchmarks (benchmarks/physics/): each scenario writes its CSV to
benchmarks/physics/<scenario>/results/, then its benchmark_analysis.ipynb is executed, which
saves the figures and benchmark_report.pdf in the same directory. Run from the repository root.

Options:
  --scenarios LIST   space-separated scenarios
                     (default: "Free_Fall Bouncing Bouncing_Conservative Contact_Forces")
  --skip-build       reuse the existing binaries (build/benchmark_physics)
  --skip-analysis    run the benchmarks only, not the notebooks
  -h, --help         show this help

The notebooks are executed with `jupyter execute` (package nbclient) and are not modified. They
need numpy, pandas, matplotlib and scipy.
EOF
}

# =====================================================================
# Argument parsing
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --scenarios)     read -ra SCENARIOS <<< "$2"; shift 2 ;;
        --skip-build)    SKIP_BUILD=true;           shift ;;
        --skip-analysis) SKIP_ANALYSIS=true;        shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ "$SKIP_ANALYSIS" == false ]] && ! jupyter execute --help >/dev/null 2>&1; then
    echo "Error: 'jupyter execute' not found (pip install nbclient ipykernel), or use --skip-analysis." >&2
    exit 1
fi

# =====================================================================
# Build
# =====================================================================
cd "$ROOT"
if [[ "$SKIP_BUILD" == false ]]; then
    ./scripts/build.sh --build-dir "$BUILD_DIR" --build-type Release \
        -D3DPE_BUILD_TESTS=OFF -D3DPE_BUILD_EXAMPLES=OFF -D3DPE_ENABLE_COVERAGE=OFF \
        -D3DPE_BUILD_VIEWER=OFF
fi

# =====================================================================
# Benchmarks + notebooks
# =====================================================================
for scenario in "${SCENARIOS[@]}"; do
    echo ""
    echo "======================================================"
    echo " ${scenario}"
    echo "======================================================"
    "$BUILD_DIR/benchmarks/$scenario"

    if [[ "$SKIP_ANALYSIS" == false ]]; then
        # The notebook reads and writes results/ relative to its own directory
        (cd "benchmarks/physics/$scenario" && jupyter execute benchmark_analysis.ipynb)
        echo "Report: benchmarks/physics/${scenario}/results/benchmark_report.pdf"
    fi
done
