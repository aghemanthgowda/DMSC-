#!/usr/bin/env bash
# Download the OpenCV LBF 68-point facemark model used for landmark mode
# (accurate EAR, yawn detection and head pose). Optional: without it the
# program runs in Haar fallback mode.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p models
OUT="models/lbfmodel.yaml"
URL="https://raw.githubusercontent.com/kurnianggoro/GSOC2017/master/data/lbfmodel.yaml"

if [ -f "$OUT" ]; then
    echo "Model already present at $OUT"
    exit 0
fi

echo "Downloading LBF facemark model to $OUT ..."
if command -v curl >/dev/null 2>&1; then
    curl -L --fail -o "$OUT" "$URL"
elif command -v wget >/dev/null 2>&1; then
    wget -O "$OUT" "$URL"
else
    echo "Neither curl nor wget found. Download manually from:"
    echo "  $URL"
    echo "and save it as $OUT"
    exit 1
fi

echo "Done. Run with:  ./build/dms --model $OUT"
