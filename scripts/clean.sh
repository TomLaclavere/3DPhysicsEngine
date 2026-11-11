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
Usage: ./scripts/clean.sh [options]

Removes the build directory. Run from the repository root.

Options:
  --build-dir DIR   Build directory to remove (default: build, or $BUILD_DIR)
  --all             Also remove CMake files generated in the current directory by an in-source
                    configure (CMakeCache.txt, CMakeFiles/, Makefile, compile_commands.json...)
  -v, --verbose     Show detailed actions
  -h, --help        Show this help message

Behaviour:
    - Refuses to remove /, /home, /root or $HOME, and asks for confirmation outside the project.

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
# Safety validation
# =====================================================================
# Normalize to absolute path
ABS_BUILD_DIR="$(realpath -m "${BUILD_DIR}" 2>/dev/null || echo "${BUILD_DIR}")"

if [[ -z "$ABS_BUILD_DIR" || "$ABS_BUILD_DIR" == "/" || "$ABS_BUILD_DIR" == "/home" || "$ABS_BUILD_DIR" == "/root" || "$ABS_BUILD_DIR" == "$HOME" ]]; then
    echo "[ERROR] Refusing to remove unsafe directory: '$ABS_BUILD_DIR'"
    exit 1
fi

# Warn if BUILD_DIR does not look like a subdirectory of the project
PROJECT_ROOT="$(realpath "$(dirname "${BASH_SOURCE[0]}")/..")"
if [[ "$ABS_BUILD_DIR" != "$PROJECT_ROOT"* ]]; then
    echo "[WARNING] '$ABS_BUILD_DIR' is outside the project directory ($PROJECT_ROOT)."
    read -p "Are you sure you want to delete it? [y/N] " confirm
    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "[CANCELLED] No changes made."
        exit 0
    fi
fi

# =====================================================================
# Perform cleaning
# =====================================================================
if [[ -d "$BUILD_DIR" ]]; then
    echo "[CLEAN] Removing build directory: $BUILD_DIR"
    rm -rf -- "$BUILD_DIR"
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
