#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

namespace dms {

// ---------------------------------------------------------------------------
// Shared enums
// ---------------------------------------------------------------------------

// Fine-grained drowsiness classification (spec section 2F).
enum class DrowsyLevel { Alert, Possible, Drowsy, Critical };
const char* toString(DrowsyLevel l);

// Attention / distraction classification (spec section 8).
enum class DistractionLevel { Attentive, Brief, Distracted, Highly };
const char* toString(DistractionLevel l);

// Phone-usage state machine (evidence-based, never from head pose).
enum class PhoneState { NoPhone, Possible, Detected, UsageConfirmed };
const char* toString(PhoneState s);

// Smoking state machine. Requires an actual cigarette-capable model; defaults
// to Unknown so we never fake a detection.
enum class SmokingState { NoSmoking, Possible, Confirmed, Unknown };
const char* toString(SmokingState s);

// Seat-belt state. Unknown when visibility/model is insufficient — never claim
// "not worn" just because the belt isn't visible.
enum class SeatBeltState { Unknown, NotDetected, Detected, Fastened };
const char* toString(SeatBeltState s);

// Overall fused driver state produced by the RiskEngine (spec section 9).
enum class DriverState {
    Safe,
    AttentionRequired,
    Drowsy,
    Distracted,
    PhoneUsage,
    HighRisk,
    Critical
};
const char* toString(DriverState s);

// ---------------------------------------------------------------------------
// Per-frame perception output from the FaceTracker.
// ---------------------------------------------------------------------------
struct FaceObservation {
    bool faceDetected = false;
    int faceCount = 0;                    // number of faces seen this frame
    double confidence = 0.0;              // detector confidence for the driver face
    cv::Rect face;                        // driver face box (largest / tracked)
    std::vector<cv::Rect> eyes;           // eye boxes from the Haar fallback path

    bool hasLandmarks = false;
    std::vector<cv::Point2f> landmarks;   // 68 points when hasLandmarks == true

    bool eyesClosed = false;              // best per-frame estimate (fallback path)
    double earLeft = -1.0;                // per-eye and averaged EAR (>=0 with landmarks)
    double earRight = -1.0;
    double ear = -1.0;
    double mar = -1.0;                    // mouth-aspect-ratio (>=0 with landmarks)

    bool hasHeadPose = false;
    double yaw = 0.0;                     // degrees, + = turned to driver's right
    double pitch = 0.0;                   // degrees, + = looking up
    double roll = 0.0;                    // degrees
};

// ---------------------------------------------------------------------------
// Approximate visual-attention (gaze) estimate. Deliberately labelled
// "approximate" — dlib's 68 points have no iris, so this is a coarse cue.
// ---------------------------------------------------------------------------
struct GazeResult {
    bool valid = false;
    std::string direction = "unknown";    // forward|left|right|up|down|unknown
    double dx = 0.0;                       // normalized pupil offset (-1..1)
    double dy = 0.0;
    bool hasPupils = false;                // pupil/iris centers located
    cv::Point2f leftPupil{0, 0};           // full-frame coords
    cv::Point2f rightPupil{0, 0};
};

// Approximate hand-activity result (Extended DMS feature). Skin-based, coarse.
struct HandResult {
    bool available = false;                // analyzer running
    int handsVisible = 0;                  // approximate count near the driver
    bool handNearFace = false;             // temporally confirmed
    bool possiblePhoneUse = false;         // hand-near-face + phone-like head posture
    double confidence = 0.0;
    std::vector<cv::Rect> boxes;
};

// ---------------------------------------------------------------------------
// Object (phone) detection result (spec section 6). Populated by the ONNX
// ObjectDetector when built; otherwise a harmless "nothing detected".
// ---------------------------------------------------------------------------
struct PhoneResult {
    bool available = false;                // detector actually running (real model)
    bool phonePresent = false;             // state >= Detected (temporal-confirmed)
    PhoneState state = PhoneState::NoPhone;
    bool heldByHand = false;               // phone box near a hand (fusion)
    double confidence = 0.0;
    cv::Rect box;
};

// Smoking detection result (Extended DMS). Honest default = Unknown.
struct SmokingResult {
    bool available = false;
    SmokingState state = SmokingState::Unknown;
    double confidence = 0.0;
    cv::Rect box;
};

// Seat-belt result (Extended DMS). Honest default = Unknown; torso ROI provided
// for a future segmentation/detection model.
struct SeatBeltResult {
    bool available = false;
    SeatBeltState state = SeatBeltState::Unknown;
    double confidence = 0.0;
    cv::Rect torsoRoi;
};

} // namespace dms
