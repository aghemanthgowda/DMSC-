# The Driver Monitoring System — Complete Project Guide

*A build-it-and-explain-it walkthrough of the whole DMS codebase, written for a
reader who has never programmed before.*

## What you'll learn

By the end of this guide you will be able to:

- Say, in plain words, **what the project is** and what it does.
- Point at **every folder and every file** in the repository and say what it is for.
- **Build** the program on an Ubuntu laptop and run it against a webcam.
- Read **each source file in the order the data flows**, and explain the key
  ideas in each one.
- **Cross-compile** the same program for the NXP i.MX 93 board and **deploy** it.
- **Run the live demo** over the network and **read the dashboard**.
- Explain the **honesty and ethics** choices in the code (why phone detection is
  evidence-based, why smoking/seat-belt say `UNKNOWN`, and what "designed with
  reference to AIS-184" really means).
- Answer the **viva questions** an examiner is likely to ask.

This is the deep, project-specific companion to the earlier guides on Linux,
Yocto, and the tools (OpenCV / dlib / YOLO / CMake / C++). Where those explained
the *ingredients*, this explains *the actual dish* — the real code in this
repository. When a general concept comes up (what a compiler is, what a "socket"
is), it is only summarised here; the earlier guides cover it in depth.

> Everything below matches the real files in the repository. File names, function
> names and code snippets are quoted from the actual source so you can open the
> file and follow along.

---

## Table of contents

1. [What the project is and does](#chapter-1--what-the-project-is-and-does)
2. [The repository layout](#chapter-2--the-repository-layout)
3. [The build system, and building on a laptop](#chapter-3--the-build-system-and-building-on-a-laptop)
4. [The code, file by file, following the pipeline](#chapter-4--the-code-file-by-file-following-the-pipeline)
5. [The configuration file](#chapter-5--the-configuration-file-configconfigjson)
6. [The command-line options](#chapter-6--the-command-line-options)
7. [Cross-compiling and deploying to the i.MX 93 board](#chapter-7--cross-compiling-and-deploying-to-the-imx-93-board)
8. [Giving the demo and reading the output](#chapter-8--giving-the-demo-and-reading-the-output)
9. [Honesty and ethics](#chapter-9--honesty-and-ethics)
10. [Troubleshooting and how to extend it](#chapter-10--troubleshooting-and-how-to-extend-it)
11. [Project recap and viva questions](#chapter-11--project-recap-and-viva-questions)

---

## Chapter 1 — What the project is and does

### 1.1 The one-sentence description

This project is a **real-time Driver Monitoring System (DMS)** — also called a
**Driver Drowsiness and Attention Warning System (DDAWS)**. It watches a driver's
face through a camera and decides, moment by moment, whether the driver is
**safe, tired, or distracted**, and raises a warning when needed.

It runs entirely on the local device — a laptop with its built-in webcam, or the
NXP i.MX 93 embedded board with a USB camera. **No cloud, no internet, no
external server.** Every frame is analysed on the spot and then thrown away.

### 1.2 An honest, important disclaimer

This system is **designed *with reference to* the Indian AIS-184 standard** —
India's official *Driver Drowsiness and Attention Warning System* standard (from
ARAI / AISC, under the Ministry of Road Transport & Highways). AIS-184 applies to
vehicle categories M2, M3, N2, N3 (buses and medium/heavy goods vehicles).

**It is NOT a certified or type-approved product.** The repository is very careful
about this wording, and you must be too when you present it:

> "AIS-184 requirement-oriented DDAWS research/demo prototype — designed *with
> reference to* AIS-184 requirements. This software has NOT been type-approved or
> officially certified for AIS-184 compliance."

Concretely that means:

- The thresholds in the code (EAR/PERCLOS values, seconds, angles) are **sensible
  engineering defaults, not clinically calibrated legal limits.**
- The gaze estimate is an **approximate visual cue**, not laboratory eye-tracking.
- The official AIS-184 PDF could not even be downloaded automatically (the ARAI
  web host returned an HTTP 403 error), so every clause-level claim in the docs is
  marked `VERIFY-OFFICIAL`.

This honesty is a *feature* of the project, not a weakness. Chapter 9 returns to
it in detail.

### 1.3 What it can actually do

| Capability | How it works | Honest status |
|---|---|---|
| **Face detection** | dlib HOG detector picks the largest face as "the driver" | Works |
| **68 facial landmarks** | dlib 68-point shape predictor marks eyes, brows, nose, mouth, jaw | Works |
| **Drowsiness** | EAR (eye openness), PERCLOS (% time eyes closed), blink rate, long blinks, yawns (MAR) | Works |
| **Distraction / attention** | Head pose (yaw/pitch/roll via `solvePnP`) + approximate gaze, temporally filtered | Works |
| **Phone use** | Real YOLO object detection (a phone box, temporally confirmed, optionally fused with a hand) | Works **only** with a model; otherwise honestly `NO PHONE` |
| **Smoking** | Multi-cue logic wired, but no cigarette model in COCO | Honestly `UNKNOWN` |
| **Seat belt** | Torso region computed, but no seat-belt model in COCO | Honestly `UNKNOWN` |
| **Monitoring quality** | A "reliability gate": if the view is too dark / face too small / too angled, it refuses to assert drowsiness | Works |
| **Risk fusion** | A weighted blend of all signals into one 0–100 risk score + a stable driver state | Works |
| **Alerts** | Visual banner + rate-limited audible beep, escalating on critical | Works |
| **Logging** | An on-screen event timeline and a `logs/events.csv` file (metadata only, never images) | Works |
| **Dashboard** | A professional heads-up display with the camera view, gauges and panels | Works |
| **Headless + network stream** | Runs with no monitor, streaming the dashboard to any browser over the network | Works (this is the demo path for the board) |

### 1.4 The seven driver states

Everything the system computes eventually collapses into **one** of these labels,
shown big at the top of the dashboard:

```text
SAFE · ATTENTION REQUIRED · DROWSY · DISTRACTED · PHONE USAGE · HIGH RISK · CRITICAL
```

Chapter 8 explains exactly what triggers each.

---

## Chapter 2 — The repository layout

Here is the whole project as a tree, with what each part holds.

```text
dmsc-/
├── CMakeLists.txt          # the build recipe (tells CMake how to compile everything)
├── README.md               # the human-facing overview and quick start
├── config/
│   └── config.json         # ALL tunable thresholds live here (Chapter 5)
├── include/dms/            # the header files (.hpp) — the "table of contents" of each module
│   ├── Config.hpp          #   every threshold as a C++ field, fully documented
│   ├── Types.hpp           #   the shared data structures + the enums (states)
│   ├── CameraGrabber.hpp   #   threaded "always newest frame" camera reader
│   ├── FaceTracker.hpp     #   face + 68 landmarks + EAR/MAR/head-pose
│   ├── GazeEstimator.hpp   #   approximate pupil-offset gaze
│   ├── HandActivity.hpp    #   skin-based hand-near-face (extended, off by default)
│   ├── MonitoringQuality.hpp #  the reliability gate
│   ├── DrowsinessDetector.hpp # EAR/PERCLOS/blink/yawn analysis
│   ├── DistractionDetector.hpp # head + gaze fusion into attention
│   ├── ObjectDetector.hpp  #   YOLO phone detection + phone state machine
│   ├── SmokingDetector.hpp #   honest UNKNOWN cigarette logic
│   ├── SeatBeltDetector.hpp#   honest UNKNOWN belt logic + torso ROI
│   ├── RiskEngine.hpp      #   weighted fusion + hysteresis state machine
│   ├── AlertManager.hpp    #   alert levels + audible-beep cooldown
│   ├── EventLogger.hpp     #   in-memory timeline + CSV file
│   ├── Dashboard.hpp       #   the HUD renderer
│   ├── MjpegServer.hpp     #   the tiny built-in web-video server
│   └── ConfigManager.hpp   #   loads/saves config.json
├── src/                    # the implementation files (.cpp) — the actual logic
│   ├── main.cpp            #   the program entry point and the per-frame loop
│   ├── Types.cpp           #   the enum → text names (toString)
│   ├── ConfigManager.cpp
│   ├── FaceTracker.cpp
│   ├── GazeEstimator.cpp
│   ├── HandActivity.cpp
│   ├── MonitoringQuality.cpp
│   ├── DrowsinessDetector.cpp
│   ├── DistractionDetector.cpp
│   ├── ObjectDetector.cpp
│   ├── SmokingDetector.cpp
│   ├── SeatBeltDetector.cpp
│   ├── RiskEngine.cpp
│   ├── AlertManager.cpp
│   ├── EventLogger.cpp
│   ├── Dashboard.cpp
│   └── MjpegServer.cpp
├── models/                 # downloaded model files (git-ignored, large)
│   ├── shape_predictor_68_face_landmarks.dat  # the dlib 68-point model (~96 MB)
│   ├── lbfmodel.yaml        #  an alternative OpenCV landmark model
│   └── (optional) yolov5s.onnx / yolov8n.onnx / pfld.onnx
├── data/                   # bundled Haar cascade XML files (the universal fallback)
│   ├── haarcascade_frontalface_default.xml
│   └── haarcascade_eye_tree_eyeglasses.xml
├── scripts/                # helper scripts (build + model download) for Linux (.sh) and Windows (.ps1)
├── docs/                   # the documentation, incl. this guide
│   ├── ARCHITECTURE.md
│   ├── AIS184_COMPLIANCE_REPORT.md
│   ├── AIS184_TRACEABILITY.md
│   ├── TEST_PLAN.md
│   ├── MODELS.md
│   └── learning/           # the beginner study guides
└── logs/
    └── events.csv          # the runtime event log (created at runtime, git-ignored)
```

### 2.1 Why the split into `include/` and `src/`?

This is the standard C++ arrangement. A **header** (`.hpp`) is like the *contract*
or the *menu*: it declares "here is a class called `FaceTracker`, it has a
function `process(...)` that takes a frame and returns a `FaceObservation`." The
matching **source** (`.cpp`) is the *kitchen*: it contains the actual code that
does the work. Other files only need to read the short header to know how to use
a module; they never need to see the long implementation. This keeps the code
organised and lets each module be compiled separately.

### 2.2 One responsibility per file

The project deliberately puts **one job in each module** and keeps `main.cpp`
thin (just wiring). This is called *single responsibility*, and it is why the
code reads like a pipeline: each file is one stage.

---

## Chapter 3 — The build system, and building on a laptop

### 3.1 What "building" means here

The source code is text. The computer's processor cannot run text — it runs
machine code. **Building** (a.k.a. compiling + linking) turns all the `.cpp` files
into one runnable program called `dms`. We use **CMake** to organise this: CMake
reads `CMakeLists.txt`, figures out the compiler commands, and produces the build.

### 3.2 The dependencies

| Dependency | Why it's needed | Required? |
|---|---|---|
| A C++17 compiler (g++/clang) | to compile the code | Yes |
| CMake ≥ 3.16 | to drive the build | Yes |
| **OpenCV 4** | camera capture, image maths, drawing, DNN (YOLO), JPEG | Yes |
| **dlib** | the accurate 68-point landmark backend (real EAR/MAR/pose) | Recommended |
| BLAS + LAPACK | fast linear algebra that dlib uses | With dlib |
| ONNX model files | phone (YOLO) / optional PFLD landmarks | Optional |

Without dlib the program still builds and runs, but only in a reduced **Haar
mode** with no landmarks (so no EAR/yawn/head-pose). The `CMakeLists.txt` is
written so the presence of each optional piece is detected automatically.

### 3.3 Reading `CMakeLists.txt`

The top declares the project and the C++ standard:

```cmake
cmake_minimum_required(VERSION 3.16)
project(driver_monitoring_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

It then finds OpenCV, listing exactly the modules used:

```cmake
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs objdetect
             highgui videoio calib3d dnn)
```

The clever part is how it finds **dlib**. There are three ways, tried in order:
an installed dlib (via `find_package`), or a **dlib source checkout compiled as
part of this build** using `add_subdirectory`:

```cmake
if(DMS_USE_DLIB)
    find_package(dlib CONFIG QUIET)
    if(dlib_FOUND)
        set(DMS_DLIB_AVAILABLE TRUE)
        set(DMS_DLIB_TARGET dlib::dlib)
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/dlib/dlib/CMakeLists.txt")
        add_subdirectory("${CMAKE_SOURCE_DIR}/dlib/dlib" dlib_build)
        set(DMS_DLIB_AVAILABLE TRUE)
        set(DMS_DLIB_TARGET dlib::dlib)
    endif()
endif()
```

`add_subdirectory` means: "there is a folder `dlib/` next to me that has its own
build recipe — go build it too, and let me link against it." This is the most
reliable way to get dlib on machines where installing it is awkward. The first
build is a few minutes longer because dlib itself is compiled.

The `add_executable` block lists **every** `.cpp` that makes up the program —
this is the master file list:

```cmake
add_executable(dms
    src/main.cpp
    src/Types.cpp
    src/ConfigManager.cpp
    src/FaceTracker.cpp
    src/GazeEstimator.cpp
    src/HandActivity.cpp
    src/MonitoringQuality.cpp
    src/MjpegServer.cpp
    src/DrowsinessDetector.cpp
    src/DistractionDetector.cpp
    src/ObjectDetector.cpp
    src/SeatBeltDetector.cpp
    src/SmokingDetector.cpp
    src/RiskEngine.cpp
    src/AlertManager.cpp
    src/EventLogger.cpp
    src/Dashboard.cpp
)
```

Finally, when dlib is available it defines a **compile flag** so the code knows to
use it:

```cmake
if(DMS_DLIB_AVAILABLE)
    target_compile_definitions(dms PRIVATE DMS_HAVE_DLIB)
    target_link_libraries(dms PRIVATE ${DMS_DLIB_TARGET})
    message(STATUS "DMS: dlib 68-point landmark mode ENABLED")
endif()
```

That `DMS_HAVE_DLIB` name reappears throughout the C++ as `#ifdef DMS_HAVE_DLIB`,
which means "only compile this block if dlib is present." (The MJPEG server also
needs `find_package(Threads REQUIRED)` because it runs its network loop on a
background thread.)

### 3.4 Building on Ubuntu — the exact commands

```bash
# 1) install the dependencies
sudo apt-get install -y libopencv-dev libdlib-dev libblas-dev liblapack-dev

# 2) download the 68-point landmark model into models/
./scripts/download_landmark_model.sh

# 3) configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# 4) run
./build/dms
```

- `cmake -S . -B build` means "**S**ource is here (`.`), put the **b**uild files
  in a new folder called `build`." This is *configuring*.
- `cmake --build build --parallel` actually *compiles*, using all CPU cores
  (`--parallel`) so it finishes faster.
- The finished program lands at **`build/dms`**. That single file is the whole
  application.

If `libdlib-dev` is not available, the alternative (matching the README's
Option A) is to `git clone https://github.com/davisking/dlib.git` into the
project so `add_subdirectory` compiles it from source.

On success the configure log prints `DMS: dlib 68-point landmark mode ENABLED`
and, at startup, the program prints `Detection backend: dlib 68-point landmarks`.
If instead you see a **HAAR MODE** warning, dlib was not compiled in.

### 3.5 The models you need

| Model file | For | Required? | Where from |
|---|---|---|---|
| `models/shape_predictor_68_face_landmarks.dat` | dlib 68 landmarks | Needed for full features | `scripts/download_landmark_model.sh` (~96 MB) |
| `models/lbfmodel.yaml` | OpenCV alternative landmarks | Optional fallback | bundled / download script |
| `models/yolov5s.onnx` or `yolov8n.onnx` | phone detection | Optional | export from Ultralytics YOLO |
| `models/pfld.onnx` | angle-robust landmarks | Optional | export from PFLD repo |
| `data/haarcascade_*.xml` | face/eye fallback | Already bundled | in the repo |

---

## Chapter 4 — The code, file by file, following the pipeline

The best way to understand the code is to follow **one frame of video** as it
travels through the system:

```text
Camera → CameraGrabber → FaceTracker → { GazeEstimator, HandActivity,
     ObjectDetector(phone), SeatBelt, Smoking, MonitoringQuality } →
     DrowsinessDetector + DistractionDetector → RiskEngine →
     AlertManager → EventLogger → Dashboard → (window OR headless OR MjpegServer)
```

We will walk the files in that order, grouped into: **Config & Types**,
**Perception**, **Analysis**, **Fusion & output**, **Streaming**, and finally the
**glue** (`main.cpp`).

---

### 4.1 Config & Types

#### `Config.hpp` — every knob in one struct

`Config` is a plain C++ `struct` holding *every* tunable number in the whole
program, each with a documented default and a comment. Nothing else in the code
hard-codes a threshold; they all read it from here. A `struct` is just a bundle of
named values.

The fields are grouped exactly like the pipeline. A few representative groups:

```cpp
// --- Eye closure / drowsiness (EAR) ---
double earThreshold = 0.21;          // fallback EAR threshold before calibration
double earCloseRatio = 0.62;         // eyes closed when EAR < ratio * open-baseline
double eyeClosedDrowsySeconds = 0.60; // sustained closure => Drowsy
double eyeClosedAlarmSeconds = 1.20; // sustained closure => microsleep (Critical)

// --- PERCLOS (rolling % eyes-closed) ---
double perclosWindowSeconds = 60.0;
double perclosWarn = 0.15;           // >=15% => building fatigue
double perclosAlarm = 0.30;          // >=30% => drowsy

// --- Risk fusion (0..100). Weights are relative contributions. ---
double wDrowsiness = 0.40;
double wDistraction = 0.30;
double wPhone = 0.20;
double wYawn = 0.10;

// --- Temporal state machine ---
double stateEnterSeconds = 0.6;      // evidence must persist this long to escalate
double stateExitSeconds = 1.5;       // recovery must persist this long to de-escalate
```

Major groups you'll meet again: **EAR / eye-closure**, **PERCLOS**, **blinks**,
**yawn (MAR)**, **head-pose / distraction**, **gaze**, **object detection (YOLO)
class IDs & thresholds**, **phone temporal confirmation**, **hand activity**,
**presence debounce**, **monitoring-quality gate**, **risk-fusion weights &
bands**, **state-machine persistence**, **calibration**, **performance**, and
**system/IO** (camera, mirror, headless, stream, privacy). Chapter 5 maps the
JSON keys to these fields.

#### `Types.hpp` — the shared vocabulary

This header defines the **data structures** modules pass around, and the **enums**
(fixed sets of named states). Enums are how the code names things like "the driver
is drowsy" without using loose strings.

The state enums:

```cpp
enum class DrowsyLevel { Alert, Possible, Drowsy, Critical };
enum class DistractionLevel { Attentive, Brief, Distracted, Highly };
enum class PhoneState { NoPhone, Possible, Detected, UsageConfirmed };
enum class SmokingState { NoSmoking, Possible, Confirmed, Unknown };
enum class SeatBeltState { Unknown, NotDetected, Detected, Fastened };
enum class DriverState {
    Safe, AttentionRequired, Drowsy, Distracted, PhoneUsage, HighRisk, Critical
};
```

Notice `SmokingState` and `SeatBeltState` both default to **`Unknown`** — the
honesty principle baked into the type itself.

The most important data structure is `FaceObservation`, the **per-frame output of
the FaceTracker**. Every later stage reads from it:

```cpp
struct FaceObservation {
    bool faceDetected = false;
    int faceCount = 0;                    // number of faces seen this frame
    double confidence = 0.0;              // detector confidence for the driver face
    cv::Rect face;                        // driver face box (largest / tracked)

    bool hasLandmarks = false;
    std::vector<cv::Point2f> landmarks;   // 68 points when hasLandmarks == true

    bool eyesClosed = false;
    double earLeft = -1.0, earRight = -1.0, ear = -1.0;
    double mar = -1.0;                    // mouth-aspect-ratio

    bool hasHeadPose = false;
    double yaw = 0.0, pitch = 0.0, roll = 0.0;   // head angles in degrees
};
```

A value of `-1.0` for `ear`/`mar` is a deliberate sentinel meaning "not measured"
(so later code can tell "eyes wide open, EAR = 0.0" apart from "no landmarks, EAR
unknown"). The other result structs — `GazeResult`, `HandResult`, `PhoneResult`,
`SmokingResult`, `SeatBeltResult` — follow the same idea: each has an `available`
or `valid` flag, then the actual measurement.

#### `Types.cpp` — names for humans

This file is just a set of `toString(...)` functions that turn each enum into text
for the dashboard and logs:

```cpp
const char* toString(DriverState s) {
    switch (s) {
        case DriverState::Safe: return "SAFE";
        case DriverState::AttentionRequired: return "ATTENTION REQUIRED";
        case DriverState::Drowsy: return "DROWSY";
        // ...
        case DriverState::Critical: return "CRITICAL";
    }
    return "?";
}
```

#### `ConfigManager.cpp` — loading and saving the JSON

`ConfigManager` reads `config/config.json` into a `Config`. It uses OpenCV's
`FileStorage` so no extra JSON library is needed. The key design choice: **missing
keys keep their compiled-in defaults**, so a partial or absent config file is
always safe. A tiny helper only overwrites a field if the key actually exists:

```cpp
template <typename T>
void readInto(const cv::FileStorage& fs, const char* key, T& v) {
    const cv::FileNode n = fs[key];
    if (!n.empty()) { T tmp{}; n >> tmp; v = tmp; }
}
```

`load()` then calls `readInto` once per key (`readInto(fs, "ear_threshold",
cfg.earThreshold);` and so on), and `writeDefault()` writes them all back out so a
fresh install gets a fully-documented config file on first run.

---

### 4.2 Perception — turning pixels into measurements

#### `CameraGrabber.hpp` — the anti-lag trick (header-only)

This small class solves the single biggest problem on a slow device: **lag**. If
the program processes frames slower than the camera produces them, the unprocessed
frames pile up in a buffer and the video you see drifts further and further behind
reality.

`CameraGrabber` fixes this by running the camera on its **own background thread**
that keeps **only the newest frame** and throws stale ones away:

```cpp
void loop() {
    cv::Mat frame;
    while (running_.load()) {
        if (!cap_->read(frame) || frame.empty()) { /* ended */ break; }
        {
            std::lock_guard<std::mutex> lk(mtx_);
            latest_ = frame.clone();   // replace whatever was pending
            hasFrame_ = true;
        }
        cv_.notify_one();
    }
}
```

The processing loop calls `read()`, which blocks until a frame is ready, then
hands over the latest one. Because older frames are dropped, latency can never
grow without bound. The comment notes that this doesn't distort the metrics
because **all timing is wall-clock based**, not frame-count based — PERCLOS and
blink timers use real seconds, so a skipped frame changes nothing.

Two synchronisation tools appear here (covered in the C++ guide): a `std::mutex`
(a lock so two threads never touch `latest_` at the same instant) and a
`std::condition_variable` (a way for `read()` to *sleep* until a new frame
arrives instead of busy-spinning).

#### `FaceTracker.cpp` — the heart of perception

`FaceTracker` turns a raw BGR frame into a filled-in `FaceObservation`. It picks
the best available **backend** automatically at `init()`:

| Backend | What it gives | When |
|---|---|---|
| **dlib** | HOG face detector + 68-point predictor: real EAR, MAR, head pose | best; `DMS_HAVE_DLIB` and a `.dat` model |
| **ONNX PFLD** | 68-point landmarks via OpenCV DNN, more angle-robust | if a `.onnx` landmark model is present |
| **OpenCV LBF** | 68-point facemark from opencv-contrib (`.yaml`) | if the `face` module is built |
| **Haar** | face + eye cascades only; "eyes closed" inferred from missing eye detections | universal fallback, always works |

`backendName()` reports which one is active. The core landmark maths lives in a
few small helper functions. **EAR (Eye Aspect Ratio)** measures how open an eye
is — vertical eyelid distance over horizontal eye width:

```cpp
double eyeAspectRatio(const std::vector<cv::Point2f>& lm, const int idx[6]) {
    const double vertical = dist(lm[idx[1]], lm[idx[5]]) + dist(lm[idx[2]], lm[idx[4]]);
    const double horizontal = dist(lm[idx[0]], lm[idx[3]]);
    if (horizontal < 1e-6) return 0.0;
    return vertical / (2.0 * horizontal);
}
```

When the eye is open, the eyelids are far apart and EAR is high (~0.3); when it
closes, vertical distance collapses and EAR drops toward 0. **MAR (Mouth Aspect
Ratio)** is the same idea for the mouth, normalised by the stable outer-lip width
so it rises smoothly during a yawn.

`fillLandmarkMetrics` computes EAR (average of left `36–41` and right `42–47`),
MAR, and a quick `eyesClosed` flag from the 68 points.

**The performance trick.** Face *detection* is expensive; landmark *fitting* is
cheap. So dlib detection runs only every Nth frame on a half-size image, and the
last box is reused in between — while the 68-point predictor still runs every
frame so the landmarks stay live:

```cpp
const int interval = std::max(1, cfg_.faceDetectEveryNFrames);
const bool doDetect = (frameCount_++ % interval == 0) || !haveLastFace_;
if (doDetect) {
    const double scale = std::clamp(cfg_.detectionScale, 0.25, 1.0);
    cv::resize(frameBGR, small, cv::Size(), scale, scale, cv::INTER_LINEAR);
    // ... run HOG detector on `small`, pick the largest face as the driver ...
}
```

The README notes this "roughly triples throughput on a laptop CPU" — the single
biggest speedup in the project.

**Head pose** is estimated with `cv::solvePnP`, which compares six known
landmarks (nose tip, chin, eye corners, mouth corners) against a generic 3D face
model to recover the head's yaw/pitch/roll angles:

```cpp
if (!cv::solvePnP(model, image, cameraMatrix, distCoeffs, rvec, tvec)) return;
cv::Rodrigues(rvec, rot);
cv::decomposeProjectionMatrix(proj, k, r, t, rx, ry, rz, euler);
obs.pitch = normalize(euler.at<double>(0));
obs.yaw   = normalize(euler.at<double>(1));
obs.roll  = normalize(euler.at<double>(2));
obs.hasHeadPose = true;
```

Yaw = turning left/right, pitch = looking up/down, roll = tilting. These feed the
distraction detector.

---

### 4.3 Analysis — turning measurements into judgements

#### `DrowsinessDetector.cpp` — fatigue over time

A single frame can't tell you someone is drowsy — you need *time*. This module
watches the EAR and MAR stream and derives fatigue signals. Its result struct
carries `eyesClosed`, `closureSeconds`, `perclos`, `blinkCount`, `blinkRate`,
`longBlinkCount`, `yawnCount`, `yawning`, a `DrowsyLevel`, and a 0–100 `score`.

**Self-calibrating eye-closed decision.** Rather than one fixed threshold (which
fails across faces, glasses, and distances), it smooths the EAR, learns the
driver's own "eyes open" baseline (the rolling *max* EAR), and flags closed
relative to that — with **hysteresis** so a value hovering at the edge doesn't
flicker:

```cpp
double threshold = cfg_.earThreshold;
if (baseline > 0.15) threshold = std::clamp(baseline * cfg_.earCloseRatio, 0.15, 0.30);
// Hysteresis: enter "closed" below threshold, only leave once EAR rises clearly above.
if (!eyeClosedState_) { if (smoothed < threshold) eyeClosedState_ = true; }
else                  { if (smoothed > threshold * 1.12) eyeClosedState_ = false; }
```

**Blinks vs long blinks vs closure** are separated by *duration* using an
edge-triggered timer: when the eyes reopen, the closure length decides whether it
was a normal blink (`0.06–0.40 s`), a long blink (`≥ 0.5 s`), or a prolonged
closure.

**PERCLOS** (the percentage of a rolling 60-second window with eyes closed) is
computed from timestamps, not frame counts, so a fluctuating frame rate doesn't
distort it:

```cpp
while (!window_.empty() && tSeconds - window_.front().first > cfg_.perclosWindowSeconds)
    window_.pop_front();
int closedCount = 0;
for (const auto& s : window_) closedCount += s.second ? 1 : 0;
r.perclos = static_cast<double>(closedCount) / window_.size();
```

**Yawns** use an adaptive MAR threshold (learned closed-mouth baseline + a delta),
a minimum open duration, and a "dip tolerance" so brief landmark jitter during a
yawn doesn't reset the timer.

Finally it produces the score and the 4-level classification, with an explicit
gate so a clear microsleep is always `Critical` regardless of the blended score:

```cpp
if (r.closureSeconds >= cfg_.eyeClosedAlarmSeconds || r.score >= 80.0)
    r.level = DrowsyLevel::Critical;
else if (r.closureSeconds >= cfg_.eyeClosedDrowsySeconds ||
         r.perclos >= cfg_.perclosAlarm || r.score >= 50.0)
    r.level = DrowsyLevel::Drowsy;
else if (r.perclos >= cfg_.perclosWarn || r.yawning || r.score >= 25.0)
    r.level = DrowsyLevel::Possible;
else
    r.level = DrowsyLevel::Alert;
```

#### `DistractionDetector.cpp` — attention from head + gaze

This module fuses **head pose** (temporally smoothed with an EMA — exponential
moving average) and **approximate gaze** into an attention score. Short natural
glances are ignored; only *sustained* head/gaze-away escalates.

First it classifies head direction from the smoothed angles:

```cpp
if (emaYaw_ > cfg_.headAwayYawDegrees) r.headDir = "right";
else if (emaYaw_ < -cfg_.headAwayYawDegrees) r.headDir = "left";
else if (emaPitch_ < -cfg_.headDownPitchDegrees) r.headDir = "down";
else if (emaPitch_ > cfg_.headDownPitchDegrees) r.headDir = "up";
else if (std::abs(emaRoll_) > cfg_.headAwayYawDegrees) r.headDir = "tilted";
else r.headDir = "forward";
```

A "sustain timer" measures how long the driver has been looking away, and the
score grows as it persists:

```cpp
const double sustain = std::clamp(r.awaySeconds / cfg_.headAwayDurationSeconds, 0.0, 1.0);
if (headAway) s += 30.0 + 40.0 * sustain;   // grows as it persists
if (gazeAway) s += 15.0;
if (phone.phonePresent) s += 25.0;          // a detected phone raises it
```

When there's **no face at all**, the module reports a presence-related score that
grows toward `DRIVER NOT DETECTED` after `noFaceAlarmSeconds`. The 4 levels
(`Attentive`/`Brief`/`Distracted`/`Highly`) come from score bands.

> Important: a detected phone *raises* the distraction score, but the phone signal
> itself comes only from real object detection — never the reverse. Turning your
> head is distraction, not "phone use." See Chapter 9.

#### `GazeEstimator.cpp` — approximate pupil offset

dlib's 68 points have **no iris landmark**, so gaze here is deliberately coarse.
For each open eye it crops the eye region, blurs it, and finds the darkest blob
(the pupil/iris) via its centroid, then reports the pupil's offset from the eye
centre normalised to `[-1, 1]`:

```cpp
cv::threshold(blurred, darkMask, minVal + (maxVal - minVal) * 0.25, 255, cv::THRESH_BINARY_INV);
cv::Moments m = cv::moments(darkMask, true);
// ... centroid becomes the pupil center ...
dx = (center.x / roi.width  - 0.5) * 2.0;
dy = (center.y / roi.height - 0.5) * 2.0;
```

It's smoothed (EMA) and turned into a `direction` string
(`forward/left/right/up/down`). The struct is honestly named `GazeResult` and the
header comment stresses this is "**NOT** medically/scientifically exact gaze."

#### `HandActivity.cpp` — skin-based hands (off by default)

An *extended* feature that finds skin-coloured blobs in bands beside the face
(ear/cheek level) using a YCrCb colour range and contour filtering. It's **off by
default** (`enableHandDetection = false`) because the skin heuristic false-fires
on ears/necks at angled mounts, and the accurate path is now YOLO. When on, it
temporally confirms "hand near face" and its boxes can be *fused* with a detected
phone. It never gates the core drowsiness path.

#### `MonitoringQuality.cpp` — the reliability gate

This is a key safety idea. If the system can't actually *see* the driver's eyes
well, it must **not** assert drowsiness — instead it reports "monitoring quality
low." It checks brightness, face size, detection confidence, landmark presence,
and head rotation, and reports the first (most limiting) failure reason:

```cpp
if (meanLuma < cfg_.qualityLowLightMean) { r.reliable = false; r.reason = "low light"; }
else if (faceFrac < cfg_.qualityMinFaceWidthFraction) { r.reliable = false; r.reason = "face too small / far"; }
// ... low detection confidence / no landmarks / extreme head rotation / partly out of frame ...
```

So a dark room, a tiny/far face, or an extreme head turn produces `MONITORING
QUALITY LOW` on the dashboard, **not** a false `DRIVER DROWSY`. The RiskEngine
respects this and zeroes out the drowsiness evidence when monitoring is unreliable.

#### `ObjectDetector.cpp` — real YOLO phone detection

This is the accurate, evidence-based phone path. It runs a **YOLO ONNX model
through OpenCV's own `cv::dnn`** — so there is *no* ONNX Runtime dependency; if you
built the project you already have the DNN module. Without a model file it stays
`available_ = false` and returns `NO PHONE` cheaply.

`runYolo()` does the standard pipeline: **letterbox** the frame to a square
keeping aspect ratio, make a blob, run `forward()`, then parse the output. It
supports both YOLOv5 (has an "objectness" column) and YOLOv8 layouts:

```cpp
if (out.dims == 3) out = out.reshape(1, out.size[1]);
if (out.rows < out.cols) cv::transpose(out, out);
const bool hasObjectness = (dim == 85);   // v5 COCO; v8 COCO = 84
const int classStart = hasObjectness ? 5 : 4;
```

For each candidate it picks the best class, thresholds on confidence, maps the box
back to frame coordinates, and finally applies **Non-Maximum Suppression** to
remove overlapping duplicate boxes:

```cpp
cv::dnn::NMSBoxes(boxes, scores, cfg_.yoloConfidence, cfg_.nmsThreshold, keep);
```

`detect()` runs YOLO only every Nth frame, then finds the best **phone** box
(class 67 by default), applies **temporal confirmation** (a phone must be seen in
several recent cycles), optionally **fuses with a hand**, and drives the phone
state machine:

```cpp
if (hits >= cfg_.phoneConfirmFrames)
    r.state = heldByHand ? PhoneState::UsageConfirmed : PhoneState::Detected;
else if (hits > 0)  r.state = PhoneState::Possible;
else                r.state = PhoneState::NoPhone;
r.phonePresent = (r.state == PhoneState::Detected || r.state == PhoneState::UsageConfirmed);
```

So `NO_PHONE → POSSIBLE → DETECTED → USAGE_CONFIRMED`, and only `Detected`/
`UsageConfirmed` count as "phone present."

#### `SmokingDetector.cpp` and `SeatBeltDetector.cpp` — honest UNKNOWN

Both follow the same honesty rule. **Smoking** needs a cigarette detection, but
COCO has no cigarette class, so without a custom model it returns `Unknown` and
never guesses:

```cpp
if (cfg_.classIdCigarette < 0 || !cigarette.available) {
    r.available = false;
    r.state = SmokingState::Unknown;
    return r;   // never fake from movement
}
```

The full multi-cue logic (cigarette near mouth **and** hand near mouth, temporally
confirmed) is already implemented and activates automatically the moment a real
cigarette model is provided. **Seat belt** likewise computes the torso ROI (where
a shoulder belt crosses) but returns `Unknown` until a belt model sets
`classIdSeatbelt >= 0`, and it "never reports 'not worn' merely because the belt
is not visible."

---

### 4.4 Fusion & output

#### `RiskEngine.cpp` — combining everything into one state

This is the decision brain. It takes the drowsiness result, the distraction
result, the phone result, whether the driver is present, and whether monitoring is
reliable, and produces one 0–100 risk score plus a stable `DriverState`.

**Step 1 — respect the monitoring gate.** If monitoring is unreliable, drowsiness
and yawn evidence are zeroed (distinguishing "can't see the driver" from "driver
is drowsy"):

```cpp
const double drowsyScore = monitoringReliable ? drowsy.score : 0.0;
const double yawnScore = (monitoringReliable && drowsy.yawning) ? 100.0 : 0.0;
```

**Step 2 — weighted blend.** Each signal contributes in proportion to its weight,
normalised by the sum of weights:

```cpp
r.drowsyContribution      = cfg_.wDrowsiness  * drowsyScore;
r.distractionContribution = cfg_.wDistraction * distract.score;
r.phoneContribution       = cfg_.wPhone       * phoneScore;
r.yawnContribution        = cfg_.wYawn        * yawnScore;
double risk = (sum of contributions) / wSum;
```

**Step 3 — per-subsystem floors.** A single strong signal must not be *averaged
away* by an otherwise-calm blend, so the risk is floored to a fraction of each
signal:

```cpp
risk = std::max(risk, drowsyScore * 0.85);
risk = std::max(risk, distract.score * 0.60);
risk = std::max(risk, phoneScore * 0.90);
if (!driverPresent) risk = std::max(risk, cfg_.riskHighThreshold);
```

**Step 4 — smooth** the score with an EMA for a steady gauge, then pick a
**candidate state** from the risk bands (warning / high / critical) plus which
signal dominates (phone vs drowsy vs distracted).

**Step 5 — the hysteresis state machine.** The candidate does not take effect
immediately: a change must *persist* before it commits. Escalations react faster
(`stateEnterSeconds`) than recoveries (`stateExitSeconds`), so the state can never
flicker frame-to-frame:

```cpp
if (candidate != current_) {
    if (candidate != pending_) { pending_ = candidate; pendingSince_ = tSeconds; }
    const double need = severity(candidate) > severity(current_)
                            ? cfg_.stateEnterSeconds : cfg_.stateExitSeconds;
    if (tSeconds - pendingSince_ >= need) current_ = candidate;
}
```

It also sets an `alertLevel` (0–3) from the committed state and builds a list of
human-readable `reasons` for the panel.

#### `AlertManager.cpp` — banners and the beep

Turns the risk state into a headline, a message, and an **audible alarm** with
cooldown. The beep fires only at level ≥ 2, rate-limited, and pulses roughly twice
as fast when critical:

```cpp
if (cfg_.beep && s.level >= 2) {
    const double interval = s.level >= 3 ? cfg_.alertCooldownSeconds * 0.5
                                         : cfg_.alertCooldownSeconds;
    if (tSeconds - lastBeep_ >= interval) {
        std::fputc('\a', stderr);   // the terminal bell character
        std::fflush(stderr);
        lastBeep_ = tSeconds;
    }
}
```

The `'\a'` is the ASCII "bell" — it makes the terminal beep, a zero-dependency
alarm.

#### `EventLogger.cpp` — the timeline and the CSV

Keeps a rolling in-memory list of the most recent events (for the dashboard
timeline) and, unless disabled, appends each to `logs/events.csv`. **Privacy mode
or `--no-log` disables all disk writes**; the timeline stays in memory only.
Crucially, **only metadata is ever written — never an image**:

```cpp
if (!cfg_.logEvents || cfg_.privacyMode) return;   // no disk writes at all
// ...
csv_ << e.time << ',' << level << ',' << '"' << text << '"' << '\n';
```

#### `Dashboard.cpp` — the heads-up display

This renders everything into one canvas: the annotated camera view on the left and
a status panel on the right, joined with `cv::hconcat`. On the camera view it
draws HUD corner brackets on the face, the 68 landmark contours (eyes, mouth,
nose, jaw), a head-pose arrow, gaze arrows and pupil dots, the phone box, and a
red border + top banner on a critical alert. During calibration it dims the frame
and shows "Sit normally and look ahead at the road."

The side panel has a **hero status card** with the big driver-state text and an
**arc gauge** for the 0–100 risk, a row of condition **pills** (HAAR MODE,
MONITORING LOW, PRIVACY MODE), a **VITALS grid** of small cards (EAR, EYES,
PERCLOS, MAR, BLINKS, YAWNS, DROWSINESS, ATTENTION, HEAD Y/P/R, GAZE, PHONE,
SMOKING, SEAT BELT, HAND/EYE), an optional **developer line** (inference ms + risk
contributions), and the **EVENT LOG** timeline. It's all drawn with basic OpenCV
primitives (`rectangle`, `circle`, `ellipse`, `putText`) — no GUI toolkit.

---

### 4.5 Streaming — `MjpegServer.cpp`

The board has no monitor, so how do you *watch* the demo? This tiny class is a
built-in web server that streams the dashboard as **MJPEG over HTTP** — a sequence
of JPEG images pushed to the browser, which is exactly how simple IP cameras work.
It uses plain POSIX sockets and OpenCV's JPEG encoder, with **no external
libraries** (and compiles to a harmless no-op on Windows).

It listens on a port, and its accept loop routes requests: any normal path gets a
tiny HTML landing page that embeds `<img src="/stream">`, while `GET /stream` gets
the live multipart feed:

```cpp
if (req.find("GET /stream") == std::string::npos) {
    sendAll(fd, kIndexPage, sizeof(kIndexPage) - 1);  // the landing page
    ::close(fd);
    continue;
}
// otherwise: send the multipart header and add this client to the broadcast list
```

`publish(canvas)` encodes the frame to JPEG once and sends it to every connected
viewer; dead clients are dropped. It's cheap when nobody is watching (it returns
immediately if the client list is empty). Multiple viewers can connect at once.

---

### 4.6 The glue — `main.cpp`, walked step by step

`main.cpp` wires everything together and runs the per-frame loop. Here is the whole
flow.

**1) Parse config, then command-line arguments.** Config file first (overlays the
defaults), then CLI flags override both:

```cpp
Config cfg;
ConfigManager::load(configPath, cfg);
// ... then a loop over argv sets cfg.cameraIndex, cfg.headless, cfg.streamEnabled, etc.
```

**2) Apply the `--fast` / `--no-phone` performance switches.** `--fast` drops
YOLO, lowers resolution, and runs face detection less often:

```cpp
if (fast) {
    noPhone = true;
    cfg.captureWidth = 480; cfg.captureHeight = 360;
    cfg.faceDetectEveryNFrames = std::max(cfg.faceDetectEveryNFrames, 5);
    cfg.streamJpegQuality = std::min(cfg.streamJpegQuality, 70);
}
if (noPhone) cfg.phoneModel.clear();
```

**3) Auto-detect resources.** Find the Haar cascade folder, the landmark model
(`.dat` or `.yaml`), an optional YOLO model, and an optional PFLD ONNX model. Write
a default config on first run.

**4) Initialise the modules.** Construct the `FaceTracker` (and fail early if the
Haar cascades can't load), the `ObjectDetector`, and print the active backend:

```cpp
FaceTracker tracker(cfg);
if (!tracker.init()) return 1;
std::cout << "[info] Detection backend: " << tracker.backendName() << "\n";
```

**5) Open the camera and hand it to the `CameraGrabber`** (the anti-lag thread):

```cpp
cv::VideoCapture cap(cfg.cameraIndex);
cap.set(cv::CAP_PROP_FRAME_WIDTH, cfg.captureWidth);
cap.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.captureHeight);
CameraGrabber grabber;
grabber.start(cap);
```

**6) Construct every analyser** (gaze, hand, seat-belt, smoking, quality,
drowsiness, distraction, risk, alerts, logger, dashboard), open a window unless
headless, and start the MJPEG server if `--stream` was given.

**7) The main loop.** For every frame:

- **Read the newest frame** from the grabber; **rotate** and **mirror** it:

```cpp
if (!grabber.read(frame) || frame.empty()) break;
if (cfg.cameraRotation == 90) cv::rotate(frame, frame, cv::ROTATE_90_CLOCKWISE);
// ... 180 / 270 ...
if (cfg.mirror) cv::flip(frame, frame, 1);
```

- **Track the face:** `FaceObservation obs = tracker.process(frame);` (and measure
  inference time for the developer overlay).

- **Head-pose neutral calibration.** During the first few seconds it averages the
  driver's normal pose (their real "looking at the road" position for *this* camera
  mount), then subtracts it so a side-mounted camera doesn't read the neutral pose
  as "looking away":

```cpp
if (cfg.headPoseCalibrate && calibratingNow && obs.hasHeadPose) {
    sumYaw += obs.yaw; sumPitch += obs.pitch; sumRoll += obs.roll; ++sumN;
}
// after calibration: neutYaw = sumYaw/sumN, etc., then every frame:
if (neutSet && obs.hasHeadPose) { obs.yaw -= neutYaw; obs.pitch -= neutPitch; obs.roll -= neutRoll; }
```

- **Presence grace.** A brief face dropout reuses the last good face so a quick
  glance away doesn't instantly say "absent":

```cpp
if (obs.faceDetected) { lastGood = obs; haveLastGood = true; lastSeen = t; }
const bool withinGrace = (t - lastSeen) < cfg.faceLostGraceSeconds;
const bool driverPresent = obs.faceDetected || withinGrace;
```

- **Gaze and hand:** `gaze.estimate(...)`, `handAnalyzer.update(...)`.

- **Phone detection (evidence only):** `phoneDet.detect(...)`. The comment here is
  emphatic that head pose *never* implies phone use.

- **Seat belt and smoking:** computed honestly (`UNKNOWN` without a model). The
  mouth centre for the smoking cue is taken from landmarks 48/54 when available.

- **Monitoring-quality gate**, then the two analysers and the fusion:

```cpp
const MonitoringQuality::Result mq = quality.assess(frame, eff);
const bool monitoringReliable = eff.faceDetected ? mq.reliable : true;
const auto dRes = drowsy.update(eff, t);
const auto kRes = distract.update(eff, g, phone, frame.size(), t);
const auto risk = riskEngine.update(dRes, kRes, phone, driverPresent, monitoringReliable, t);
```

- **Alerts** (suppressed and shown as `CALIBRATING` during calibration):
  `alert = alerts.update(risk, t);`

- **Event logging on transitions** — driver detected/lost, state changes, each new
  yawn, phone first detected.

- **Render** the dashboard into `canvas`, then output it three possible ways:

```cpp
const cv::Mat canvas = dashboard.render(frame, df);
if (cfg.streamEnabled) stream.publish(canvas);      // network viewers
if (cfg.headless) { /* print status line + imwrite snapshot, then continue */ }
cv::imshow(win, canvas);                             // desktop window
```

- **Keys** (windowed mode only): `q`/ESC quit, `d` toggle developer mode, `c`
  restart calibration.

**8) Cleanup.** On exit it stops the stream, stops the grabber, releases the
camera, and closes windows.

The headless status line printed each second looks like:

```text
[dms] t=12.0s state=Safe risk=8 | present | EAR=0.28 PERCLOS=2% drowsy=5 | yawns=0 | attn=Attentive | phone=no | 14.6fps
```

---

## Chapter 5 — The configuration file (`config/config.json`)

Every threshold lives here as JSON, and each maps directly to a field in
`Config.hpp`. Edit the JSON and restart — no rebuild needed. Here is the real file
(abbreviated) with the meaning of the important keys.

```json
{
    "ear_threshold": 0.21,
    "ear_close_ratio": 0.62,
    "eye_closed_drowsy_seconds": 0.6,
    "eye_closed_alarm_seconds": 1.2,
    "perclos_window_seconds": 60.0,
    "perclos_warn": 0.15,
    "perclos_alarm": 0.30,
    "mar_threshold": 0.18,
    "yawn_min_seconds": 0.55,
    "head_away_yaw_degrees": 25.0,
    "head_down_pitch_degrees": 18.0,
    "gaze_off_threshold": 0.28,
    "class_id_phone": 67,
    "class_id_cigarette": -1,
    "class_id_seatbelt": -1,
    "phone_confirm_frames": 3,
    "phone_detect_every_n_frames": 4,
    "w_drowsiness": 0.40,
    "w_distraction": 0.30,
    "w_phone": 0.20,
    "w_yawn": 0.10,
    "risk_warning_threshold": 40.0,
    "risk_high_threshold": 65.0,
    "risk_critical_threshold": 85.0,
    "state_enter_seconds": 0.6,
    "state_exit_seconds": 1.5,
    "camera_index": 2,
    "head_pose_calibrate": 1,
    "mirror": 1, "beep": 1, "log_events": 1, "privacy_mode": 0
}
```

### 5.1 Key-by-key tuning guide

| JSON key | `Config.hpp` field | What it controls | Tuning effect |
|---|---|---|---|
| `ear_threshold` | `earThreshold` | fixed eyes-closed EAR before calibration | lower = closes only when eyes very shut |
| `ear_close_ratio` | `earCloseRatio` | adaptive threshold = ratio × open-baseline | higher = more sensitive to closure |
| `eye_closed_drowsy_seconds` | `eyeClosedDrowsySeconds` | closure → DROWSY | lower = triggers sooner |
| `eye_closed_alarm_seconds` | `eyeClosedAlarmSeconds` | closure → CRITICAL (microsleep) | lower = alarms sooner |
| `perclos_warn` / `perclos_alarm` | `perclosWarn` / `perclosAlarm` | PERCLOS fatigue bands | lower = fatigue flagged with less eye-closure |
| `mar_threshold` / `yawn_min_seconds` | `marThreshold` / `yawnMinSeconds` | mouth-open floor + min yawn duration | lower/shorter = more yawns counted |
| `head_away_yaw_degrees` | `headAwayYawDegrees` | side-turn angle counted as "away" | lower = stricter about looking away |
| `head_down_pitch_degrees` | `headDownPitchDegrees` | look-down angle (phone posture) | lower = stricter |
| `head_away_duration_seconds` | `headAwayDurationSeconds` | how long "away" before distraction | lower = reacts faster |
| `gaze_off_threshold` | `gazeOffThreshold` | pupil offset counted as looking away | lower = more sensitive gaze |
| `class_id_phone` | `classIdPhone` | COCO class id for cell phone (67) | leave at 67 for COCO models |
| `class_id_cigarette` / `class_id_seatbelt` | `classIdCigarette` / `classIdSeatbelt` | −1 = no model → UNKNOWN | set to your custom class id to enable |
| `phone_confirm_frames` | `phoneConfirmFrames` | detections needed to confirm a phone | higher = fewer false phones, slower |
| `phone_detect_every_n_frames` | `phoneDetectEveryNFrames` | run YOLO 1/N frames | higher = faster, less responsive |
| `w_drowsiness` … `w_yawn` | `wDrowsiness` … | risk-fusion weights | raise one to give that signal more say |
| `risk_warning/high/critical_threshold` | `riskWarnThreshold` … | risk → state bands | lower = escalates at lower risk |
| `state_enter_seconds` / `state_exit_seconds` | `stateEnterSeconds` / `stateExitSeconds` | escalate/recover persistence | raise to reduce flicker |
| `alert_cooldown_seconds` | `alertCooldownSeconds` | min gap between repeat beeps | raise for less frequent beeping |
| `face_lost_grace_seconds` / `no_face_alarm_seconds` | `faceLostGraceSeconds` / `noFaceAlarmSeconds` | presence debounce / absent alarm | tune brief-glance tolerance |
| `head_pose_calibrate` | `headPoseCalibrate` | learn neutral pose for an angled mount | set 0 to use fixed offsets |
| `camera_index` / `capture_width/height` / `camera_rotation` | camera IO | which camera and at what size/rotation | match your hardware |
| `mirror` / `beep` / `log_events` / `privacy_mode` | toggles | selfie view / audible alarm / CSV logging / no-disk mode | 0 or 1 |

Rule of thumb from the README: **raising a weight** increases that signal's
influence on overall risk; **lowering an enter/exit second** makes the state react
or recover faster.

---

## Chapter 6 — The command-line options

Anything in the config can be overridden at launch. These flags are parsed in
`main.cpp`.

| Flag | Meaning |
|---|---|
| `--config <path>` | use a different config JSON (default `config/config.json`) |
| `--camera <n>` | camera index to open |
| `--list-cameras` | probe camera indices 0–9, print which are available, then exit |
| `--rotate <deg>` | rotate frames 0/90/180/270 (for a rotated mounting) |
| `--model <path>` | landmark model: dlib `.dat` or OpenCV `.yaml` (auto-detected) |
| `--landmark-model <p>` | PFLD-68 `.onnx` landmarks (angle-robust, via OpenCV DNN) |
| `--phone-model <p>` | YOLO `.onnx` for phone detection |
| `--cascades <dir>` | Haar cascade directory (auto-detected if omitted) |
| `--headless` | no GUI window: print a status line + save the dashboard to `dms_frame.jpg` |
| `--snapshot <path>` | change the headless snapshot target (also enables headless) |
| `--stream [port]` | serve the live dashboard as MJPEG over HTTP (default port 8080) |
| `--fast` | smoother/low-latency on slow devices: no YOLO, lower resolution, less-frequent detection |
| `--no-phone` | disable only the CPU-heavy YOLO phone detector |
| `--dev` | start in developer mode |
| `--privacy` | privacy mode: no logging / image storage |
| `--no-mirror` | do not mirror the view |
| `--no-beep` | disable the audible alarm |
| `--no-log` | do not write `logs/events.csv` |
| `-h`, `--help` | print usage and exit |

Example invocations:

```bash
./build/dms                                   # laptop, full features, windowed
./build/dms --list-cameras                    # find your webcam index
./build/dms --camera 1 --rotate 90            # side-mounted USB cam
./build/dms --stream --dev                    # windowed + live network stream + dev overlay
./dms --camera 0 --headless --stream --fast   # the board demo (Chapter 7)
```

Runtime keys while a window is open: `q`/ESC quit, `d` toggle developer mode,
`c` recalibrate.

---

## Chapter 7 — Cross-compiling and deploying to the i.MX 93 board

The laptop build made a program for the laptop's processor (x86-64). The i.MX 93
uses a different processor family (ARM 64-bit, "aarch64"), so its program must be
**cross-compiled**: built on the laptop but *for* the board. The Yocto SDK
provides a cross-compiler and a matching set of ARM libraries (the "sysroot").

### 7.1 Source the SDK

```bash
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux
```

`source` runs the script in your current shell so it can set environment variables
(`CC`, `CXX`, `CMAKE_...`, the sysroot path). After this, `cmake`/the compiler
target ARM instead of your laptop.

### 7.2 Configure and build for ARM

```bash
cmake -S . -B build-arm64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm64 --parallel
```

A **separate** build folder (`build-arm64`) keeps the ARM build from clobbering
the laptop build. Verify the result really is an ARM binary:

```bash
file build-arm64/dms
# expect: ELF 64-bit LSB ... ARM aarch64 ...
```

### 7.3 Copy it to the board and run

```bash
scp build-arm64/dms root@192.168.1.173:~/deploy/dms
```

`scp` is "secure copy" — it copies the file over the network to the board. Then,
on the board (over SSH or the serial console):

```bash
cd ~/deploy
./dms --camera 0 --stream --headless --fast
```

### 7.4 Why `--headless` and `--stream` are needed on the board

The board is not a laptop: **it has only a serial console — no monitor, no desktop,
no display server.** OpenCV's `cv::imshow` needs a graphical display (Qt/Wayland),
so on the board it would fail with a Qt/Wayland error and abort. Two flags solve
this:

- `--headless` runs the *full* pipeline with **no window**, printing a status line
  and writing a `dms_frame.jpg` snapshot.
- `--stream` serves the live dashboard over the network, so you watch the actual
  moving demo in a browser instead of a window.

### 7.5 Watch the live demo from your laptop

On the board, find its IP:

```bash
hostname -I        # e.g. 192.168.1.173
```

Then, on a laptop or phone **on the same network**, open a browser at:

```text
http://192.168.1.173:8080/
```

You'll see the full DMS overlay — face box, landmarks, gauges, the drowsiness /
attention / phone panels — delivered over the network at full frame rate. Multiple
viewers can connect at once. Use `--stream 9000` to choose a different port.

### 7.6 Known board fixes and harmless warnings

| Symptom | Explanation / fix |
|---|---|
| Link/build error about `opencv_ts` / `opencv_superres` in the sysroot | a known SDK packaging quirk — create the missing symlink in the sysroot so the OpenCV CMake config resolves those optional libs |
| GStreamer warnings when the camera opens | **harmless** — the camera still works; they're just backend chatter |
| The stream lags on the board | use `--fast` — it drops YOLO, lowers resolution, and detects less often, giving a smooth near-real-time demo. Keep everything except the phone detector with `--no-phone` instead |
| `imshow` aborts with a Qt/Wayland error | you forgot `--headless`; the board has no display |

The reason `--fast` works: two things cause lag on an embedded CPU — frames
buffering faster than they're processed (already handled by the `CameraGrabber`
dropping stale frames), and the CPU-heavy YOLO model (dropped by `--fast`).

---

## Chapter 8 — Giving the demo and reading the output

### 8.1 The dashboard panels

The window (or the browser stream) is split in two:

**Left — the annotated camera view.** Look for:

- HUD **corner brackets** around the driver's face, with a face-confidence %.
- The **68 landmarks**: green eye and eyebrow contours, an amber mouth contour, the
  nose and jaw outlines, and yellow landmark dots.
- A **head-pose arrow** from the nose, and **gaze arrows / pupil dots** on the eyes.
- A **red phone box** if a phone is detected; a **red border + top banner** on a
  critical alert; a "MULTIPLE FACES" note if more than one face is present.
- During startup, a dimmed **CALIBRATION** overlay: "Sit normally and look ahead."

**Right — the status panel.** From top to bottom:

- The **hero card**: the big current **driver state** and an **arc gauge** showing
  the 0–100 risk score.
- **Pills**: `HAAR MODE` (landmarks off), `MONITORING LOW: <reason>`, `PRIVACY
  MODE`.
- The **VITALS grid**: EAR, EYES (open/CLOSED), PERCLOS %, MAR/open threshold,
  BLINKS (count + rate/min), YAWNS, DROWSINESS level, ATTENTION level, HEAD Y/P/R
  angles, GAZE direction, PHONE state, SMOKING, SEAT BELT, HAND/EYE.
- A **developer line** (if `d`/`--dev`): inference ms and the raw risk
  contributions `D`/`K`/`P`.
- The **EVENT LOG** timeline of recent transitions with timestamps.

### 8.2 The headless status line fields

```text
[dms] t=12.0s state=Safe risk=8 | present | EAR=0.28 PERCLOS=2% drowsy=5 | yawns=0 | attn=Attentive | phone=no | 14.6fps
```

| Field | Meaning |
|---|---|
| `t=12.0s` | seconds since start |
| `state=Safe` | the committed driver state |
| `risk=8` | the 0–100 fused risk score |
| `present` / `ABSENT` | driver presence (with `(monitoring-low)` if the quality gate tripped) |
| `EAR=0.28` | smoothed eye-aspect-ratio |
| `PERCLOS=2%` | % of the window with eyes closed |
| `drowsy=5` | drowsiness sub-score |
| `yawns=0` | yawn count |
| `attn=Attentive` | attention level |
| `phone=no` | phone present? |
| `14.6fps` | current frame rate (plus `viewers=N` when streaming) |

### 8.3 What triggers each driver state

| State | Roughly triggered by |
|---|---|
| **SAFE** | risk below the warning band; eyes on road |
| **ATTENTION REQUIRED** | risk ≥ warning band from a mild/mixed signal |
| **DROWSY** | risk ≥ warning band and drowsiness dominates (sustained closure, high PERCLOS, yawning) |
| **DISTRACTED** | risk ≥ warning band and distraction dominates (sustained head/gaze-away) |
| **PHONE USAGE** | risk ≥ warning band and a *detected* phone dominates |
| **HIGH RISK** | risk ≥ high band, or a confirmed absent driver |
| **CRITICAL** | risk ≥ critical band (e.g. a clear microsleep — eyes shut past `eye_closed_alarm_seconds`) |

Remember the **hysteresis**: a state only commits after it persists for
`stateEnterSeconds` (escalating) or `stateExitSeconds` (recovering), so it never
flickers.

### 8.4 A suggested demo script

1. Sit normally, look at the camera, let the 3-second calibration finish → **SAFE /
   ATTENTIVE**.
2. Blink normally → blink counter rises, **no** escalation.
3. Close your eyes ~0.7 s → **DROWSY**; ~1.3 s → **CRITICAL** + beep.
4. Open your mouth wide ~1 s → the **yawn** counter increments.
5. Turn your head or look down for > 2 s → **DISTRACTED**.
6. Leave the frame → after the grace + alarm seconds, **DRIVER NOT DETECTED / HIGH
   RISK**; a quick glance away should **not** trigger it (grace).
7. Return to normal → the state **de-escalates** after `stateExitSeconds`.
8. Point out `logs/events.csv` and the on-screen timeline recording each
   transition.

---

## Chapter 9 — Honesty and ethics

A defining quality of this project is that **it never fakes a detection**. This is
both an engineering choice and an ethical one, and it's worth being able to explain
clearly.

### 9.1 Phone detection is evidence-based only

Turning your head is **not** proof of phone use. Many naive systems infer "phone"
from a downward head tilt and produce constant false accusations. This project
removed that path entirely. The `main.cpp` comment is explicit:

> "PHONE USE IS BASED ONLY ON ACTUAL OBJECT DETECTION EVIDENCE. Head pose / looking
> away / body movement NEVER imply phone use — they feed the distraction score
> only."

A phone is reported only when a **real YOLO object detector** sees a phone box,
that box is **temporally confirmed** across several cycles, and (optionally) it's
**near a hand**. Without a YOLO model, the detector honestly reports `NO PHONE`
(`available = false`), so turning your head can *never* become "phone use." Looking
down is *distraction*; it contributes to the distraction score, not the phone
signal.

### 9.2 Smoking and seat belt report UNKNOWN

Standard COCO YOLO has **no cigarette class and no seat-belt class.** Rather than
guess from hand or head movement, both detectors return **`UNKNOWN`** until a
custom model is supplied:

- `SmokingDetector` returns `Unknown` unless `classIdCigarette >= 0` and a real
  cigarette detection is available. The full multi-cue logic is written and waiting.
- `SeatBeltDetector` computes the torso region but returns `Unknown` unless
  `classIdSeatbelt >= 0`, and **never** says "not worn" just because the belt isn't
  visible.

The default types default to `Unknown`, and the config ships `class_id_cigarette:
-1` and `class_id_seatbelt: -1` — the `-1` literally means "no such class in the
loaded model." The docs put it bluntly: "Never set an invalid COCO id to fake a
class."

### 9.3 The monitoring-quality gate

Even for the features that *do* work, the system refuses to over-claim. If the view
is too dark, the face too small/far, or the head too turned, the `MonitoringQuality`
gate reports **MONITORING QUALITY LOW** and the RiskEngine suppresses the
drowsiness evidence — so a bad view produces "I can't tell," not a false "DRIVER
DROWSY."

### 9.4 The AIS-184 wording

Always use the project's careful phrasing: it is **"designed *with reference to*
AIS-184"** — a requirement-oriented research/demo prototype, **not** type-approved
or certified. The official standard PDF couldn't even be retrieved (HTTP 403), so
every clause-level claim in the docs is tagged `VERIFY-OFFICIAL`. The thresholds
are prototype defaults, not legal limits.

### 9.5 Privacy

All processing is local; frames are analysed and discarded; there is **no
face-recognition or biometric identity database**. `--privacy` (or `privacy_mode`)
disables all disk logging, and logs contain metadata only — never images.

---

## Chapter 10 — Troubleshooting and how to extend it

### 10.1 Troubleshooting table

| Symptom | Likely cause | Fix |
|---|---|---|
| Startup shows `Detection backend: Haar cascade` (no EAR/yawns) | dlib not compiled in | rebuild with dlib present (install `libdlib-dev` or clone `dlib/` so `add_subdirectory` compiles it); delete `build/` and reconfigure |
| `Could not open camera index N` | wrong index, or camera in use | run `--list-cameras`; close Zoom/Teams; try another `--camera` index; check the `video` group permission |
| `Could not locate Haar cascades` | run from the wrong directory | run from the repo root, or pass `--cascades data` |
| `imshow` aborts with a Qt/Wayland error | no display (board/SSH) | add `--headless`, and `--stream` to watch remotely |
| Video lags badly on the board | slow CPU + buffering + YOLO | use `--fast` (or `--no-phone`); the grabber already drops stale frames |
| Can't connect to the stream in the browser | wrong IP/port, or different network | confirm the board IP with `hostname -I`; ensure the same LAN; check the `--stream` port; try `http://<ip>:8080/` |
| Face lost in low light | insufficient lighting | improve front lighting; the quality gate will say "low light" |
| Erratic state switching | thresholds too twitchy | raise `state_enter_seconds` / `state_exit_seconds` |
| Model file "not found" warning | model missing from `models/` | run the download script, or pass the explicit `--model` / `--phone-model` path |
| Link error re `opencv_ts`/`opencv_superres` (board SDK) | SDK packaging quirk | add the missing symlink in the sysroot so OpenCV's CMake config resolves |

### 10.2 How to extend it

**Add a cigarette or seat-belt model.** No code changes are needed beyond the model
and the class IDs:

1. Train or obtain a YOLO model that includes the class (cigarette or seat belt),
   exported to ONNX.
2. Set the matching `class_id_cigarette` / `class_id_seatbelt` in `config.json` to
   that model's class index (any value ≥ 0).
3. Point the detector at the model. The existing `SmokingDetector` /
   `SeatBeltDetector` logic then consumes the detections automatically.

**Use the PFLD ONNX landmark option** for angle-robust landmarks. dlib's 68 points
assume a near-frontal face and degrade at steep mounts. Drop a `models/pfld.onnx`
(input `1×3×112×112`, output `1×136` normalised (x,y) pairs) into `models/` or pass
`--landmark-model`; the `FaceTracker` will run it via OpenCV DNN and print
`Detection backend: ONNX 68-pt landmarks`. The single most effective fix, though,
is a more frontal camera mount.

**NPU acceleration path.** The i.MX 93 has an Ethos-U NPU. On a laptop CPU a
YOLOv5s inference is ~100–200 ms, so it runs every 4th frame. For production on the
board, a **quantized** model on the Ethos-U NPU is the intended acceleration path
for both YOLO and landmarks — far faster than CPU inference.

**Enable phone detection (no ONNX Runtime needed).** Because YOLO runs through
OpenCV's own `cv::dnn`, you already have everything except the model: put a
`yolov5s.onnx` / `yolov8n.onnx` in `models/` (or pass `--phone-model`), rebuild is
not even required, and the phone state machine comes alive.

---

## Chapter 11 — Project recap and viva questions

### 11.1 One-paragraph recap

This is a real-time Driver Monitoring System written in C++ using OpenCV and dlib.
A background thread grabs the newest camera frame; the `FaceTracker` finds the
driver's face and 68 landmarks and computes EAR, MAR, and head pose. A
`MonitoringQuality` gate decides whether the view is even reliable. The
`DrowsinessDetector` derives sustained closure, PERCLOS, blinks and yawns; the
`DistractionDetector` fuses head pose and approximate gaze; the `ObjectDetector`
does evidence-based YOLO phone detection. The `RiskEngine` blends these with
configurable weights, applies per-signal floors and a hysteresis state machine, and
outputs one of seven driver states plus a 0–100 risk score. The `AlertManager`
raises visual + audible warnings, the `EventLogger` records metadata, and the
`Dashboard` renders a professional HUD — shown in a window on a laptop or streamed
as MJPEG-over-HTTP from the headless i.MX 93 board. It is designed with reference
to AIS-184 but is explicitly a research/demo prototype, and it never fakes a
detection.

### 11.2 Viva questions and short answers

**Q1. What is EAR and why use it?**
Eye Aspect Ratio — vertical eyelid distance over horizontal eye width from the
landmarks. It's high when the eye is open and drops toward 0 when it closes, so it's
a cheap, reliable eye-closure signal.

**Q2. What is PERCLOS?**
The percentage of a rolling time window (60 s here) during which the eyes are
closed. It's the standard fatigue metric, and it's computed from timestamps so a
varying frame rate doesn't distort it.

**Q3. How does the system tell a blink from drowsiness?**
By duration, using an edge-triggered timer. A short closure (0.06–0.40 s) is a
blink; ≥ 0.5 s is a long blink; sustained closure past `eye_closed_drowsy_seconds`
is drowsy, and past `eye_closed_alarm_seconds` is a critical microsleep.

**Q4. How are yawns detected?**
Via the Mouth Aspect Ratio (MAR) with an adaptive threshold (learned closed-mouth
baseline + delta), a minimum open duration, and a dip-tolerance so landmark jitter
during a yawn doesn't reset the timer.

**Q5. How is head pose computed?**
`cv::solvePnP` matches six 2D landmarks (nose, chin, eye and mouth corners) to a
generic 3D face model to recover yaw/pitch/roll, which are then EMA-smoothed.

**Q6. Why is gaze called "approximate"?**
dlib's 68 points have no iris landmark, so gaze is estimated from the darkest blob
(pupil) offset inside each eye box — a coarse visual cue, not calibrated eye
tracking.

**Q7. How does phone detection avoid false positives?**
It uses only real YOLO object evidence — a phone box, temporally confirmed over
several cycles, optionally fused with a nearby hand. Head pose never implies a
phone. Without a model it reports `NO PHONE`.

**Q8. Why do smoking and seat belt say UNKNOWN?**
COCO YOLO has no cigarette or seat-belt class. Rather than guess, the detectors
return `UNKNOWN` until a custom model and its class ID are supplied. They never
fake a result.

**Q9. What does the RiskEngine do?**
It fuses the drowsiness, distraction and phone scores with configurable weights,
applies per-signal floors so a single strong signal isn't averaged away, smooths
the result, and commits a driver state through a hysteresis state machine.

**Q10. What is the hysteresis state machine for?**
To prevent flicker. A candidate state must persist for `stateEnterSeconds` to
escalate or `stateExitSeconds` to recover before it takes effect, so the displayed
state is stable.

**Q11. What is the MonitoringQuality gate?**
A reliability check (light, face size, confidence, landmarks, head rotation). If
monitoring is unreliable it reports "monitoring quality low" and the RiskEngine
suppresses drowsiness evidence — no false alarms when the driver can't be seen.

**Q12. Why the CameraGrabber thread?**
On a slow device, processing lags behind capture and the video drifts behind
reality. The grabber keeps only the newest frame and drops stale ones, bounding
latency; wall-clock timing keeps the metrics correct.

**Q13. Why headless and streaming on the board?**
The i.MX 93 has only a serial console, no monitor, so `cv::imshow` can't open a
window. `--headless` runs without a window and `--stream` serves the dashboard as
MJPEG-over-HTTP to watch in a browser on the same network.

**Q14. What does `--fast` change and why?**
It drops the CPU-heavy YOLO model, lowers the capture resolution, and runs face
detection less often — giving a smooth, low-latency demo on the embedded CPU.

**Q15. How would you cross-compile for the board?**
Source the Yocto SDK environment-setup script, then
`cmake -S . -B build-arm64 && cmake --build build-arm64 --parallel`, verify with
`file` that it's an aarch64 binary, and `scp` it to the board.

**Q16. Is this AIS-184 certified?**
No. It's designed *with reference to* AIS-184 as a research/demo prototype. It is
not type-approved or certified, and the thresholds are engineering defaults, not
legal limits.

**Q17. How is privacy handled?**
All processing is local, frames are discarded after analysis, there's no biometric
identity database, logs are metadata-only, and `privacy_mode` disables all disk
logging.

**Q18. Where do all the thresholds live and how do you change one?**
In `config/config.json`, each mapping to a documented field in `Config.hpp`. Edit
the JSON and restart — no rebuild needed.

---

*End of guide. Open any file named above alongside this document and re-read its
section — the code and the explanation are meant to be read together.*
