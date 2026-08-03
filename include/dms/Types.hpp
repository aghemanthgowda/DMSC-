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
};

// ---------------------------------------------------------------------------
// Object (phone) detection result (spec section 6). Populated by the ONNX
// ObjectDetector when built; otherwise a harmless "nothing detected".
// ---------------------------------------------------------------------------
struct PhoneResult {
    bool available = false;                // detector actually running
    bool phonePresent = false;             // confirmed over several frames
    double confidence = 0.0;
    cv::Rect box;
};

} // namespace dms
