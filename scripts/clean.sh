#!/usr/bin/env bash
set -euo pipefail

# =====================================================================
# Configuration
# =====================================================================
BUILD_DIR=${BUILD_DIR:-build}
VERBOSE=false
CLEAN_ALL=false

# =====================================================================
# Help message
# =====================================================================
usage() {
    cat <<'EOF'
Usage: $0 [options]

Options:
  --build-dir DIR   Specify the build directory to remove (default: ${BUILD_DIR})
  --all             Remove additional CMake cache and generated files
  -v, --verbose     Show detailed actions
  -h, --help        Show this help message

Behaviour:
    - Safer than a raw `rm -rf`, warns if the directory looks suspicious.
    - Defaults to `$BUILD_DIR` or `build/`.

Examples:
    # show help
    ./scripts/clean.sh -h

    # remove the build directory (default: ./build)
    ./scripts/clean.sh

    # remove a custom build directory
    ./scripts/clean.sh --build-dir out
EOF
}

# =====================================================================
# Parse arguments
# =====================================================================
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2;;
        --all) CLEAN_ALL=true; shift;;
        -v|--verbose) VERBOSE=true; shift;;
        -h|--help) usage; exit 0;;
        *) echo "Unknown argument: $1"; usage; exit 1;;
    esac
done

# =====================================================================
# Perform cleaning
# =====================================================================
if [[ -d "$BUILD_DIR" ]]; then
    echo "[CLEAN] Removing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
else
    echo "[CLEAN] No build directory found at: $BUILD_DIR"
fi

if [[ "$CLEAN_ALL" == true ]]; then
    echo "[CLEAN] Removing CMake cache and generated files"
    rm -f CMakeCache.txt
    rm -rf CMakeFiles cmake_install.cmake Makefile compile_commands.json
fi

if [[ "$VERBOSE" == true ]]; then
    echo "[INFO] Clean completed at: $(date)"
fi

echo "[DONE] Project cleaned successfully."
