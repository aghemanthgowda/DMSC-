#!/usr/bin/env bash
# Configure and build the Driver Monitoring System.
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD_DIR="build"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --parallel

echo
echo "Built: $BUILD_DIR/dms"
echo "Run with:  ./$BUILD_DIR/dms            (Haar mode)"
echo "       or:  ./$BUILD_DIR/dms --model models/lbfmodel.yaml   (landmark mode)"
