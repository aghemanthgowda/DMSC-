# AIS-184 Requirement Traceability

**Status of this document:** requirement-oriented mapping for a research/demo
prototype. It is **not** a certification artifact.

## Reference & sourcing note (read first)

- **Standard:** AIS-184, *"Driver Drowsiness and Attention Warning System
  (DDAWS)"*, published by the Automotive Research Association of India (ARAI),
  Pune, as secretariat of the Automotive Industry Standards Committee (AISC),
  under MoRTH direction.
- **Official document:** hosted by ARAI at
  `https://hmr.araiindia.com/` (AIS-184 PDF). **The full normative PDF could not
  be retrieved in this build environment (HTTP 403).** Therefore every
  clause-level detail below (exact thresholds, KSS trigger level, timing,
  warning-signal specifications, test procedures) is marked
  **`VERIFY-OFFICIAL`** and must be checked against the official AIS-184 PDF
  before any compliance claim.
- **High-level facts used (from authoritative reporting / ARAI-hosted context):**
  DDAWS uses cabin camera(s) and assesses driver drowsiness with reference to the
  **Karolinska Sleepiness Scale (KSS)**; applies to vehicle categories
  **M2, M3, N2, N3**; implementation timeline commonly cited as new models
  **1 Apr 2026** / existing models **1 Oct 2026**; the standard covers technical
  requirements, validation/verification, and EMI/EMC testing. Category/date
  specifics are still marked `VERIFY-OFFICIAL`.

**Do not treat the requirement wording below as verbatim regulatory text.** It is
a good-faith functional interpretation. Requirement IDs (RQ-*) are our own
labels, not official clause numbers.

## Status legend

`IMPLEMENTED` · `PARTIALLY_IMPLEMENTED` · `NOT_IMPLEMENTED` ·
`REQUIRES_VEHICLE_TEST` · `REQUIRES_LAB_VALIDATION` ·
`NOT_APPLICABLE_TO_WEBCAM_PROTOTYPE` · `VERIFY-OFFICIAL`

## Traceability table

| ID | Requirement (functional interpretation) | Implementation | C++ Module | Test | Status | Limitations |
|---|---|---|---|---|---|---|
| RQ-01 | Detect the driver's face and track it continuously | dlib HOG detection + box tracking, half-scale/N-frame for perf | FaceTracker | T01,T12,T13 | IMPLEMENTED | webcam-only; not vehicle-integrated |
| RQ-02 | Distinguish temporary tracking loss from "driver not detected" | face-loss grace + sustained no-face timer | FaceTracker + main | T12,T13 | IMPLEMENTED | thresholds are prototype values |
| RQ-03 | Handle multiple faces; monitor the primary driver | largest/tracked face = driver | FaceTracker | T14 | PARTIALLY_IMPLEMENTED | position-based selection only |
| RQ-04 | Facial landmark tracking (eyes, mouth, nose, contour) | 68-pt dlib shape predictor | FaceTracker | T01,T16,T17 | IMPLEMENTED | iris not tracked (no iris model) |
| RQ-05 | Eye-closure / EAR-based eye-state detection | L/R/avg EAR, smoothing, per-driver calibration | DrowsinessDetector | T01–T05,T16,T17 | IMPLEMENTED | thresholds prototype; `VERIFY-OFFICIAL` |
| RQ-06 | Blink vs long-blink vs prolonged closure (temporal) | duration-gated edge-triggered logic | DrowsinessDetector | T02,T03,T04,T05 | IMPLEMENTED | |
| RQ-07 | PERCLOS over a configurable, time-based window | timestamped rolling window | DrowsinessDetector | T04,T05 | IMPLEMENTED | window/threshold prototype |
| RQ-08 | Yawn detection (MAR, duration-gated) as a drowsiness input | MAR + min-duration | DrowsinessDetector | T06,T07 | IMPLEMENTED | one contributing cue, not sole |
| RQ-09 | Drowsiness classification with temporal persistence | ALERT/POSSIBLE/DROWSY/CRITICAL + score | DrowsinessDetector | T02–T07 | IMPLEMENTED | mapping to KSS levels `VERIFY-OFFICIAL` |
| RQ-10 | KSS-referenced drowsiness assessment | not mapped to KSS scale | DrowsinessEstimator | — | NOT_IMPLEMENTED / VERIFY-OFFICIAL | needs official KSS trigger definition + lab validation |
| RQ-11 | Head-pose estimation (yaw/pitch/roll) + classification | solvePnP + EMA + direction | FaceTracker + DistractionDetector | T08–T11 | IMPLEMENTED | monocular; approximate |
| RQ-12 | Attention / distraction estimation with temporal logic | head (+ approx gaze) fusion, 4 levels | DistractionDetector | T08–T11 | PARTIALLY_IMPLEMENTED | gaze is approximate |
| RQ-13 | Do not warn when monitoring is unreliable | light/size/pose/confidence gate → "monitoring quality low" | MonitoringQuality + RiskEngine | T15,T16,T17,T23 | IMPLEMENTED | heuristic quality metric |
| RQ-14 | Central DDAW decision with confirmation/hysteresis/cooldown | fusion + state machine | RiskEngine | T02–T11 | IMPLEMENTED | thresholds prototype |
| RQ-15 | Driver warning: clear, timely, visual + audible; no per-frame spam | banner + rate-limited alarm + escalation/recovery | AlertManager + Dashboard | T02–T11 | PARTIALLY_IMPLEMENTED | HMI signal spec (loudness/'duration/colour) `VERIFY-OFFICIAL` |
| RQ-16 | Warning timing / latency requirement | measurable, but target unknown | AlertManager | T-latency | REQUIRES_LAB_VALIDATION / VERIFY-OFFICIAL | official latency limits needed |
| RQ-17 | Configurable thresholds, documented | config/config.json + Config.hpp | ConfigManager | — | IMPLEMENTED | |
| RQ-18 | Optional driver calibration (no identity) | "look straight" baseline capture | main + DrowsinessDetector | T01 | IMPLEMENTED | |
| RQ-19 | Privacy / data minimization; no identity DB; local processing | privacy mode; metadata-only logs; no recognition | EventLogger + main | — | IMPLEMENTED | |
| RQ-20 | Environmental robustness (lighting, vibration, temperature) | partial (light gate) | MonitoringQuality | T15 | REQUIRES_LAB_VALIDATION | webcam prototype cannot test vibration/temp |
| RQ-21 | EMI / EMC compliance | — | — | — | REQUIRES_LAB_VALIDATION | hardware/vehicle EMC lab only |
| RQ-22 | Vehicle-level integration, field-of-view, mounting, on-road validation | — | — | — | REQUIRES_VEHICLE_TEST | not possible on a laptop webcam |
| RQ-23 | Fault detection / system-status signalling | camera/model/inference error handling + status | main + Dashboard | T21,T22 | PARTIALLY_IMPLEMENTED | basic status only |
| RQ-24 | Extended distraction (phone) — NOT an AIS-184 DDAW requirement. Must be evidence-based, never head-pose-based | YOLO/ONNX object detection + temporal state machine + hand fusion; **head pose never implies phone** | ObjectDetector, PhoneState | T01–T07 (false-pos) | PARTIALLY_IMPLEMENTED | extended; needs an ONNX object model (COCO has phone). See docs/MODELS.md |
| RQ-25 | Smoking detection (Extended) | cigarette-near-mouth + hand-near-mouth + temporal; **UNKNOWN without a cigarette model** | SmokingDetector | T14,T15 | NOT_IMPLEMENTED (honest UNKNOWN) | COCO has no cigarette class; needs a custom model |
| RQ-26 | Seat-belt monitoring (Extended) | torso ROI + belt model; **UNKNOWN when not determinable** | SeatBeltDetector | T21,T22,T23 | NOT_IMPLEMENTED (honest UNKNOWN) | COCO has no seat-belt class; needs a custom model; never reports "not worn" on non-visibility |
| RQ-27 | Turning head / looking away / body movement MUST NOT trigger phone or smoking | posture removed from phone logic; distraction-only | main, RiskEngine | T01–T11 | IMPLEMENTED | verified: turned head → phonePresent=0 |

## Summary counts

- IMPLEMENTED: RQ-01,02,04,05,06,07,08,09,11,13,14,17,18,19
- PARTIALLY_IMPLEMENTED: RQ-03,12,15,23,24
- NOT_IMPLEMENTED / needs official mapping: RQ-10
- REQUIRES_LAB_VALIDATION: RQ-16,20,21
- REQUIRES_VEHICLE_TEST: RQ-22

Every row touching an exact regulatory value carries **VERIFY-OFFICIAL** — resolve
these against the official AIS-184 PDF from ARAI before making any claim.
