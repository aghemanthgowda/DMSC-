#pragma once

#include <string>

namespace dms {

// Central configuration. Every threshold is documented and can be overridden
// from config/config.json (see ConfigManager). Defaults suit a laptop webcam at
// arm's length in normal lighting.
struct Config {
    // --- Eye closure / drowsiness (EAR) ---
    double earThreshold = 0.21;          // fallback EAR threshold before calibration
    double earSmoothingFrames = 5;       // moving-average window over raw EAR
    double earBaselineWindowSeconds = 5; // rolling window whose max = "eyes open" baseline
    double earCloseRatio = 0.62;         // eyes closed when EAR < ratio * open-baseline
    double eyeClosedAlarmSeconds = 1.20; // sustained closure => microsleep (Critical)
    double eyeClosedDrowsySeconds = 0.60; // sustained closure => Drowsy

    // --- PERCLOS (rolling % eyes-closed) ---
    double perclosWindowSeconds = 60.0;  // rolling window for PERCLOS
    double perclosWarn = 0.15;           // >=15% => building fatigue
    double perclosAlarm = 0.30;          // >=30% => drowsy

    // --- Blinks ---
    double blinkMinSeconds = 0.06;       // shorter closures ignored as noise
    double blinkMaxSeconds = 0.40;       // longer closures count as closure, not a blink
    double longBlinkSeconds = 0.50;      // "long blink" (fatigue cue)

    // --- Yawn (MAR) ---
    // Yawn detection is adaptive: it learns the driver's closed-mouth MAR
    // baseline and flags "mouth open" when MAR rises marOpenDelta above it (or
    // above the absolute marThreshold floor), with hysteresis + a dip tolerance
    // so landmark jitter during a yawn does not reset the timer.
    double marThreshold = 0.30;          // absolute open floor
    double marOpenDelta = 0.22;          // open when MAR > closed-baseline + this
    double marBaselineWindowSeconds = 8; // window for the closed-mouth baseline
    double marSmoothingFrames = 3;       // moving-average window over raw MAR
    double yawnMinSeconds = 0.60;        // mouth open this long => a yawn
    double yawnDipToleranceSeconds = 0.30; // brief MAR dips shorter than this don't reset

    // --- Head pose / distraction ---
    double headAwayYawDegrees = 25.0;    // |yaw| beyond this => looking left/right
    double headDownPitchDegrees = 18.0;  // pitch below -this => looking down
    double headAwayDurationSeconds = 2.0; // sustained head-away => distraction
    double headPoseSmoothing = 0.35;     // EMA factor for yaw/pitch/roll (0..1)
    double offCenterFraction = 0.22;     // fallback: face-centre offset fraction of width

    // --- Gaze (approximate) ---
    double gazeOffThreshold = 0.28;      // normalized pupil offset => looking away

    // --- Object detection (YOLO / ONNX) ---
    // Class IDs into the model's label set. COCO defaults: person=0, cell phone=67.
    // cigarette / seat belt are NOT in COCO — leave at -1 until a custom model is
    // provided (the detector then reports UNKNOWN rather than faking it).
    int classIdPhone = 67;
    int classIdPerson = 0;
    int classIdCigarette = -1;
    int classIdSeatbelt = -1;
    double yoloConfidence = 0.45;        // detection score threshold
    double nmsThreshold = 0.45;          // non-max-suppression IoU
    double objectTrackTimeoutSeconds = 0.6;  // drop a track after this without a hit
    double handPhoneDistanceFraction = 0.9;  // hand-phone dist / face-width => "held"
    double handMouthDistanceFraction = 0.6;  // hand-mouth dist / face-width (smoking)

    // --- Phone temporal confirmation ---
    double phoneConfidenceThreshold = 0.50;
    int phoneConfirmFrames = 3;          // detections needed to confirm PHONE
    int phoneDetectEveryNFrames = 4;     // run YOLO ~1/N frames for performance

    // --- Hand activity (Extended DMS, skin-based, approximate) ---
    bool enableHandDetection = true;     // detect hand-near-face for phone-use inference
    double handMinAreaFraction = 0.10;   // min skin blob area as fraction of face area
    double handConfirmSeconds = 0.4;     // temporal confirmation window

    // --- Presence / face-loss debounce ---
    double faceLostGraceSeconds = 0.7;   // ignore brief dropouts before "absent"
    double noFaceAlarmSeconds = 1.5;     // no face this long => driver absent alarm

    // --- Monitoring quality / low-confidence (AIS-184-oriented reliability gate) ---
    // When monitoring is unreliable the system reports "MONITORING QUALITY LOW"
    // instead of asserting drowsiness, so it never false-alarms when it cannot
    // actually see the driver's eyes.
    double qualityMinFaceWidthFraction = 0.12;  // face narrower than this => too far/small
    double qualityLowLightMean = 45.0;          // mean luma below this => too dark
    double qualityMinConfidence = 0.30;         // detector confidence floor
    double qualityMaxYawDegrees = 45.0;         // beyond this landmarks get unreliable
    double qualityMaxPitchDegrees = 35.0;

    // --- Risk fusion (0..100). Weights are relative contributions. ---
    double wDrowsiness = 0.40;
    double wDistraction = 0.30;
    double wPhone = 0.20;
    double wYawn = 0.10;
    double riskWarnThreshold = 40.0;     // >= => ATTENTION_REQUIRED
    double riskHighThreshold = 65.0;     // >= => HIGH_RISK
    double riskCriticalThreshold = 85.0; // >= => CRITICAL

    // --- Temporal state machine ---
    double stateEnterSeconds = 0.6;      // evidence must persist this long to escalate
    double stateExitSeconds = 1.5;       // recovery must persist this long to de-escalate
    double alertCooldownSeconds = 4.0;   // min gap between repeat audio alerts

    // --- Calibration ---
    double calibrationSeconds = 3.0;     // "look straight" baseline capture

    // --- Performance / detection ---
    int faceDetectEveryNFrames = 3;      // run HOG face detection 1/N frames (reuse box between)
    double detectionScale = 0.5;         // downscale factor for HOG detection

    // --- System / IO ---
    int cameraIndex = 0;
    int captureWidth = 640;
    int captureHeight = 480;
    int cameraRotation = 0;              // 0/90/180/270 for a rotated mounting
    bool mirror = true;                  // selfie-mirrored view

    // --- Head-pose neutral (for an angled/off-centre camera mount) ---
    // Captured during calibration: the driver's normal "looking at the road"
    // pose becomes the zero reference, so a side-mounted camera doesn't read the
    // neutral pose as "looking away". Can also be preset here.
    bool headPoseCalibrate = true;       // learn neutral pose during calibration
    double headYawOffset = 0.0;          // preset offset (used if not calibrating)
    double headPitchOffset = 0.0;
    double headRollOffset = 0.0;
    bool beep = true;                    // audible alarm
    bool developerMode = false;          // extra on-screen diagnostics
    bool logEvents = true;               // append events to logs/events.csv
    bool privacyMode = false;            // no logging/image storage; metadata only
    std::string cascadeDir;              // Haar cascade dir (auto-detected if empty)
    std::string facemarkModel;           // dlib .dat or OpenCV .yaml (auto-detected)
    std::string phoneModel;              // YOLO .onnx path (optional)
    std::string configPath = "config/config.json";
};

} // namespace dms
