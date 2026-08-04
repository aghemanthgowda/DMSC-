# Models & Detection Capabilities (honest status)

This prototype **never fakes a detection**. Features that need a model they don't
have report `off (no model)` or `UNKNOWN` instead of guessing. This file lists
what is implemented, what each feature needs, and how to plug a model in.

## Current status

| Feature | Backend now | Needs | State without model |
|---|---|---|---|
| Face + 68 landmarks | **dlib** (working) | — | works |
| Head pose, EAR, PERCLOS, blinks, yawn | landmarks (working) | — | works |
| Eyeball / iris (approx) | OpenCV pupil blob (working) | — | works |
| Attention / distraction | head pose + gaze (working) | — | works |
| Hand activity (approx) | OpenCV skin heuristic (working) | — | "near-face / clear" |
| **Phone** | **none** | YOLO object model via **ONNX Runtime** | `off (no model)` → `NO PHONE` |
| **Smoking** | none | **custom cigarette** model (not in COCO) | `UNKNOWN` |
| **Seat belt** | none | **custom seat-belt** model / segmentation | `UNKNOWN` |
| Hand *landmarks* (21-pt) | none | ONNX/MediaPipe hand model (C++) | skin heuristic only |

## Why phone/smoking/seat belt are not "on" yet

- **Phone**: needs a real object detector. The architecture is wired (`ObjectDetector`,
  ONNX-gated) with a configurable class map. **Standard COCO YOLO has "cell phone"
  (class 67)** — so phone detection is achievable by dropping in a YOLOv8n ONNX
  model. Until then the detector honestly reports `NO PHONE`. **Head pose / looking
  away NEVER imply a phone** (that false-positive path was removed).
- **Smoking**: **COCO has no cigarette class.** A generic YOLO cannot detect
  cigarettes reliably, so this stays `UNKNOWN` until a **custom-trained cigarette
  model** is provided. The multi-cue logic (cigarette near mouth + hand near mouth
  + temporal) is implemented in `SmokingDetector` and activates only with a real
  cigarette detection.
- **Seat belt**: **COCO has no seat-belt class.** `SeatBeltDetector` computes the
  torso ROI and returns `UNKNOWN` until a **custom seat-belt detector/segmentation
  model** is provided. It never reports "not worn" just because the belt isn't
  visible.

## Class mapping (config/config.json)

```json
"class_id_phone": 67,     // COCO cell phone
"class_id_person": 0,     // COCO person
"class_id_cigarette": -1, // -1 = no model; set to your custom class id
"class_id_seatbelt": -1,  // -1 = no model; set to your custom class id
"yolo_confidence": 0.45,
"nms_threshold": 0.45
```

`-1` means "no such class in the loaded model" → the feature reports UNKNOWN.
Never set an invalid COCO id to fake a class.

## How to enable phone detection (YOLO via OpenCV DNN — NO ONNX Runtime)

Phone detection runs the YOLO model through **OpenCV's own `cv::dnn` module**, so
there is **no ONNX Runtime dependency** — if you built the project, you already
have everything except the model file.

1. Download a model (COCO, ~28 MB):
   ```bash
   ./scripts/download_yolo_model.sh      # -> models/yolov5s.onnx
   ```
   Or a faster one: `pip install ultralytics && yolo export model=yolov8n.pt format=onnx`
   then put `yolov8n.onnx` in `models/`. **Both YOLOv5 and YOLOv8 formats are
   supported** (auto-detected).
2. Rebuild and run — the model in `models/` is auto-detected (or pass
   `--phone-model models/yolov5s.onnx`).
3. `ObjectDetector` runs the model every N frames (`phone_detect_every_n_frames`),
   applies confidence + NMS, temporal-confirms, fuses with a nearby hand, and
   drives the phone state machine `NO_PHONE → POSSIBLE → DETECTED →
   USAGE_CONFIRMED`. Verified end-to-end (detects COCO objects via cv::dnn).

**Performance note:** YOLOv5s on a laptop CPU is ~100–200 ms/inference, so it runs
every 4th frame by default. Use **yolov8n** for higher FPS, or raise
`phone_detect_every_n_frames`. On the i.MX 93 the Ethos-U NPU can accelerate this.

## For cigarette / seat belt

Train (or obtain) a YOLO model that includes those classes, set the matching
`class_id_*` in the config, and the existing `SmokingDetector` / `SeatBeltDetector`
logic will consume the detections. No code changes required beyond the model + ids.
