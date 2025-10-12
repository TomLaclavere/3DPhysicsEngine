#!/usr/bin/env bash
set -euo pipefail

# =====================================================================
# Defaults
# =====================================================================
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
FORMAT="console"       # html | console
OPEN_HTML=false
CMAKE_ARGS=()
VERBOSE=false

# =====================================================================
# Help message
# =====================================================================
usage() {
    cat <<'EOF'
Usage: $0 [options] [cmake-args...]

Options:
  --build-dir DIR       Build directory (default: ${BUILD_DIR})
  --build-type TYPE     Build type (Debug|Release) (default: ${BUILD_TYPE})
  --format html|console Output format (default: console)
  --open                If html, open report in default browser
  -v, --verbose         Enable verbose output
  -h, --help            Show this help

Behavior:
  - Coverage is automatically enabled unless explicitly disabled with `-D3DPE_ENABLE_COVERAGE=OFF`.
  - HTML reports are generated under: `build/coverage_report/coverage_report.html`.
  - The script will try to open the report automatically on Linux (`xdg-open`) or macOS (`open`) if `--open` is provided.

Examples:
  show help
  ./scripts/coverage.sh -h

  generate console report (default)
  ./scripts/coverage.sh --format console

  generate HTML report
  ./scripts/coverage.sh --format html

  generate HTML report and open it in your default browser
  ./scripts/coverage.sh --format html --open

  add compile-time flags (example: build with Clang)
  ./scripts/coverage.sh --format html -D3DPE_USE_CLANG=ON --open
EOF
}

# =====================================================================
# Parse arguments
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --format) FORMAT="$2"; shift 2;;
        --open) OPEN_HTML=true; shift;;
        -v|--verbose) VERBOSE=true; shift;;
        -h|--help) usage; exit 0;;
        -*) CMAKE_ARGS+=("$1"); shift;;
        *) echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

[[ "$VERBOSE" == true ]] && echo "[COVERAGE] Build dir: $BUILD_DIR, Build type: $BUILD_TYPE, Format: $FORMAT"

# =====================================================================
# Ensure coverage is enabled unless explicitly disabled
# =====================================================================
HAS_COVERAGE_FLAG=false
for a in "${CMAKE_ARGS[@]}"; do
    if [[ "$a" == "-DCOVERAGE="* ]] || [[ "$a" == "-DCOVERAGE" ]]; then
        HAS_COVERAGE_FLAG=true
        break
    fi
done
$HAS_COVERAGE_FLAG || CMAKE_ARGS+=("-DCOVERAGE=ON")

# =====================================================================
# Build project with coverage enabled
# =====================================================================
[[ "$VERBOSE" == true ]] && echo "[COVERAGE] Building project with coverage..."
./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"

# =====================================================================
# Generate coverage report
# =====================================================================
if [[ "$FORMAT" == "html" ]]; then
    echo "[COVERAGE] Generating HTML report..."
    cmake --build "$BUILD_DIR" --target coverage
    REPORT="$BUILD_DIR/coverage_report/coverage_report.html"
    if [[ -f "$REPORT" ]]; then
        echo "[COVERAGE] HTML report generated: $REPORT"
        if [[ "$OPEN_HTML" == true ]]; then
            if command -v xdg-open >/dev/null 2>&1; then xdg-open "$REPORT" >/dev/null 2>&1 || true; fi
            if command -v open >/dev/null 2>&1; then open "$REPORT" >/dev/null 2>&1 || true; fi
        fi
    else
        echo "[ERROR] HTML coverage report not found at $REPORT"
        exit 2
    fi
elif [[ "$FORMAT" == "console" ]]; then
    echo "[COVERAGE] Generating console report..."
    cmake --build "$BUILD_DIR" --target coverage-console
else
    echo "[ERROR] Unknown format: $FORMAT"
    usage
    exit 1
fi
