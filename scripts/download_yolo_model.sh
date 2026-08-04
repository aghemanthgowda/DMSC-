#!/usr/bin/env bash
# Download a YOLO ONNX model (COCO classes incl. cell phone = 67) for phone /
# object detection via OpenCV's DNN module. No ONNX Runtime needed.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p models
OUT="models/yolov5s.onnx"
URL="https://raw.githubusercontent.com/doleron/yolov5-opencv-cpp-python/main/config_files/yolov5s.onnx"

if [ -f "$OUT" ]; then
    echo "Model already present at $OUT"
    exit 0
fi

echo "Downloading YOLOv5s ONNX (~28 MB) to $OUT ..."
if command -v curl >/dev/null 2>&1; then curl -L --fail -o "$OUT" "$URL";
elif command -v wget >/dev/null 2>&1; then wget -O "$OUT" "$URL";
else echo "Need curl or wget. URL: $URL"; exit 1; fi

echo "Done. It is auto-detected on startup (phone detection active)."
echo "For a faster model: pip install ultralytics && yolo export model=yolov8n.pt format=onnx"
echo "then put yolov8n.onnx in models/ (both formats are supported)."
