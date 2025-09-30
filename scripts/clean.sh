#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${BUILD_DIR:-build}

echo "Removing $BUILD_DIR ..."
rm -rf "$BUILD_DIR"
