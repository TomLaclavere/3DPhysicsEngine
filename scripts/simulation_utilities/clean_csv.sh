#!/usr/bin/env bash
set -euo pipefail

shopt -s nullglob

EXAMPLE_DIR="examples"
OUTPUT_DIR="output/CSV"

# Cleaning CSV in examples
echo "-> Cleaning CSV in examples"

for example_dir in "$EXAMPLE_DIR"/*/CSV; do
    [[ -d "$example_dir" ]] || continue

    echo "[CLEAN] Removing CSV directory: $example_dir"
    rm -rf -- "$example_dir"
done

# Cleaning CSV in output
echo "-> Cleaning CSV in output"

if [[ -d "$OUTPUT_DIR" ]]; then
    echo "[CLEAN] Removing CSV directory: $OUTPUT_DIR"
    rm -rf -- "$OUTPUT_DIR"
fi

echo "[DONE] CSV cleaned successfully."