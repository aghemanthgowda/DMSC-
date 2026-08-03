# AIS-184 Compliance Report (Prototype)

> **This software prototype has NOT been type-approved or officially certified
> for AIS-184 compliance.** It is described as an *"AIS-184 requirement-oriented
> DDAWS research/demo prototype — designed with reference to AIS-184
> requirements."* No claim of legal or regulatory compliance is made.

## 1. Scope

A laptop-webcam **Driver Drowsiness and Attention Warning System (DDAWS)**
demonstration prototype implementing the software-addressable functional concepts
of AIS-184 (drowsiness detection via eye/face analysis, attention monitoring,
temporal decision-making, and driver warning). Hardware, vehicle-integration,
environmental and EMC aspects are out of scope for this prototype.

## 2. Applicable AIS-184 version

The latest official ARAI-published AIS-184 revision. **The full normative PDF
could not be retrieved in this environment (HTTP 403 from the ARAI host)**, so the
exact revision/amendment level, clause numbers and quantitative limits are
**unverified here** and must be confirmed against the official document.

## 3. Reference document

- ARAI-hosted AIS-184 PDF: `https://hmr.araiindia.com/` (Automotive Industry
  Standards, AIS-184).
- Automotive Industry Standards Committee (AISC) / MoRTH.
- See `AIS184_TRACEABILITY.md` for the requirement-by-requirement mapping and
  the `VERIFY-OFFICIAL` markers.

## 4. Vehicle-category scope

AIS-184 is reported to apply to categories **M2, M3, N2, N3** (buses/coaches and
medium/heavy goods vehicles), with implementation timelines commonly cited as new
models **1 Apr 2026** and existing models **1 Oct 2026**. **VERIFY-OFFICIAL** —
this prototype targets none of these vehicle categories; it is a bench/webcam
demonstration only.

## 5. Implemented requirements (software)

Face/driver detection & tracking; 68-point landmarks; L/R/average EAR with
per-driver calibration; blink vs long-blink vs prolonged-closure; time-based
PERCLOS; MAR yawn detection; head-pose (yaw/pitch/roll) and classification;
attention estimation; temporal drowsiness classification (ALERT/POSSIBLE/DROWSY/
CRITICAL); central DDAW decision with hysteresis, confirmation and cooldown;
visual + audible warning with escalation/recovery; monitoring-quality gate;
configurable thresholds; optional non-identifying calibration; privacy mode.

## 6. Partially implemented

Multiple-face primary-driver selection (position-based); approximate gaze;
warning-HMI signal specification (exact loudness/colour/duration
`VERIFY-OFFICIAL`); fault/system-status signalling; extended phone detection
(optional, not an AIS-184 DDAW requirement).

## 7. Unimplemented / needs official mapping

Explicit **KSS-referenced** drowsiness scoring and its official trigger level
(requires the official definition + lab validation).

## 8. Requirements needing testing beyond this prototype

- **Lab validation:** warning-latency limits, KSS correlation, environmental
  robustness, EMI/EMC.
- **Vehicle test:** mounting/field-of-view, on-road drowsiness/attention
  scenarios, integration with vehicle HMI and telltales.

## 9. Environmental limitations

Tuned for indoor/office lighting. No validation for sunlight, night/IR, vibration,
temperature extremes, or occlusion beyond a basic low-light/……quality gate.

## 10. Hardware limitations

Standard laptop RGB webcam only. No automotive-grade camera, no IR illuminator
(so eye/eyelid tracking in darkness is unsupported), no dedicated compute.

## 11. Webcam-prototype limitations

Monocular head pose and gaze are **approximate**. No true 3D, no eye-gaze
calibration rig. Frame rate depends on the host CPU.

## 12. Privacy / data handling

Local processing only; frames discarded after analysis; **no face-recognition /
biometric identity database**; event logs contain metadata only. `privacy_mode`
disables all disk logging. (Aligns with spec §18.)

## 13. DDAW functionality

Present and temporally filtered (see §5). Decision is evidence-based, not
single-frame; unreliable monitoring is reported as such rather than as
drowsiness.

## 14. Warning functionality

Visual banner + rate-limited audible alarm with escalation and recovery; core
DDAW warning is separate from extended (phone) alerts. Exact warning-signal
conformity is **VERIFY-OFFICIAL**.

## 15. Known limitations

See §9–11 and every `VERIFY-OFFICIAL` / `REQUIRES_*` row in
`AIS184_TRACEABILITY.md`. **No official certification is claimed or implied.**
