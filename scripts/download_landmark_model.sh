#!/usr/bin/env bash
# Download the dlib 68-point facial-landmark model for accurate eye-closure,
# yawn and head-pose detection. Served uncompressed (no bunzip2 needed).
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p models
OUT="models/shape_predictor_68_face_landmarks.dat"
URL="https://raw.githubusercontent.com/italojs/facial-landmarks-recognition/master/shape_predictor_68_face_landmarks.dat"

if [ -f "$OUT" ]; then
    echo "Model already present at $OUT"
    exit 0
fi

echo "Downloading dlib 68-point model (~96 MB) to $OUT ..."
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

echo "Done. It will be picked up automatically by ./build/dms."
