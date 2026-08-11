# DMS — Quick Revision & Interview Prep

**How to use this:** This is the *one* sheet to skim the night before your interview. It distills the four longer guides (Linux, Yocto/Embedded, Tools & Vision Science, and the DMS Project) into fast, high-signal bullets, tables and formulas — Part A is rapid revision, Part B is a large bank of interview Q&A you can actually say out loud. Aim to read the whole thing in ~30–45 minutes. Don't memorize word-for-word; understand *why each piece exists*. Above all, be honest about what the system can and cannot do (see the honesty statements) — that honesty is a selling point, not a weakness.

---

## Table of contents

**PART A — RAPID REVISION**
1. [The project in 60 seconds](#1-the-project-in-60-seconds)
2. [Linux essentials recap](#2-linux-essentials-recap)
3. [Embedded Linux & Yocto recap](#3-embedded-linux--yocto-recap)
4. [C++ / CMake recap](#4-c--cmake-recap)
5. [OpenCV & dlib recap](#5-opencv--dlib-recap)
6. [The vision math recap (high value)](#6-the-vision-math-recap-high-value)
7. [YOLO / object-detection recap](#7-yolo--object-detection-recap)
8. [Project architecture recap](#8-project-architecture-recap)
9. [Build / deploy / run recap](#9-build--deploy--run-recap)
10. [Everything cheat-sheet (one page)](#10-everything-cheat-sheet-one-page)

**PART B — INTERVIEW QUESTIONS WITH ANSWERS**
- [B1. "Tell me about your project"](#b1-tell-me-about-your-project)
- [B2. Linux / OS questions](#b2-linux--os-questions)
- [B3. Embedded / Yocto questions](#b3-embedded--yocto-questions)
- [B4. C++ / CMake / tools questions](#b4-c--cmake--tools-questions)
- [B5. Computer-vision / DMS-specific questions](#b5-computer-vision--dms-specific-questions)
- [B6. HR / behavioral questions](#b6-hr--behavioral-questions)
- [Final tips](#final-tips)

---

# PART A — RAPID REVISION

## 1. The project in 60 seconds

**Elevator pitch:** A real-time **Driver Monitoring System (DMS)** in **C++** using **OpenCV** and **dlib**. It watches a driver through a camera, finds the face and **68 facial landmarks**, and computes **drowsiness** (eye closure/EAR, PERCLOS, blinks, yawns/MAR), **distraction/attention** (head pose + approximate gaze), and **phone use** (real YOLO object detection). A **RiskEngine** fuses these into a single 0–100 risk score and one of seven driver states, shown on a professional live **dashboard**. It runs on a laptop webcam and is **cross-compiled to the NXP i.MX 93 EVK** board, where it runs **headless** and **streams** the dashboard over the network. All processing is local — no cloud.

**Honesty statement (say this):** It is **designed *with reference to* AIS-184** (India's driver drowsiness standard) but is **NOT certified or type-approved** — thresholds are sensible engineering defaults, not legal limits. **Phone detection is evidence-based only** (a real detected phone box, never inferred from head turning). **Smoking and seat-belt are honestly reported as UNKNOWN** because COCO has no such class and the system never fakes a detection.

**Seven driver states:** `SAFE · ATTENTION REQUIRED · DROWSY · DISTRACTED · PHONE USAGE · HIGH RISK · CRITICAL`

---

## 2. Linux essentials recap

### ~30 most important commands

| Command | One-line meaning |
|---|---|
| `pwd` | Print working directory ("where am I?") |
| `ls` | List files (`ls -lah` = long, all/hidden, human sizes) |
| `cd` | Change directory (`cd ..` up, `cd` home, `cd -` previous) |
| `cp` | Copy files (`cp -r` for folders) |
| `mv` | Move **or** rename (renaming = moving) |
| `rm` | Delete — **no undo** (`rm -r` folder, `rm -rf` = force, dangerous) |
| `mkdir` | Make a directory (`mkdir -p a/b/c` makes parents) |
| `cat` | Print a whole (short) file |
| `less` | Scroll a long file (`q` to quit) |
| `grep` | Search **text inside** files (`grep -rn "x" .`) |
| `find` | Search for files **by name** (`find . -name "*.cpp"`) |
| `chmod` | Change permissions (`chmod 755 run.sh`) |
| `chown` | Change owner (`sudo chown me file`) |
| `sudo` | Run one command as root (superuser) |
| `apt update` | Refresh the package catalog (installs nothing) |
| `apt install` | Install a package + dependencies |
| `ps` | List processes (`ps aux`) |
| `kill` | Stop a process by PID (`kill -9` = force) |
| `df` | Disk free (`df -h`) |
| `ip addr` | Show network interfaces / IP (also `hostname -I`) |
| `ssh` | Log into another machine remotely (`ssh root@IP`) |
| `scp` | Secure copy files over the network |
| `source` | Run a script **in the current shell** (`. script`) — vars persist |
| `nano` | Friendly terminal text editor (`^O` save, `^X` exit) |
| `man` | Read a command's manual (`q` to quit) |
| `&` | Run a command in the **background** |
| `\|` | **Pipe** — feed one command's output into the next |
| `>` | Redirect output to a file (overwrite; `>>` appends) |
| `&&` | Run next command **only if** the previous succeeded |

### Key concepts

- **Filesystem hierarchy:** one tree from `/` (no drive letters). `/home` (your files), `/etc` (config), `/usr` (installed programs/libs), `/dev` (devices), `/opt` (add-on software, e.g. the SDK).
- **Permissions `rwx`:** read/write/execute, for **owner / group / others**. Numeric: r=4, w=2, x=1. `755` = rwx / r-x / r-x. `644` = rw- / r-- / r--.
- **root / sudo:** root is the all-powerful superuser; normal users prefix `sudo` to run one command as root. Respect it — a `sudo` mistake can break the whole system.
- **Devices are files:** `/dev/video0` = the camera; `/dev/ttyUSB2` = the board's serial console. Access needs group membership: **`video`** group (camera), **`dialout`** group (serial). Add with `sudo usermod -aG video hemanth` (log out/in after).
- **Environment variables:** named values programs read. `$HOME`, `$USER`, `$PATH` (folders searched for commands). Read with `$` (`echo $PATH`). `export VAR=value` shares it with launched programs.
- **PATH:** colon-separated list of directories; when you type `ls`, Bash searches these in order to find the program.

---

## 3. Embedded Linux & Yocto recap

### Must-know terms

| Term | Plain meaning |
|---|---|
| **Embedded system** | A computer built into a larger device for one dedicated task (the DMS in a vehicle). |
| **EVK (Evaluation Kit)** | A ready-made dev board built around an SoC, with connectors (USB, Ethernet, camera). |
| **SoC (System on a Chip)** | One chip integrating CPU, memory controllers, graphics, NPU, etc. |
| **aarch64 vs x86-64** | Two **incompatible** CPU instruction sets. Board = aarch64 (Arm 64-bit / arm64 / armv8a); laptop = x86-64 (Intel/AMD). |
| **Cross-compilation** | Build on one architecture (x86-64 laptop) *for* another (aarch64 board). |
| **Host vs target** | Host = where you build (laptop); target = where it runs (board). |
| **Toolchain** | The coordinated build tools: compiler, assembler, **linker**, etc. A *cross*-toolchain targets a different arch. |
| **Sysroot** | A host-side mirror of the target's filesystem (its headers + libraries) so the cross-compiler links against the board's libs. `$SDKTARGETSYSROOT`. |
| **Yocto Project** | A build **system** that *builds a custom Linux distro* for your hardware — not a distro itself. |
| **Poky** | Yocto's reference distribution you customize from (source of `-poky-linux`). |
| **OpenEmbedded** | The build framework + recipe collection underneath Yocto. |
| **bitbake** | Yocto's build engine — reads recipes and cross-builds everything (`bitbake imx-image-full`). |
| **Layer (`meta-*`)** | A folder grouping related recipes + config (e.g. `meta-imx`). |
| **Recipe (`.bb`)** | Instructions to build **one** software component. |
| **BSP (Board Support Package)** | Everything needed to boot Linux on a specific board: bootloader, kernel, device tree. |
| **meta-imx** | NXP's Yocto layers providing the i.MX BSP. |
| **Image / rootfs** | The bootable OS package; rootfs = all files/programs/libs of the running system. Here: **`imx-image-full`** (includes **OpenCV 4.10**), kernel **6.6**, scarthgap. |
| **U-Boot** | The board's bootloader — first program at power-on; loads the kernel. |
| **NPU / Ethos-U** | Neural Processing Unit — the i.MX 93's Arm **Ethos-U** AI accelerator (needs quantized `.tflite` + `vela` compiler; OpenCV DNN does **not** use it). |

### Cross-compile flow (5 lines)

```bash
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux   # activate SDK (sets CC, CXX, sysroot)
cmake -S . -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake
cmake --build build-arm64 --parallel                                            # compile + link
file build-arm64/dms          # expect: ELF 64-bit LSB executable, ARM aarch64
scp build-arm64/dms root@192.168.1.173:~/deploy/                                # copy to the board
```

### Why you can't just copy a laptop binary to the board

A compiled binary contains machine code for **one** architecture. The laptop produces **x86-64** code; the board's CPU is **aarch64** and can't understand those bytes — you get `Exec format error`. The fix is not to copy harder but to **cross-compile for aarch64** in the first place, using the Yocto SDK's cross-toolchain and the board's matching sysroot.

---

## 4. C++ / CMake recap

### C++ key facts

- **Compiled language:** translated once, ahead of time, into machine code → runs at full speed with no interpreter (vs Python, which is interpreted/slower). Chosen for **speed** (~30 FPS), **small embedded footprint** (i.MX 93), **direct hardware access**, and **predictable memory** (no garbage-collector pauses).
- **`.hpp` (header) vs `.cpp` (source):** header = **declarations** ("what exists" — the menu); source = **definitions** ("how it works" — the kitchen). `#pragma once` guards a header against double-inclusion.
- **`#include`:** pastes a header in. `"..."` = our headers; `<...>` = system/library headers.
- **struct vs class:** struct bundles related values (fields); class also bundles **methods** and hides internals (`private`) behind a public interface (**encapsulation**). An **object** is a built instance.
- **namespace `dms`:** a labeled container of names to avoid clashes; `::` = "belongs to" (`cv::Mat`, `std::vector`, `dms::Config`).
- **Reference (`&`) and `const`:** `const cv::Mat& frameBGR` = "another name for the frame (no copy — fast), promise not to modify it." A non-const `&` lets a function write a result back (return multiple values).
- **`std::vector<T>`:** a resizable list (e.g. `std::vector<cv::Point2f>` for the 68 landmarks; index from 0, `.size()` for count).
- **Compile + link:** compiler turns each `.cpp` into an object file; **linker** stitches object files + libraries (OpenCV, dlib) into the final executable `dms`.

### CMake basics

| Item | Purpose |
|---|---|
| `CMakeLists.txt` | The build recipe. |
| `cmake_minimum_required` / `project(... LANGUAGES CXX)` | Version + name the project (C++). |
| `set(CMAKE_CXX_STANDARD 17)` | Use C++17. |
| `find_package(OpenCV REQUIRED COMPONENTS ...)` | Locate a library + the modules used. |
| `add_subdirectory(dlib/...)` | Compile dlib from source as part of the build (most reliable path). |
| `add_executable(dms src...)` | Declare the program + its `.cpp` files. |
| `target_include_directories` | Tell the compiler where headers live. |
| `target_link_libraries(dms ... ${OpenCV_LIBS})` | **Link** libraries into the program. |
| `target_compile_definitions(dms PRIVATE DMS_HAVE_DLIB)` | Set a compile flag the code checks with `#ifdef`. |
| **Out-of-source build** | Build into a separate `build/` folder; delete it to start clean, source stays untouched. |

---

## 5. OpenCV & dlib recap

### OpenCV

- **`cv::Mat`** = an image = a grid of pixels. Properties: **rows** (height), **cols** (width), **channels** (1 = gray, 3 = color). Color is stored **BGR** (Blue, Green, Red), each 0–255 — hence `frameBGR`.
- **`cv::VideoCapture`** — open a webcam by index (`cv::VideoCapture(0)` → `/dev/video0`); `cap.read(frame)` grabs the newest frame.
- **`cv::imshow` / `cv::waitKey`** — show a window; `waitKey(1)` also *refreshes* the window and reads a key. (Fails on the headless board — no display.)
- **`cv::imencode(".jpg", ...)`** — compress a frame to JPEG **in memory** (for streaming); `cv::imwrite` saves to disk.
- **`dnn` module** — loads and runs neural networks (YOLO) with **no separate runtime** needed.
- **Modules used:** core, imgproc, imgcodecs, objdetect (Haar fallback), highgui, videoio, calib3d (`solvePnP`), dnn.

### dlib

- **HOG face detector** (`get_frontal_face_detector`) — **Histogram of Oriented Gradients**: tallies edge directions in cells to find the face's edge pattern. Robust, needs no color, expects a roughly frontal face.
- **68-point shape predictor** (`shape_predictor_68_face_landmarks.dat`, ~96 MB) — places 68 labeled points on the face; the raw material for EAR/MAR/head-pose.

### The 68-landmark index map (iBUG-68, count from 0)

| Indices | Feature | Key points |
|---|---|---|
| 0–16 | Jawline | **8** = chin bottom |
| 17–21 / 22–26 | Right / left eyebrow | |
| 27–30 | Nose bridge | **30** = nose tip |
| 31–35 | Lower nose | |
| **36–41** | **Right eye** | EAR (right) |
| **42–47** | **Left eye** | EAR (left) |
| 48–59 | Outer lip | **48** = right corner, **54** = left corner |
| 60–67 | Inner lip | **61,62,63 ↔ 67,66,65** = MAR vertical gaps |

Head-pose uses six well-spread points: **30, 8, 36, 45, 48, 54**.

---

## 6. The vision math recap (high value)

### EAR — Eye Aspect Ratio (eye closure / blinks)

```text
        ‖p2 − p6‖ + ‖p3 − p5‖
EAR =  ─────────────────────────
              2 · ‖p1 − p4‖
```

- **Intuition:** open eye is tall, closed eye is flat. Sum the two **vertical** eyelid gaps, divide by twice the **horizontal** width. It's a ratio, so distance-to-camera doesn't matter.
- Open ≈ 0.30, closed ≈ 0.10. Fallback threshold `earThreshold = 0.21`.
- **Self-calibration:** learns each driver's own open-eye baseline (rolling max EAR); closed threshold = `baseline × earCloseRatio (0.62)`, clamped 0.15–0.30. Uses **smoothing** + **hysteresis** (enter closed below threshold, leave only above `threshold × 1.12`) to stop flicker.

### Blinks / closure / microsleep

Timed from when EAR crosses the threshold: normal **blink** 0.06–0.40 s; **long blink** ≥ 0.50 s (fatigue cue); sustained closure ≥ `eyeClosedDrowsySeconds (0.60)` → **Drowsy**; ≥ `eyeClosedAlarmSeconds (1.20)` → **microsleep → Critical**.

### PERCLOS — % eye closure over time

- **Intuition:** the fraction of a rolling **60-second** window with eyes closed — one of the most validated fatigue metrics.
- `PERCLOS = closedFrames / totalFrames` in the window; computed from **timestamps**, not frame counts (frame-rate-proof). Bands: `perclosWarn = 0.15` (building fatigue), `perclosAlarm = 0.30` (drowsy).

### MAR — Mouth Aspect Ratio (yawns)

```text
       ‖p61−p67‖ + ‖p62−p66‖ + ‖p63−p65‖
MAR = ────────────────────────────────────
                 3 · ‖p48 − p54‖
```

- **Intuition:** closed mouth is flat (MAR≈0), a yawn is tall. Three inner-lip vertical gaps over 3× the **stable outer** mouth width (48↔54). A yawn is only counted after the mouth stays open ≥ `yawnMinSeconds (0.55)` (distinguishes a yawn from talking), with an adaptive baseline + dip tolerance.

### Head pose via `solvePnP` (yaw / pitch / roll)

- **Intuition:** **PnP = Perspective-n-Point** — given known **3-D** model points and where they appear in the **2-D** image, recover the head's rotation. Uses 6 landmarks (30, 8, 36, 45, 48, 54) vs a generic 3-D face model; `cv::Rodrigues` → rotation matrix → Euler angles.
- **Yaw** = turn left/right; **pitch** = nod up/down; **roll** = tilt to shoulder. EMA-smoothed. Thresholds: `headAwayYawDegrees = 25`, `headDownPitchDegrees = 18`.
- **Why an angled mount needs neutral-pose calibration:** in a car the camera is off to the side, so a driver looking at the road reads as "turned." A 3-second **calibration** averages the neutral pose and **subtracts** it, so only *departures* from normal count as looking away.

### Gaze approximation

- dlib's 68 points have **no iris**, so gaze is **coarse**: crop the eye, blur it, find the **darkest blob** (pupil) centroid, measure its offset from the eye center normalized to −1..+1. Over `gazeOffThreshold (0.28)` → looking away. Only **contributes** to distraction; never triggers a hard alarm alone; skipped when eyes are closed.

---

## 7. YOLO / object-detection recap

- **Neural network / model:** a program that **learns from examples** (tunable weights). A **model** = the trained network's weights saved to a file. Training happens once offline (often in Python); the DMS only does **inference** (runs a pre-trained model), live, in C++.
- **YOLO = "You Only Look Once":** a fast, single-pass **object detector**. For each object it outputs a **bounding box** + a **class ID** + a **confidence** (0–1).
- **COCO dataset:** the standard ~80-category set YOLO is trained on. **Cell phone = class 67**, person = 0.
- **Cigarette / seat belt are NOT in COCO** → set to `-1` → features report **UNKNOWN** (never faked).
- **ONNX** (`.onnx`): a portable, universal model file format ("PDF for neural nets"). Both YOLOv5 (85-col, has objectness) and YOLOv8 (84-col) auto-detected. Project uses `yolov5s.onnx` / `yolov8n.onnx`.
- **cv::dnn runs it — no separate runtime:** pipeline = **letterbox** (resize to 640×640 keeping aspect ratio, gray pad) → **blobFromImage** (scale ÷255, swap R/B) → **`forward()`** (inference) → **decode** (best class + confidence ≥ 0.45, map box back) → **NMS** (Non-Maximum Suppression removes overlapping duplicates using IoU cutoff 0.45).
- **Phone "in use":** requires **temporal confirmation** (seen in several recent cycles), then a state machine `NoPhone → Possible → Detected → UsageConfirmed`; optionally **fused with a nearby hand** to upgrade to UsageConfirmed. Runs only every 4th frame (`phoneDetectEveryNFrames`). **Golden rule: phone is decided ONLY by a real detected phone box — never inferred from head pose.**

---

## 8. Project architecture recap

**Data flow (one frame, ~30×/sec):**

`camera → CameraGrabber (latest frame, drops stale) → FaceTracker (face + 68 landmarks + EAR/MAR/head-pose) → { GazeEstimator, ObjectDetector(phone via YOLO), MonitoringQuality } → DrowsinessDetector + DistractionDetector → RiskEngine (weighted fusion + hysteresis) → AlertManager + EventLogger → Dashboard → window OR headless snapshot OR MJPEG stream`

### Main source files and their one-line jobs

| File | Job |
|---|---|
| `main.cpp` | Entry point + per-frame loop; parses config/CLI, wires modules, calibration, output. |
| `Config.hpp` | Every tunable threshold as a documented C++ field (nothing else hard-codes numbers). |
| `Types.hpp` | Shared data structs (`FaceObservation`, `PhoneResult`…) + state enums. |
| `ConfigManager.cpp` | Load/save `config/config.json` (via OpenCV FileStorage; missing keys keep defaults). |
| `CameraGrabber.hpp` | Background-thread camera reader that keeps only the newest frame (anti-lag). |
| `FaceTracker.cpp` | Face detection + 68 landmarks + EAR/MAR + head pose (`solvePnP`). |
| `GazeEstimator.cpp` | Approximate pupil-offset gaze. |
| `MonitoringQuality.cpp` | Reliability gate — refuses to assert drowsiness on a bad view. |
| `DrowsinessDetector.cpp` | EAR calibration, PERCLOS, blinks, yawns → 4 levels + score. |
| `DistractionDetector.cpp` | Head pose + gaze (+ phone) fusion → attention level. |
| `ObjectDetector.cpp` | YOLO phone detection via cv::dnn + phone state machine. |
| `SmokingDetector.cpp` / `SeatBeltDetector.cpp` | Honest UNKNOWN until a custom model is supplied. |
| `RiskEngine.cpp` | Weighted fusion → 0–100 risk + DriverState + hysteresis state machine. |
| `AlertManager.cpp` | Alert levels + rate-limited audible beep. |
| `EventLogger.cpp` | In-memory timeline + `logs/events.csv` (metadata only, never images). |
| `Dashboard.cpp` | Renders the HUD (camera view + status panel + gauge + timeline). |
| `MjpegServer.cpp` | Tiny built-in web server streaming the dashboard as MJPEG-over-HTTP. |

**Fusion weights:** `wDrowsiness = 0.40`, `wDistraction = 0.30`, `wPhone = 0.20`, `wYawn = 0.10`. RiskEngine also applies **per-signal floors** (a strong single signal can't be averaged away) and a **hysteresis state machine** (`stateEnterSeconds 0.6` to escalate, `stateExitSeconds 1.5` to recover).

---

## 9. Build / deploy / run recap

### Laptop build

```bash
sudo apt-get install -y libopencv-dev libdlib-dev libblas-dev liblapack-dev
./scripts/download_landmark_model.sh          # 68-point .dat (~96 MB)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/dms
```

### Cross-compile for the board

```bash
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux
cmake -S . -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake
cmake --build build-arm64 --parallel
file build-arm64/dms                          # expect ARM aarch64
scp build-arm64/dms root@192.168.1.173:~/deploy/
```

### Run modes & key flags

| Flag | What it does | Why it exists (one line) |
|---|---|---|
| `--camera <n>` | Choose the camera index | Match the actual webcam (`--list-cameras` to probe). |
| `--headless` | No GUI window; prints status + saves `dms_frame.jpg` | The board has no monitor; `imshow` fails on Qt/Wayland. |
| `--stream [port]` | Serve the dashboard as MJPEG-over-HTTP (default 8080) | Watch the live demo in a browser at `http://192.168.1.173:8080/`. |
| `--fast` | No YOLO, lower resolution, less-frequent detection | The board CPU is slow → YOLO lags; `--fast` keeps it real-time. |
| `--no-phone` | Disable only the YOLO phone detector | Keep everything else but drop the heaviest step. |

Board demo command: `./dms --camera 0 --stream --headless --fast`. Runtime keys (windowed): `q`/ESC quit, `d` developer mode, `c` recalibrate.

---

## 10. Everything cheat-sheet (one page)

### Linux / board

```bash
ls -lah              # list all, long, human sizes          grep -rn "text" .    # search inside files
cd ~/dmsc-           # go to project                        find . -name "*.cpp" # find files by name
chmod 755 run.sh     # rwx/r-x/r-x                           ps aux | grep dms    # find the process
sudo apt install X   # install software                     kill -9 <PID>        # force-stop
df -h ; free -h      # disk ; memory                        hostname -I          # my IP
ssh root@192.168.1.173                # log into the board
scp build-arm64/dms root@192.168.1.173:~/deploy/            # copy binary to board
sudo minicom -D /dev/ttyUSB2 -b 115200                      # serial console (Ctrl-A X to exit)
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux   # activate SDK
```

### Build & run the DMS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel   # laptop build
./build/dms                                   # laptop, full features, windowed
./build/dms --list-cameras                    # find webcam index
# cross-compile:
cmake -S . -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake
cmake --build build-arm64 --parallel && file build-arm64/dms      # verify ARM aarch64
# on the board:
./dms --camera 0 --stream --headless --fast   # then browse http://192.168.1.173:8080/
```

### Numbers worth remembering

| Value | Meaning |
|---|---|
| EAR 0.21 / ratio 0.62 | fallback / adaptive eyes-closed threshold |
| 0.60 s / 1.20 s | closure → Drowsy / Critical (microsleep) |
| PERCLOS 60 s, 0.15 / 0.30 | window, warn / alarm bands |
| MAR yawn ≥ 0.55 s | min open duration for a yawn |
| Yaw 25° / Pitch 18° / Gaze 0.28 | head-away / head-down / gaze-off thresholds |
| Weights 0.40/0.30/0.20/0.10 | drowsy / distraction / phone / yawn |
| COCO 67 / 0 | cell phone / person |
| Port 8080, IP 192.168.1.173 | stream / board |
| OpenCV 4.10, kernel 6.6, aarch64 | board software stack |

---

# PART B — INTERVIEW QUESTIONS WITH ANSWERS

## B1. "Tell me about your project"

**Q1. Give me a 30-second summary of your project.**
I built a real-time Driver Monitoring System in C++ using OpenCV and dlib. It watches a driver through a camera, finds the face and 68 facial landmarks, and detects drowsiness (eye closure, PERCLOS, yawns), distraction (head pose and gaze), and phone use (YOLO object detection). A risk engine fuses all of that into a single 0–100 score and a driver state shown on a live dashboard. It runs on a laptop and I cross-compiled it to an NXP i.MX 93 embedded board, where it runs headless and streams the dashboard to a browser.

**Q2. Give me the 2-minute version — the architecture and what you did.**
The system is a pipeline of single-responsibility modules. A background CameraGrabber thread always hands the newest frame to the FaceTracker, which uses dlib's HOG detector and a 68-point predictor to get landmarks, then computes EAR (eye openness), MAR (mouth openness), and head pose via `solvePnP`. A MonitoringQuality gate decides whether the view is even reliable. The DrowsinessDetector turns EAR/MAR over time into PERCLOS, blinks and yawns; the DistractionDetector fuses head pose and approximate gaze; the ObjectDetector runs a YOLO ONNX model through OpenCV's dnn module for evidence-based phone detection. The RiskEngine blends these with configurable weights, applies per-signal floors and a hysteresis state machine, and outputs one of seven driver states. The AlertManager raises visual and audible alerts, the EventLogger writes metadata-only logs, and the Dashboard renders the HUD. On the board it runs headless and streams MJPEG-over-HTTP. I made it honest by design — it never fakes a detection, and it's designed with reference to AIS-184 but is not certified.

**Q3. What was the hardest part?**
Getting it to run well on the embedded board. Three things: cross-compilation with the Yocto SDK (and a packaging quirk where OpenCV's CMake config referenced test libraries missing from the sysroot, which I fixed with a symlink); the board having no display, so `cv::imshow` failed on Qt/Wayland — I added a headless mode and an MJPEG stream so you watch it in a browser; and the slow CPU making YOLO lag, which I solved with a `--fast` mode and a frame-dropping grabber so latency can't grow unbounded.

**Q4. What did you learn, and what was your specific contribution?**
I learned the full embedded workflow end to end — Linux, cross-compilation, Yocto concepts, and real-time computer vision — plus the discipline of building an honest system that reports "unknown" instead of guessing. My contribution was the vision and fusion logic: implementing EAR/PERCLOS/MAR, the self-calibrating thresholds with hysteresis, the head-pose neutral calibration for an angled mount, the evidence-based phone state machine, and getting the whole thing cross-compiled and demoed live on the i.MX 93.

---

## B2. Linux / OS questions

**Q5. What is Linux, and what is the kernel?**
Linux is a free, open-source operating system — the software that manages a computer's hardware and runs programs. Technically "Linux" refers to just the **kernel**, the innermost core that talks directly to hardware: it manages processes, memory, devices, and the filesystem. A full usable system (like Ubuntu) is the kernel plus tools, packaged as a distribution.

**Q6. What is a shell?**
A shell is a program that reads text commands you type and asks the OS to carry them out — a way to control the computer by typing instead of clicking. The common one on Ubuntu is Bash. On the embedded board, with no mouse or desktop, the shell is the only way to work.

**Q7. Absolute vs relative path?**
An absolute path starts from the top `/` and spells out the full route (`/home/hemanth/dmsc-`) — it works from anywhere. A relative path is interpreted from your current directory (`dmsc-` or `../build`). Absolute is like a full postal address; relative is like "the room next door."

**Q8. What does `chmod 755` mean?**
It sets permissions using numbers where read=4, write=2, execute=1, for owner/group/others. 755 = owner rwx (7), group r-x (5), others r-x (5) — the owner can change and run it, everyone else can read and run it. It's the typical setting for a program or a folder.

**Q9. What is `sudo`?**
`sudo` runs a single command as the all-powerful root (superuser). You normally work as a limited user for safety; when a task genuinely needs system-wide power — installing software, editing system files — you prefix it with `sudo` and enter your own password. It should be used carefully because a `sudo` mistake can break the whole system.

**Q10. How do you find a file, and how do you find a running process?**
To find files by name I use `find . -name "*.cpp"`, or `grep -rn "text" .` to search inside files. To find a process I use `ps aux | grep dms`, which shows its PID; I can then stop it with `kill <PID>` (or `kill -9` to force).

**Q11. What is the difference between `ssh` and `scp`?**
Both use the same secure SSH channel. `ssh` opens a remote shell — it logs you into another machine so you can run commands there (`ssh root@192.168.1.173`). `scp` (secure copy) transfers files over that channel (`scp dms root@192.168.1.173:~/deploy/`). I used ssh to run the DMS on the board and scp to copy the binary over.

**Q12. What is an environment variable?**
A named value stored in the shell that programs can read, like a system setting. Examples: `$HOME`, `$USER`, and `$PATH` (the list of folders searched to find commands). You read one with `$` (`echo $PATH`) and share it with programs using `export`. Sourcing the Yocto SDK script sets a batch of them (`CC`, `CXX`, the sysroot path) so the build targets the board.

**Q13. Why does `/dev/video0` and `/dev/ttyUSB2` exist — what's the idea?**
On Linux "everything is a file," including hardware. `/dev/video0` is the camera and `/dev/ttyUSB2` is the board's serial console over a USB-to-serial adapter. Programs open these files to talk to the hardware. To use them without root you must be in the right group — `video` for the camera, `dialout` for the serial port.

---

## B3. Embedded / Yocto questions

**Q14. What is cross-compilation and why did you need it?**
Cross-compilation is building a program on one CPU architecture *for* a different one. My laptop is x86-64 but the i.MX 93 board is aarch64 (Arm 64-bit). A binary compiled for x86-64 can't run on aarch64 — you get an "Exec format error." So I compile on the fast laptop but target the board's architecture using the Yocto SDK's cross-compiler. It's also necessary because the board is too slow and storage-limited to compile a big C++/OpenCV project itself.

**Q15. Host vs target?**
Host is the machine where you build — my x86-64 laptop. Target is the machine where the program actually runs — the aarch64 i.MX 93 board.

**Q16. What is a toolchain and a sysroot?**
A toolchain is the coordinated set of build tools — compiler, assembler, linker, and utilities. A cross-toolchain targets a different architecture than the host. A sysroot is a host-side copy of the target's filesystem — its headers and libraries — so the cross-compiler links against the board's OpenCV, not the laptop's. In the SDK these are `$SDKTARGETSYSROOT` (target libs) and `$OECORE_NATIVE_SYSROOT` (host-side tools).

**Q17. What is Yocto, bitbake, a recipe, and a layer?**
Yocto is a build **system** that builds a *custom Linux distribution* for your hardware — it's not a distro itself, it's a distro factory. **bitbake** is its build engine that reads recipes and cross-builds everything. A **recipe** (`.bb`) is the instructions to build one software component. A **layer** (`meta-*`) is a folder grouping related recipes and config — for example NXP's `meta-imx` layers provide the i.MX board support.

**Q18. What is a BSP?**
A Board Support Package — everything needed to boot Linux on a *specific* board: the right bootloader (U-Boot), a kernel with that board's drivers, and the device tree describing its hardware. It turns "Linux in general" into "Linux that boots on the i.MX 93 EVK."

**Q19. What's the difference between aarch64 and x86-64?**
They're two different, incompatible CPU instruction sets. x86-64 is Intel/AMD, used in laptops and servers. aarch64 (arm64/armv8a) is 64-bit Arm, used in phones and embedded boards like the i.MX 93. Machine code built for one is meaningless to the other, which is exactly why cross-compilation is required.

**Q20. How did you get OpenCV onto the board — did you have to build it?**
No. The board's Linux image was built with Yocto using NXP's `imx-image-full`, which already includes **OpenCV 4.10** in the root filesystem. So when the DMS starts on the board it finds the OpenCV shared libraries already in `/usr/lib`. My laptop's sysroot matched that same OpenCV 4.10, so the versions line up and the binary loads cleanly.

**Q21. What is an NPU, and did your system use it?**
An NPU is a Neural Processing Unit — hardware that runs neural-network math far faster and more efficiently than the CPU. The i.MX 93 has an Arm **Ethos-U** NPU. My system does **not** use it, because OpenCV's dnn module runs on the CPU and isn't wired to the Ethos-U. Using the NPU would require quantizing the model to int8 TensorFlow Lite and compiling it with the `vela` tool. That's why I added `--fast` — to stay real-time on the CPU. Porting to the NPU is the production optimization path.

**Q22. How do you talk to the board when it has no monitor?**
Two ways: over the network with `ssh root@192.168.1.173`, or over a serial console with `minicom -D /dev/ttyUSB2 -b 115200`. The serial console is the most reliable — it works even before networking is up, so you can watch the board boot. Both sides must agree on the 115200 baud rate.

---

## B4. C++ / CMake / tools questions

**Q23. Why C++ for this project?**
Because a DMS is a hard-real-time safety system. It has to process ~30 frames a second and still run face detection, landmark math and sometimes a neural network before the next frame. C++ compiles directly to machine code with no interpreter, so it's fast; it's small and self-contained for a constrained embedded board; it gives direct hardware access; and it has predictable memory with no garbage-collector pauses. Python is great for training models offline, but the live loop needs C++.

**Q24. Difference between a `.h`/`.hpp` and a `.cpp` file?**
The header declares *what* exists — the class names, function signatures, structs — like a menu. The `.cpp` source contains *how* it works — the actual code, like the kitchen. Other files only need to read the header to use a module, which keeps the code organized and lets files compile separately.

**Q25. What is a pointer, and what is a reference? (briefly)**
Both let you refer to data without copying it. A reference (`&`) is simply another name for an existing variable — it can't be null and doesn't need special syntax. A pointer is a variable that stores a memory address and can be reassigned or null. In this project I mostly use `const` references to pass big images cheaply — `const cv::Mat& frameBGR` avoids copying hundreds of thousands of pixels while promising not to modify the frame.

**Q26. What is OpenCV?**
OpenCV is a large, free computer-vision library. It provides ready-made building blocks for images and video: reading a webcam, converting colors, drawing, JPEG encoding, and even running neural networks through its dnn module. Everything lives in the `cv::` namespace. I used it for capture, all image math, the dashboard drawing, head-pose geometry, and YOLO.

**Q27. What is a `cv::Mat`?**
It's OpenCV's image type — a matrix of pixels. It has rows (height), cols (width), and channels (1 for grayscale, 3 for color). Color is stored in **BGR** order, not RGB — that's an OpenCV quirk, which is why my frame variables are named `frameBGR`.

**Q28. What does CMake do?**
CMake is a build-system generator. You describe the project once in `CMakeLists.txt` — the source files and the libraries needed — and CMake figures out the exact compiler and linker commands and generates the native build. It's cross-platform, which is why the same file builds on Linux, Windows, and the embedded board via the SDK's toolchain file.

**Q29. What is a library?**
A library is a bundle of pre-written, reusable code you build on instead of writing yourself — OpenCV and dlib are libraries. Your program is compiled and then **linked** against them so it can call their functions.

**Q30. Static vs dynamic linking? (briefly)**
Static linking copies the library's code into your executable at build time — the program is bigger but self-contained. Dynamic linking keeps the library as a separate shared file (`.so`) that's loaded at run time — the executable is smaller but needs the library present on the machine. My board binary is dynamically linked against OpenCV, which is why OpenCV 4.10 already being in `imx-image-full` matters.

**Q31. How does the build know whether dlib is available?**
The `CMakeLists.txt` tries to find dlib, or compiles it from a local source checkout with `add_subdirectory`. If it's found, CMake defines a compile flag `DMS_HAVE_DLIB`, and the C++ code uses `#ifdef DMS_HAVE_DLIB` to compile the dlib paths. If dlib isn't present, those paths are skipped and the program still builds, falling back to the OpenCV Haar-cascade detector.

---

## B5. Computer-vision / DMS-specific questions

**Q32. How does drowsiness detection work?**
It's based on the eyes over time. From the landmarks I compute EAR (eye openness) every frame; a sustained drop means the eyes are closing. I track how long they stay closed — a short closure is a blink, a longer one is a long blink, and closure past thresholds becomes Drowsy and then a Critical microsleep. I also compute PERCLOS, the percentage of a rolling minute with eyes closed, and count yawns via MAR. All of these feed a drowsiness score.

**Q33. What is EAR and what's the formula?**
Eye Aspect Ratio. For the six eye landmarks it's the sum of the two vertical eyelid distances divided by twice the horizontal eye width: `EAR = (‖p2−p6‖ + ‖p3−p5‖) / (2·‖p1−p4‖)`. It's high when the eye is open (~0.3) and drops toward zero as it closes, and because it's a ratio it's independent of how far the face is from the camera.

**Q34. Why calibrate EAR instead of using a fixed threshold?**
Because eyes, glasses, and camera angles differ between people. Instead of one fixed number, the system learns each driver's own open-eye baseline (the recent maximum EAR) and calls the eyes closed at 62% of that. It also smooths the signal and uses hysteresis — separate enter and leave thresholds — so a value hovering at the edge doesn't flicker.

**Q35. What is PERCLOS?**
The **PER**centage of eye **CLOS**ure — the fraction of a rolling time window (60 seconds here) during which the eyes are closed. It's one of the most validated fatigue metrics. I compute it from timestamps rather than frame counts, so a fluctuating frame rate doesn't distort it. Above 15% is building fatigue; above 30% is drowsy.

**Q36. How do you detect a yawn?**
With the Mouth Aspect Ratio (MAR) — the vertical inner-lip gaps over the stable outer-mouth width. A closed mouth is near zero and a yawn is tall. To avoid counting talking, a yawn only counts when the mouth stays open past a minimum duration (~0.55 s), using an adaptive baseline and a dip tolerance so brief landmark jitter doesn't reset the timer.

**Q37. How do you detect head pose?**
With `cv::solvePnP`. I take six 2-D landmarks (nose tip, chin, eye corners, mouth corners) and match them to a generic 3-D face model; solvePnP recovers the rotation, which I convert to yaw, pitch and roll angles. Yaw is turning left/right, pitch is up/down, roll is tilt. The angles are EMA-smoothed and feed the distraction detector.

**Q38. How does phone detection work, and why is it NOT based on head turning?**
It uses real object detection — a YOLO model finds an actual phone box in the frame. I require temporal confirmation (the phone seen across several cycles) and optionally fuse it with a nearby hand, driving a state machine from NoPhone to UsageConfirmed. It is deliberately never inferred from head pose, because turning your head to check a mirror is not phone use — many naive systems make that mistake and produce false accusations. Looking down feeds the distraction score only; a phone is reported only when a phone is actually seen.

**Q39. What is YOLO and how does OpenCV run it?**
YOLO — "You Only Look Once" — is a fast single-pass object detector that outputs a bounding box, class ID and confidence for each object. I run it through OpenCV's dnn module, so there's no separate ONNX Runtime dependency. The pipeline is: letterbox the frame to a square, make a blob (scale and swap R/B), run `forward()`, decode the outputs, and apply Non-Maximum Suppression to remove duplicate overlapping boxes.

**Q40. What is the COCO dataset?**
COCO is the standard public dataset — about 80 everyday object categories — that off-the-shelf YOLO models are trained on. Each category has a fixed ID: cell phone is class 67, person is class 0. A model can only detect the categories it was trained on.

**Q41. Why are smoking and seat belt UNKNOWN?**
Because COCO has no cigarette class and no seat-belt class, so a standard YOLO simply can't detect them. Rather than fake it from hand or head movement, both detectors honestly return UNKNOWN. The full multi-cue logic is already written and waiting; the moment someone supplies a custom-trained model and sets its class ID in the config, the features come alive with no code changes.

**Q42. How do you handle an angled camera mount?**
In a real car the camera is off to the side, so a driver looking at the road doesn't appear straight-on. At startup there's a 3-second calibration where the system averages the driver's neutral "looking at the road" pose and stores it as the zero reference, then subtracts it from every later reading. So "forward" becomes relative to that mount, and only real departures count as looking away. I can also preset the offsets or redo calibration by pressing `c`.

**Q43. How did you reduce lag on the board?**
Two causes. First, frames buffering faster than they're processed — I fixed that with a CameraGrabber that runs the camera on a background thread and always keeps only the newest frame, dropping stale ones, so latency can't grow. Second, the CPU-heavy YOLO model — I added a `--fast` mode that drops YOLO, lowers the resolution, and runs face detection less often. I also always detect the face on a half-size frame every few frames while running the cheap landmark predictor every frame.

**Q44. What is AIS-184 and is your system certified?**
AIS-184 is India's Driver Drowsiness and Attention Warning System standard, from ARAI under the Ministry of Road Transport, applying to buses and heavy goods vehicles. My system is designed *with reference to* AIS-184 as a research and demonstration prototype. It is **not** type-approved or certified, and I'm careful to say so. The thresholds are sensible engineering defaults, not clinically calibrated legal limits. We couldn't even retrieve the official standard PDF automatically, so every clause-level claim in the docs is marked as needing official verification.

**Q45. What is the MonitoringQuality gate and why does it matter?**
It's a reliability check — brightness, face size, detector confidence, landmark presence and head rotation. If the system can't actually see the driver's eyes well (dark room, tiny/far face, extreme head turn), it reports "monitoring quality low" and the RiskEngine suppresses the drowsiness evidence. That way a bad view produces "I can't tell" instead of a false "driver drowsy." A safety system that cries wolf gets ignored; being honest about uncertainty is what makes it trustworthy.

**Q46. What does the RiskEngine do and why hysteresis?**
It fuses the drowsiness, distraction and phone scores with configurable weights (0.40/0.30/0.20/0.10), applies per-signal floors so a single strong signal isn't averaged away by an otherwise calm blend, smooths the result, and commits one of seven driver states. The hysteresis state machine means a state change must persist before it takes effect — faster to escalate, slower to recover — so the displayed state never flickers frame to frame, like a thermostat with a deadband.

**Q47. How does gaze estimation work, and why is it "approximate"?**
dlib's 68 landmarks have no iris point, so gaze is coarse by design. For each open eye I crop the eye region, blur it, find the darkest blob (the pupil) via its centroid, and measure its offset from the eye center. It's smoothed and only *contributes* to the distraction score — it never triggers a hard alarm on its own, and it's skipped when the eyes are closed.

---

## B6. HR / behavioral questions

**Q48. Why did you choose this project?**
It sits right at the intersection of things I wanted to learn — real-time computer vision, C++, and embedded Linux — and it solves a genuinely important problem: driver fatigue and distraction cause a huge share of road accidents. Building something that could actually run in a vehicle and reference a real safety standard made it feel meaningful, not just an academic exercise.

**Q49. What challenges did you face and how did you solve them?**
The biggest was moving from "works on my laptop" to "works on the board." I hit cross-compilation issues, an OpenCV sysroot packaging quirk that I fixed with a symlink, no display on the board (solved with headless mode and a browser stream), and CPU lag (solved with a frame-dropping grabber and a `--fast` mode). Each one taught me to diagnose the real cause instead of guessing — for example, checking `file` on the binary to confirm it was really aarch64.

**Q50. What would you improve?**
Two things. First, move the neural networks onto the i.MX 93's Ethos-U NPU by quantizing to TensorFlow Lite and running through the vela compiler — that would make YOLO and landmarks much faster than CPU inference. Second, add proper models for the currently-UNKNOWN features — a custom cigarette and seat-belt detector — since the logic is already wired and just needs the model and class IDs. I'd also want to validate the thresholds against real labelled data rather than engineering defaults.

**Q51. What did you learn during the internship?**
The whole embedded pipeline end to end — Linux and the command line, cross-compilation, Yocto concepts, CMake, and real-time OpenCV/dlib. But the biggest lesson was engineering honesty: building a system that reports "unknown" or "monitoring quality low" instead of faking confidence. In a safety product, a false alarm that gets ignored is worse than admitting uncertainty.

**Q52. How was teamwork / how did you handle feedback?**
I kept the code modular — one responsibility per file — so pieces could be worked on and reviewed independently, and I documented every threshold and design decision so others could follow the reasoning. When something was ambiguous, like exactly how strict the phone confirmation should be, I made it configurable rather than hard-coding my own guess, so it could be tuned collaboratively.

**Q53. Where do you see this being used?**
Automotive driver safety — it's aligned with AIS-184, which targets buses and heavy commercial vehicles. Realistic uses are fleet and commercial-vehicle monitoring (trucks, buses, cabs) to reduce fatigue-related accidents, and as a building block in broader ADAS (Advanced Driver Assistance Systems). The local, privacy-preserving design — no cloud, metadata-only logs, no biometric database — makes it suitable for real vehicles.

**Q54. How is privacy handled? (common follow-up)**
Everything is processed on the device and frames are discarded after analysis — no cloud, no internet. There's no face-recognition or biometric identity database. Logs contain only metadata (timestamps and events), never images, and a privacy mode disables all disk logging entirely.

---

## Final tips

> **Before you walk in:**
> - **Revise the four high-value topics cold:** EAR (formula + why it drops), PERCLOS (rolling % over 60 s), YOLO (box + class + confidence, COCO 67 = phone), and cross-compilation (host vs target, why an x86-64 binary won't run on aarch64).
> - **Be honest about limitations** — "designed *with reference to* AIS-184, not certified"; phone is evidence-based only; smoking/seat-belt are honestly UNKNOWN; the monitoring-quality gate says "I can't tell" instead of false-alarming. Interviewers respect this.
> - **Know your own code** — be able to name the modules, the data flow, and why each design choice exists (frame-dropping grabber, hysteresis, neutral-pose calibration, `--fast`/`--headless`/`--stream`).
> - **When unsure, explain your reasoning** rather than bluffing a fact — that mirrors the honesty the system itself is built on.
