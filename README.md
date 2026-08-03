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

## Detection backends (auto-selected, best available)

The program picks the most accurate backend available at startup and prints
which one it is using:

1. **dlib 68-point landmarks (recommended, most accurate)** — dlib's HOG face
   detector plus a 68-point shape predictor give a real Eye-Aspect-Ratio
   (precise blink/drowsy detection), Mouth-Aspect-Ratio (real yawn detection)
   and head pose. Needs dlib at build time + a one-time model download.
2. **OpenCV contrib landmarks** — same 68-point idea using the opencv-contrib
   `face` module, if you built OpenCV with it.
3. **Haar cascade (always available)** — bundled Haar cascades for face + eyes,
   with "eyes closed" inferred from missing eye detections. Zero downloads,
   works offline, but noticeably less accurate — it can lose the face at angles
   and cannot measure yawns.

> If face/eye/yawn detection feels inaccurate, you are almost certainly in
> **Haar mode**. Install dlib and download the model (below) to jump to
> backend #1.

---

## Requirements

- A C++17 compiler, CMake ≥ 3.16
- **OpenCV 4** (base modules only)
- **dlib** (optional but recommended — enables the accurate 68-point backend)

---

## Windows (PowerShell)

> Run everything from the project folder (e.g. `C:\Users\<you>\DMSC-`) in
> PowerShell. Paths below assume OpenCV was extracted to `C:\opencv\opencv\build`
> — adjust if yours differs.

### 1. Install the toolchain

- **Visual Studio 2022** with the *"Desktop development with C++"* workload
  (the MSVC compiler). The free *Community* edition is fine.
- **CMake** — <https://cmake.org/download/> (tick "Add CMake to PATH").

### 2. Install OpenCV (prebuilt — no compiling)

Download the Windows package from <https://opencv.org/releases/>, run it, and
extract to `C:\`. This creates `C:\opencv\opencv\build`. The prebuilt DLL lives
in `...\build\x64\vc16\bin`.

### 3. Install dlib for the accurate 68-point backend (recommended)

The prebuilt OpenCV has no landmark model, so on its own the app runs in Haar
mode (less accurate). dlib adds precise eye-closure, yawn and head-pose
detection. Install it with vcpkg (this compiles dlib only — much faster than
rebuilding OpenCV):

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install dlib:x64-windows
```

Then download the landmark model (~96 MB, served uncompressed):

```powershell
./scripts/download_landmark_model.ps1
```

> Skipping dlib? The app still builds and runs in Haar mode.

### 4. Build

```powershell
$env:Path += ";C:\opencv\opencv\build\x64\vc16\bin"
cmake -S . -B build -A x64 `
  -DOpenCV_DIR=C:\opencv\opencv\build `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

(Omit the `-DCMAKE_TOOLCHAIN_FILE=...` line if you skipped dlib.) The configure
output prints `dlib 68-point landmark mode ENABLED` when dlib was found. This
produces `build\Release\dms.exe`.

### 5. Run

```powershell
.\build\Release\dms.exe
```

The model in `models\` is picked up automatically, so no flags are needed —
the startup log prints `Detection backend: dlib 68-point landmarks`.

Press **`q`** or **`ESC`** to quit. Allow camera access if Windows prompts. If it
says *"Could not open camera"*, close other apps using the webcam (Teams, Zoom)
or try `--camera 1`. If a fresh terminal can't find `opencv_world4100.dll`,
re-run the `$env:Path += ...` line before launching.

---

## Linux / macOS

Install OpenCV + dlib:

```bash
# Ubuntu / Debian
sudo apt-get install -y libopencv-dev libdlib-dev libblas-dev liblapack-dev

# macOS (Homebrew)
brew install opencv dlib
```

Download the 68-point landmark model, then build and run:

```bash
./scripts/download_landmark_model.sh     # one-time, ~96 MB -> models/
./scripts/build.sh                       # or: cmake -S . -B build && cmake --build build --parallel
./build/dms
```

> The model in `models/` is auto-detected, so no flags are needed. The startup
> log prints the active backend. Press **`q`** or **`ESC`** to quit.

### Command-line options

```
--camera <n>        camera index (default 0)
--model <path>      landmark model: dlib .dat or OpenCV .yaml (auto-detected from models/)
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
