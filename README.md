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
- **OpenCV 4** (base modules required; the `face`/contrib module is optional and
  only needed for landmark mode)

---

## Windows (PowerShell)

> These are the steps for a Windows laptop — the folder examples use
> `C:\Users\Heman\DMS-NXP`. Run everything from the project folder in PowerShell.

### 1. Install the toolchain

- **Visual Studio 2022** with the *"Desktop development with C++"* workload
  (gives you the MSVC compiler). The *Community* edition is free.
- **CMake** — <https://cmake.org/download/> (tick "Add CMake to PATH").

### 2. Install OpenCV — pick ONE

**Option A — Prebuilt OpenCV (fastest, no compiling). Runs in Haar mode.**

1. Download the Windows package from <https://opencv.org/releases/> and run it
   to extract, e.g. to `C:\opencv`.
2. Add the DLL folder to your PATH so the app can find `opencv_world4xx.dll`:
   ```powershell
   $env:Path += ";C:\opencv\build\x64\vc16\bin"
   ```
   (Use `vc16` for VS 2019/2022; check which folder exists.)

The official prebuilt package does **not** include the `face` module, so
landmark mode is off — but the full drowsiness/distraction demo still works.

**Option B — vcpkg with contrib (enables landmark mode).** Compiles OpenCV, so
it takes a while, but you get accurate EAR + yawns + head pose:

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install "opencv4[contrib]:x64-windows"
$env:VCPKG_ROOT = "C:\vcpkg"
```

### 3. Build

```powershell
# Option A (prebuilt): tell CMake where OpenCV is
./scripts/build.ps1 -OpenCVDir C:\opencv\build

# Option B (vcpkg): $env:VCPKG_ROOT is picked up automatically
./scripts/build.ps1
```

This produces `build\Release\dms.exe`.

### 4. Run

```powershell
# Haar mode (works with either OpenCV option):
.\build\Release\dms.exe

# Landmark mode (Option B / vcpkg only):
./scripts/download_facemark_model.ps1
.\build\Release\dms.exe --model models\lbfmodel.yaml
```

Press **`q`** or **`ESC`** in the window to quit. Allow camera access if Windows
prompts. If the window says *"Could not open camera"*, close other apps using the
webcam (Teams, Zoom) or try `--camera 1`.

---

## Linux / macOS

Install OpenCV:

```bash
# Ubuntu / Debian (includes the contrib 'face' module)
sudo apt-get install -y libopencv-dev

# macOS (Homebrew) — includes opencv_contrib
brew install opencv
```

Build:

```bash
./scripts/build.sh
# or manually:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run:

```bash
# Haar fallback mode (works immediately):
./build/dms

# Landmark mode (accurate EAR + yawns + head pose):
./scripts/download_facemark_model.sh     # one-time, ~54 MB -> models/lbfmodel.yaml
./build/dms --model models/lbfmodel.yaml
```

> If `models/lbfmodel.yaml` exists, the program picks it up automatically.
> Press **`q`** or **`ESC`** in the window to quit.

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
