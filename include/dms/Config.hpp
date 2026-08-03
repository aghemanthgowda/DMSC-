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
    double marThreshold = 0.50;          // MAR above this => mouth open
    double marSmoothingFrames = 3;       // moving-average window over raw MAR
    double yawnMinSeconds = 0.70;        // mouth open this long => a yawn

    // --- Head pose / distraction ---
    double headAwayYawDegrees = 25.0;    // |yaw| beyond this => looking left/right
    double headDownPitchDegrees = 18.0;  // pitch below -this => looking down
    double headAwayDurationSeconds = 2.0; // sustained head-away => distraction
    double headPoseSmoothing = 0.35;     // EMA factor for yaw/pitch/roll (0..1)
    double offCenterFraction = 0.22;     // fallback: face-centre offset fraction of width

    // --- Gaze (approximate) ---
    double gazeOffThreshold = 0.28;      // normalized pupil offset => looking away

    // --- Phone detection (ONNX) ---
    double phoneConfidenceThreshold = 0.50;
    int phoneConfirmFrames = 3;          // detections needed to confirm PHONE USE
    int phoneDetectEveryNFrames = 4;     // run YOLO ~1/N frames for performance

    // --- Presence / face-loss debounce ---
    double faceLostGraceSeconds = 0.7;   // ignore brief dropouts before "absent"
    double noFaceAlarmSeconds = 1.5;     // no face this long => driver absent alarm

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
    bool mirror = true;                  // selfie-mirrored view
    bool beep = true;                    // audible alarm
    bool developerMode = false;          // extra on-screen diagnostics
    bool logEvents = true;               // append events to logs/events.csv
    std::string cascadeDir;              // Haar cascade dir (auto-detected if empty)
    std::string facemarkModel;           // dlib .dat or OpenCV .yaml (auto-detected)
    std::string phoneModel;              // YOLO .onnx path (optional)
    std::string configPath = "config/config.json";
};

} // namespace dms
