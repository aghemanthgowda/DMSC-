# Driver Monitoring System (C++ / OpenCV)

A self-contained **driver monitoring system (DMS)** that runs entirely on a
laptop using its **built-in webcam** — no external camera, no cloud, no extra
hardware. It watches the driver in real time and raises escalating visual and
audible alerts for **drowsiness** and **distraction**, all shown in a single
live OpenCV window.

Built for a demo: one command to build, one command to run.

![mode](https://img.shields.io/badge/input-builtin_webcam-blue) ![lang](https://img.shields.io/badge/C%2B%2B-17-informational) ![deps](https://img.shields.io/badge/deps-OpenCV_4-green)

---

## What it detects

| Signal | How | Alert |
| --- | --- | --- |
| **Eye closure / microsleep** | Eye-Aspect-Ratio (EAR) from 68 facial landmarks, or eye-cascade presence in fallback mode | Alarm when eyes stay closed ≥ 1.2 s |
| **PERCLOS (fatigue)** | % of time eyes are closed over a rolling 60 s window | Warning ≥ 15 %, alarm ≥ 30 % |
| **Blink rate** | Blink counting with duration gating | Shown live (blinks/min) |
| **Yawning** | Mouth-Aspect-Ratio (MAR) sustained over time *(landmark mode)* | Warning while yawning |
| **Looking away** | Head pose (yaw/pitch via `solvePnP`), or face-offset heuristic in fallback | Warning, then alarm ≥ 2 s |
| **Driver absent** | No face detected | Alarm ≥ 1.5 s ("eyes off road") |

The system fuses these into one status — **MONITORING → CAUTION → ALARM** —
with a colour-coded banner, a red pulsing border, and a rate-limited beep.

## Two detection modes (automatic)

- **Landmark mode (recommended)** — 68-point facial landmarks give an accurate
  EAR, real yawn detection and head-pose estimation. Needs a one-time model
  download (see below).
- **Haar fallback mode** — if no landmark model is present, the system uses
  OpenCV's bundled Haar cascades for the face and eyes. Zero downloads, works
  offline out of the box. Slightly less precise but fully functional.

The program picks the best available mode at startup and tells you which one
it is using.

---

## Requirements

- A C++17 compiler, CMake ≥ 3.16
- **OpenCV 4** with the `face` (contrib) module for landmark mode

Install OpenCV:

```bash
# Ubuntu / Debian
sudo apt-get install -y libopencv-dev

# macOS (Homebrew) — includes opencv_contrib
brew install opencv
```

## Build

```bash
./scripts/build.sh
# or manually:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

This produces `build/dms`.

## Run

**Haar fallback mode** (no download, works immediately):

```bash
./build/dms
```

**Landmark mode** (accurate EAR + yawns + head pose):

```bash
./scripts/download_facemark_model.sh     # one-time, ~54 MB -> models/lbfmodel.yaml
./build/dms --model models/lbfmodel.yaml
```

> If `models/lbfmodel.yaml` exists, `./build/dms` picks it up automatically.

Press **`q`** or **`ESC`** in the window to quit.

### Command-line options

```
--camera <n>        camera index (default 0)
--model <path>      LBF facemark model (lbfmodel.yaml) for landmark mode
--cascades <dir>    Haar cascade directory (auto-detected if omitted)
--no-mirror         do not mirror the view
--no-beep           disable the audible alarm
-h, --help          show this help
```

---

## Demo tips

- Sit at a normal arm's-length distance in reasonable lighting.
- **Close your eyes** for ~1.5 s → drowsiness alarm + beep.
- **Turn your head** left/right or **look down** at a "phone" → distraction alert.
- **Step out of frame** → "driver not detected" alarm.
- Fake a **yawn** (landmark mode) → yawn counter ticks up.

Thresholds live in [`include/dms/Config.hpp`](include/dms/Config.hpp) and can be
tuned for your camera and lighting.

## How it works

```
webcam ─▶ FaceTracker ─▶ FaceObservation ─┬▶ DrowsinessDetector ─┐
         (Haar / LBF)                      └▶ DistractionDetector ─┴▶ AlertManager ─▶ Dashboard ─▶ window
```

| File | Responsibility |
| --- | --- |
| `src/FaceTracker.cpp` | Face + eye detection, landmark fitting, EAR/MAR, head pose |
| `src/DrowsinessDetector.cpp` | Eye-closure timing, PERCLOS, blinks, yawns |
| `src/DistractionDetector.cpp` | Head-pose / face-offset attention logic |
| `src/AlertManager.cpp` | Fuses signals, escalates, drives the beep |
| `src/Dashboard.cpp` | Draws the annotated video + status panel |
| `src/main.cpp` | Camera loop, CLI, FPS |

## Notes & limitations

This is a **demo / educational** project, not a certified safety system. Haar
and LBF detectors are lightweight and can struggle with poor lighting, extreme
angles, or heavy occlusion. Numbers like EAR and PERCLOS thresholds are sensible
defaults, not calibrated per-driver values.
