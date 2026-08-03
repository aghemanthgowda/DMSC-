# Driver Monitoring System (C++ / OpenCV / dlib)

A real-time, AI-based **Driver Monitoring System (DMS)** that runs entirely on a
laptop using its **built-in webcam** — no external camera, no cloud, no extra
hardware. It fuses face, eyes, blinks, PERCLOS, yawning, head pose and
(approximate) gaze through temporal filtering and a central risk engine into a
single stable driver-state assessment, shown on a professional live dashboard.

> **Research / demonstration prototype — NOT an automotive safety-certified or
> production system.** Metrics such as EAR/PERCLOS thresholds are sensible
> defaults, not calibrated clinical values, and gaze is an *approximate* visual
> cue, not exact eye tracking.

## AIS-184 scope & regulatory disclaimer

This project is an **AIS-184 requirement-oriented DDAWS research/demo prototype
— designed *with reference to* AIS-184 requirements**. AIS-184 is India's
*Driver Drowsiness and Attention Warning System (DDAWS)* standard (ARAI / AISC,
under MoRTH; reported to apply to vehicle categories M2, M3, N2, N3).

**This software has NOT been type-approved or officially certified for AIS-184
compliance**, and no claim of legal/regulatory compliance is made. The full
official AIS-184 PDF could not be retrieved automatically (the ARAI host returned
HTTP 403), so all clause-level specifics are marked **`VERIFY-OFFICIAL`** in the
docs and must be checked against the official document from ARAI.

Regulatory documentation:
- [`docs/AIS184_TRACEABILITY.md`](docs/AIS184_TRACEABILITY.md) — requirement matrix + statuses
- [`docs/AIS184_COMPLIANCE_REPORT.md`](docs/AIS184_COMPLIANCE_REPORT.md) — scope, limitations, disclaimer
- [`docs/TEST_PLAN.md`](docs/TEST_PLAN.md) — 23-case manual test plan
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — modules, pipeline, core vs extended

**Privacy:** all processing is local; frames are discarded after analysis; there
is **no face-recognition / biometric identity database**; `--privacy` (or
`privacy_mode`) disables all disk logging. Logs contain metadata only, never
images.

![input](https://img.shields.io/badge/input-builtin_webcam-blue) ![lang](https://img.shields.io/badge/C%2B%2B-17-informational) ![cv](https://img.shields.io/badge/OpenCV-4-green) ![landmarks](https://img.shields.io/badge/landmarks-dlib_68-orange)

---

## What it does

| Subsystem | Signals | Output |
|---|---|---|
| **Face** | dlib HOG detection, confidence, multi-face → primary driver, face-loss grace | bounding box + confidence, "driver absent" (debounced) |
| **Landmarks** | 68 points (eyes, lids, nose, mouth, jaw) | contours + per-region metrics |
| **Eyes / EAR** | left, right, average EAR, **auto-calibrated** threshold, smoothing | open / blink / long-blink / prolonged closure |
| **Blinks** | count, rate/min, duration, long-blink count | fatigue cue |
| **PERCLOS** | rolling % eyes-closed over a configurable window | fatigue cue |
| **Drowsiness** | EAR + PERCLOS + closure + blinks + yawns (temporal) | **ALERT / POSSIBLE / DROWSY / CRITICAL** + 0–100 score |
| **Yawning** | Mouth-Aspect-Ratio (MAR), duration-gated | yawn count + "yawning" |
| **Head pose** | yaw / pitch / roll via `solvePnP`, EMA-smoothed | forward / left / right / up / down / tilted |
| **Gaze (approx)** | pupil offset in each eye box | forward / left / right / up / down |
| **Phone** *(optional)* | YOLO via ONNX Runtime, low-FPS + temporal confirm | phone detected (Phase 2) |
| **Distraction** | head + gaze + phone (temporal) | **ATTENTIVE / BRIEF / DISTRACTED / HIGHLY** + 0–100 |
| **Risk engine** | weighted fusion + **state machine (hysteresis)** | 0–100 risk, driver state, alert level |
| **Alerts** | level 0–3, cooldown, escalation, recovery | visual banner + audible alarm |
| **Logging** | events + timestamps | in-app timeline + `logs/events.csv` |

**Driver states:** SAFE · ATTENTION REQUIRED · DROWSY · DISTRACTED · PHONE USAGE
· HIGH RISK · CRITICAL.

---

## Architecture

Clean, single-responsibility modules under `include/dms` + `src` (no logic dumped
in `main.cpp`):

```
Camera/main.cpp  capture loop, timing, keys, wiring
ConfigManager    load/save config/config.json (via OpenCV FileStorage)
FaceTracker      face detection + 68 landmarks + EAR/MAR/head-pose + confidence
GazeEstimator    approximate pupil-offset gaze
DrowsinessDetector  EAR smoothing/calibration, PERCLOS, blinks, yawns, 4 levels
DistractionDetector head-pose + gaze + phone fusion, 4 levels
ObjectDetector   optional YOLO/ONNX phone detection (temporal-confirmed)
RiskEngine       weighted fusion → 0–100 + DriverState + hysteresis state machine
AlertManager     alert levels, cooldown, escalation, audible alarm
EventLogger      rolling timeline + CSV
Dashboard        professional OpenCV dashboard (feed + panel + gauge + timeline)
```

Data flow each frame:
`camera → FaceTracker → {Gaze, Drowsiness, Distraction(+Phone)} → RiskEngine → AlertManager → EventLogger → Dashboard`.

### Folder structure

```
dmsc-/
├── CMakeLists.txt
├── config/config.json          # all thresholds (documented below)
├── data/                       # bundled Haar cascades (fallback)
├── include/dms/*.hpp
├── src/*.cpp
├── scripts/                    # build + model-download helpers (.sh / .ps1)
├── models/                     # downloaded models (git-ignored)
└── logs/events.csv             # runtime event log (git-ignored)
```

---

## Requirements

- C++17 compiler, CMake ≥ 3.16
- **OpenCV 4** (base modules)
- **dlib** (recommended — enables the accurate 68-point backend)
- *(optional, Phase 2)* ONNX Runtime for phone detection

The 68-point landmark backend needs a model file (`shape_predictor_68_face_landmarks.dat`).
Without dlib the app still runs in a reduced Haar mode.

## Build & run — Windows (PowerShell)

**Prerequisites:** Visual Studio 2022 ("Desktop development with C++") + CMake,
and OpenCV prebuilt extracted to `C:\opencv\opencv\build`.

dlib provides the 68-point landmarks. There are two ways to get it; **Option A
(compile from source) is the most reliable** — no vcpkg, no toolchain file.

### Option A — dlib from source (recommended)

```powershell
cd C:\Users\<you>\DMSC-
git clone https://github.com/davisking/dlib.git       # dlib source into ./dlib
./scripts/download_landmark_model.ps1                 # 68-point model (~96 MB)

Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
$env:Path += ";C:\opencv\opencv\build\x64\vc16\bin"
cmake -S . -B build -A x64 -DOpenCV_DIR=C:\opencv\opencv\build
cmake --build build --config Release
.\build\Release\dms.exe
```

CMake automatically compiles the `./dlib` source into the build (the first build
is a few minutes longer). No vcpkg and no `-DCMAKE_TOOLCHAIN_FILE` needed.

### Option B — dlib via vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install dlib:x64-windows
./scripts/download_landmark_model.ps1

Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
$env:Path += ";C:\opencv\opencv\build\x64\vc16\bin"
cmake -S . -B build -A x64 -DOpenCV_DIR=C:\opencv\opencv\build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
.\build\Release\dms.exe
```

**Either way**, the configure log must show `DMS: dlib 68-point landmark mode
ENABLED` and startup must print `Detection backend: dlib 68-point landmarks`. If
you instead see a red **HAAR MODE** banner, dlib was not compiled in — delete the
`build` folder and re-run the `cmake -S . -B build ...` line.

## Build & run — Linux / macOS

```bash
sudo apt-get install -y libopencv-dev libdlib-dev libblas-dev liblapack-dev   # or: brew install opencv dlib
./scripts/download_landmark_model.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel
./build/dms
```

### Keys (while running)

| Key | Action |
|---|---|
| `q` / `ESC` | quit |
| `d` | toggle **developer mode** (inference ms, risk contributions) |
| `c` | restart **calibration** ("look straight") |

### Command-line options

```
--config <path>     config JSON (default config/config.json)
--camera <n>        camera index
--model <path>      landmark model: dlib .dat or OpenCV .yaml (auto-detected)
--phone-model <p>   YOLO .onnx for phone detection (needs ONNX build)
--cascades <dir>    Haar cascade dir (auto-detected if omitted)
--dev               start in developer mode
--no-mirror | --no-beep | --no-log
```

---

## Configuration & thresholds

All thresholds live in `config/config.json` (auto-created on first run). Every
value is documented in `include/dms/Config.hpp`. Key ones:

| Key | Meaning |
|---|---|
| `ear_threshold`, `ear_close_ratio` | fixed / adaptive eyes-closed threshold |
| `eye_closed_drowsy_seconds`, `eye_closed_alarm_seconds` | closure → DROWSY / CRITICAL |
| `perclos_window_seconds`, `perclos_warn`, `perclos_alarm` | PERCLOS window + levels |
| `long_blink_seconds` | blink duration counted as a "long blink" |
| `mar_threshold`, `yawn_min_seconds` | mouth-open level + min duration for a yawn |
| `head_away_yaw_degrees`, `head_down_pitch_degrees`, `head_away_duration_seconds` | head-pose distraction |
| `gaze_off_threshold` | pupil offset counted as looking away |
| `phone_confidence_threshold`, `phone_confirm_frames`, `phone_detect_every_n_frames` | phone detection |
| `face_lost_grace_seconds`, `no_face_alarm_seconds` | presence debounce / absent alarm |
| `w_drowsiness`, `w_distraction`, `w_phone`, `w_yawn` | **risk-fusion weights** |
| `risk_warning_threshold`, `risk_high_threshold`, `risk_critical_threshold` | risk → state bands |
| `state_enter_seconds`, `state_exit_seconds` | state-machine escalate / recover persistence |
| `alert_cooldown_seconds` | min gap between repeat audible alerts |

To tune: edit the JSON and restart. Raising a weight increases that signal's
influence on the overall risk; lowering an "enter"/"exit" second makes the state
react faster / recover faster.

## Calibration

At startup (and whenever you press `c`) the app shows *"Please look straight at
the camera"* for `calibration_seconds`. During this window the EAR baseline is
learned per-driver and alerts are suppressed. Calibration is optional — it simply
improves robustness across faces, glasses and camera positions.

## Developer mode

Press `d` (or start with `--dev`) to overlay inference time and the individual
risk contributions (drowsiness / distraction / phone / yawn) — useful for tuning.

## Performance notes

Targets 20–30 FPS on a normal laptop. Optimizations in place:
- dlib HOG face detection runs on a **half-size frame** and only **every 3rd
  frame** (landmarks still every frame) — the single biggest speedup.
- EAR/MAR moving averages + head-pose EMA avoid per-frame jitter without cost.
- Phone/YOLO (Phase 2) runs at ~5–10 FPS with box reuse + temporal confirmation.
- Capture is 640×480 by default (configurable).

## Troubleshooting

| Symptom | Fix |
|---|---|
| `Detection backend: Haar cascade` (no yawns/EAR) | dlib not compiled in — install dlib (vcpkg) and rebuild **with** the toolchain flag in a fresh `build` dir |
| `Could not open camera` | close Teams/Zoom, or try `--camera 1` |
| Missing `opencv_world4100.dll` | re-run the `$env:Path += ...` line before launching |
| `Could not locate Haar cascades` | bundled in `data/`; run from the repo root or pass `--cascades` |
| Face lost in low light | improve lighting; Haar/HOG need a reasonably lit, front-facing face |
| Erratic state switching | increase `state_enter_seconds` / `state_exit_seconds` |

## Testing procedure

1. **Presence** — leave the frame → "DRIVER NOT DETECTED" after `no_face_alarm_seconds`; brief look-away should NOT trigger it (grace).
2. **Drowsiness** — close eyes ~0.7 s → DROWSY; ~1.3 s → CRITICAL; alarm beeps.
3. **PERCLOS** — blink slowly/often → PERCLOS % rises → fatigue.
4. **Yawn** — open mouth wide ~1 s → yawn counter increments.
5. **Distraction** — turn head or look down for >2 s → DISTRACTED.
6. **Recovery** — return to normal → state de-escalates after `state_exit_seconds`.
7. Check `logs/events.csv` and the on-screen timeline recorded each transition.

## Roadmap

- **Phase 2 (optional):** phone detection via ONNX Runtime + YOLOv8-nano
  (`--phone-model models/yolov8n.onnx`), already wired into the RiskEngine and
  gated behind a CMake option so the build works with or without it.
