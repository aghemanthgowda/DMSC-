#pragma once

#include <string>

namespace dms {

// Tunable thresholds for the whole system. Defaults are reasonable for a
// laptop webcam at arm's length; every value can be overridden from the CLI.
struct Config {
    // --- Eye closure / drowsiness ---
    double earThreshold = 0.21;          // EAR below this => eye considered closed
    double eyeClosedAlarmSeconds = 1.20; // sustained closure => microsleep alarm
    double perclosWindowSeconds = 60.0;  // rolling window for the PERCLOS metric
    double perclosWarn = 0.15;           // >=15% eyes-closed over window => warning
    double perclosAlarm = 0.30;          // >=30% => alarm

    // --- Blink classification ---
    double blinkMinSeconds = 0.06;       // shorter closures are ignored as noise
    double blinkMaxSeconds = 0.40;       // longer closures count as drowsiness, not blinks

    // --- Yawn (only when landmarks are available) ---
    double marThreshold = 0.60;          // MAR above this => mouth open
    double yawnMinSeconds = 1.00;        // mouth open this long => a yawn

    // --- Distraction ---
    double noFaceAlarmSeconds = 1.50;    // no face this long => alarm ("eyes off road")
    double yawDistractDegrees = 25.0;    // |yaw| beyond this => looking left/right
    double pitchDownDegrees = 18.0;      // pitch below -this => looking down (phone)
    double lookAwayAlarmSeconds = 2.00;  // sustained look-away => escalate to alarm
    double offCenterFraction = 0.22;     // fallback: face-center offset as fraction of width

    // --- System / IO ---
    int cameraIndex = 0;
    bool mirror = true;                  // show a selfie-mirrored view
    bool beep = true;                    // audible terminal bell on alarm
    std::string cascadeDir;              // Haar cascade directory (auto-detected if empty)
    std::string facemarkModel;           // path to lbfmodel.yaml (optional)
};

} // namespace dms
