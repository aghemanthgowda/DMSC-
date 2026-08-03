# Architecture — DDAWS Prototype

**AIS-184 requirement-oriented Driver Drowsiness & Attention Warning System
(DDAWS) research/demo prototype.** Designed *with reference to* AIS-184; **not**
type-approved or certified (see `AIS184_COMPLIANCE_REPORT.md`).

## Pipeline

```
LAPTOP WEBCAM
   │  CameraManager (main.cpp capture loop, timestamped frames)
   ▼
DRIVER / FACE DETECTION ── FaceTracker (dlib HOG; multi-face → primary driver)
   ▼
FACE TRACKING + LANDMARKS ─ FaceTracker (68-pt dlib shape predictor)
   ▼
MONITORING-QUALITY GATE ── MonitoringQuality (light / size / pose / confidence)
   ▼                        └─ if LOW → report "monitoring quality low", do NOT assert drowsiness
EYE ANALYSIS ────────────── DrowsinessDetector (L/R/avg EAR, smoothing, auto-calibration)
   ├ BLINK + PERCLOS ─────── DrowsinessDetector (time-based, timestamped)
   ▼
MOUTH ANALYSIS + YAWN ───── DrowsinessDetector (MAR, duration-gated)
   ▼
HEAD POSE ───────────────── FaceTracker (solvePnP yaw/pitch/roll) + EMA smoothing
   ▼
ATTENTION ESTIMATION ────── DistractionDetector (head + approx gaze, temporal)
   ▼
DROWSINESS ESTIMATION ───── DrowsinessDetector (ALERT/POSSIBLE/DROWSY/CRITICAL + score)
   ▼
DDAW DECISION ENGINE ────── RiskEngine (fusion + hysteresis state machine)
   ▼
WARNING MANAGER ─────────── AlertManager (levels, cooldown, escalation, recovery)
   ▼
DRIVER HMI ──────────────── Dashboard (OpenCV live view + status panel)
```

## Core DDAWS vs Extended DMS (spec §16)

| Layer | Modules | Notes |
|---|---|---|
| **CORE DDAWS** | FaceTracker, MonitoringQuality, DrowsinessDetector (EAR/blink/PERCLOS/yawn), DistractionDetector (attention), RiskEngine (DDAW decision), AlertManager (warning) | The regulatory-oriented path. Runs standalone. |
| **EXTENDED DMS** | ObjectDetector (phone, ONNX — optional), GazeEstimator (approximate), EventLogger, developer diagnostics | Clearly "Additional DMS features". Do **not** gate core DDAW. Phone detection is **not** claimed as an AIS-184 DDAW requirement. |

## Module map (requested DDAWS name → this codebase)

The prototype reuses existing, tested modules rather than renaming working code.
Mapping to the spec's module list:

| Requested | Implemented as | File |
|---|---|---|
| CameraManager | capture loop | `src/main.cpp` |
| FaceDetector / FaceTracker / FaceLandmarkDetector | `FaceTracker` | `FaceTracker.*` |
| MonitoringQuality (low-confidence) | `MonitoringQuality` | `MonitoringQuality.*` |
| EyeAnalyzer / BlinkAnalyzer / PERCLOSAnalyzer | `DrowsinessDetector` | `DrowsinessDetector.*` |
| MouthAnalyzer / YawnAnalyzer | `DrowsinessDetector` | `DrowsinessDetector.*` |
| HeadPoseEstimator | `FaceTracker::estimateHeadPose` | `FaceTracker.*` |
| AttentionEstimator | `DistractionDetector` | `DistractionDetector.*` |
| DrowsinessEstimator | `DrowsinessDetector` | `DrowsinessDetector.*` |
| DDAWDecisionEngine | `RiskEngine` | `RiskEngine.*` |
| WarningManager | `AlertManager` | `AlertManager.*` |
| ConfigManager | `ConfigManager` | `ConfigManager.*` |
| EventLogger | `EventLogger` | `EventLogger.*` |
| GazeEstimator (approx, extended) | `GazeEstimator` | `GazeEstimator.*` |
| ObjectDetector (phone, extended) | `ObjectDetector` | `ObjectDetector.*` |
| PerformanceMonitor | FPS + inference timing in `main.cpp`/`Dashboard` | inline |

> A future refactor can split these into the exact 18-file layout; the behaviour
> is already separated by class. Renaming was deliberately avoided to preserve
> working, tested code.

## Temporal design (spec §9–14)

- **PERCLOS**: timestamp-based rolling window (not frame counts), robust to
  fluctuating FPS.
- **Blink/closure state**: edge-triggered; normal blink vs long blink vs
  prolonged closure separated by duration.
- **Drowsiness / attention**: smoothed EAR/MAR/pose; per-driver EAR calibration.
- **DDAW decision**: hysteresis state machine — escalate after
  `state_enter_seconds`, recover after `state_exit_seconds`; no frame-to-frame
  flicker.
- **Warning**: audible alarm rate-limited by `alert_cooldown_seconds`, escalates
  on CRITICAL.

## Privacy (spec §18)

Frames are processed locally and discarded; no video is stored; **no biometric
identity / face-recognition database exists**. `privacy_mode` disables all disk
logging (in-memory timeline only). Event logs contain metadata only (timestamp,
state, event type) — never images.

## Threading

The current prototype runs a single real-time loop (capture → analyse → render).
Sub-frame detection skipping (HOG every N frames) keeps it real-time on a laptop
CPU. A camera/inference/UI thread split is a documented future optimization
(spec §22) and is not required for the demo.
