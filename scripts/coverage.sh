#!/usr/bin/env bash
set -euo pipefail

# Defaults
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
FORMAT="console"    # html | console
OPEN_HTML=0
CMAKE_ARGS=()

usage() {
    cat <<EOF
Usage: $0 [options] [cmake-args...]
Options:
  --build-dir DIR      Build dir (default: ${BUILD_DIR})
  --build-type TYPE    Build type (default: ${BUILD_TYPE})
  --format html|console  Output format (default: console)
  --open               If html, try to open the report in default browser
  -h,--help            Show this help
Any '-D' CMake-style options provided will be forwarded to CMake.
Examples:
  ./scripts/coverage.sh --format html -DUSE_CLANG=ON
  ./scripts/coverage.sh --format console
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --build-type) BUILD_TYPE="$2"; shift 2;;
        --format) FORMAT="$2"; shift 2;;
        --open) OPEN_HTML=1; shift;;
        -h|--help) usage; exit 0;;
        -*) CMAKE_ARGS+=("$1"); shift;;
        *) echo "Unknown arg: $1"; usage; exit 1;;
    esac
done

# Ensure coverage is enabled during configure so coverage target exists.
# If your CMake adds coverage only when -DCOVERAGE=ON, we set it here unless user overrides.
HAS_COVERAGE_FLAG=0
for a in "${CMAKE_ARGS[@]}"; do
    if [[ "$a" == "-DCOVERAGE="* || "$a" == "-DCOVERAGE" ]]; then
        HAS_COVERAGE_FLAG=1
        break
    fi
done
if [[ $HAS_COVERAGE_FLAG -eq 0 ]]; then
    CMAKE_ARGS+=("-DCOVERAGE=ON")
fi

echo "Building with coverage enabled. CMake args: ${CMAKE_ARGS[*]}"
./scripts/build.sh --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" "${CMAKE_ARGS[@]}"

if [[ "$FORMAT" == "html" ]]; then
    echo "Generating HTML coverage report..."
    cmake --build "$BUILD_DIR" --target coverage
    REPORT="$BUILD_DIR/coverage_report/coverage_report.html"
    if [[ -f "$REPORT" ]]; then
        echo "Coverage HTML report generated: $REPORT"
        if [[ $OPEN_HTML -eq 1 ]]; then
            if command -v xdg-open >/dev/null 2>&1; then xdg-open "$REPORT" >/dev/null 2>&1 || true; fi
            if command -v open >/dev/null 2>&1; then open "$REPORT" >/dev/null 2>&1 || true; fi
        fi
    else
        echo "Error: HTML coverage report not found at $REPORT"
        exit 2
    fi
elif [[ "$FORMAT" == "console" ]]; then
    echo "Generating console coverage report..."
    cmake --build "$BUILD_DIR" --target coverage-console
else
    echo "Unknown format: $FORMAT"
    usage
    exit 1
fi
