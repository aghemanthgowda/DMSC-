#include "dms/Dashboard.hpp"

#include <opencv2/imgproc.hpp>

#include <cstdio>
#include <string>

namespace dms {

namespace {

const cv::Scalar kGreen(80, 200, 90);
const cv::Scalar kAmber(40, 180, 250);
const cv::Scalar kRed(60, 60, 240);
const cv::Scalar kInk(235, 235, 235);
const cv::Scalar kMuted(150, 150, 150);
const cv::Scalar kPanelBg(32, 30, 28);

cv::Scalar levelColor(int level) {
    switch (level) {
        case 2: return kRed;
        case 1: return kAmber;
        default: return kGreen;
    }
}

void putLabel(cv::Mat& img, const std::string& text, cv::Point org, double scale,
              const cv::Scalar& color, int thickness = 1) {
    cv::putText(img, text, org, cv::FONT_HERSHEY_SIMPLEX, scale, color, thickness,
                cv::LINE_AA);
}

// A labelled horizontal meter filled to `value` (0..1) and coloured by warn/alarm.
void drawMeter(cv::Mat& panel, int x, int y, int w, const std::string& label,
               double value, double warn, double alarm) {
    value = std::max(0.0, std::min(1.0, value));
    putLabel(panel, label, {x, y - 6}, 0.42, kMuted);
    const int h = 12;
    cv::rectangle(panel, {x, y}, {x + w, y + h}, cv::Scalar(70, 66, 62), cv::FILLED);
    cv::Scalar c = kGreen;
    if (value >= alarm) c = kRed;
    else if (value >= warn) c = kAmber;
    cv::rectangle(panel, {x, y}, {x + static_cast<int>(w * value), y + h}, c, cv::FILLED);
}

std::string fmt(const char* f, double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, v);
    return buf;
}

} // namespace

cv::Mat Dashboard::render(const cv::Mat& frameBGR,
                          const FaceObservation& obs,
                          const DrowsinessDetector::Result& drowsy,
                          const DistractionDetector::Result& distract,
                          const AlertManager::State& alert,
                          double fps,
                          bool usingLandmarks) {
    cv::Mat view = frameBGR.clone();
    const cv::Scalar faceColor = levelColor(alert.level);

    // --- Overlays on the video ---
    if (obs.faceDetected) {
        cv::rectangle(view, obs.face, faceColor, 2);
        for (const auto& e : obs.eyes) cv::rectangle(view, e, kGreen, 1);
        if (obs.hasLandmarks) {
            for (const auto& p : obs.landmarks) {
                cv::circle(view, p, 1, cv::Scalar(0, 255, 255), cv::FILLED, cv::LINE_AA);
            }
        }
    }

    // A pulsing border when the system is alarming.
    if (alert.level == 2) {
        cv::rectangle(view, {0, 0}, {view.cols - 1, view.rows - 1}, kRed, 8);
    }

    // --- Build the side panel ---
    cv::Mat panel(view.rows, kPanelWidth, view.type(), kPanelBg);
    int y = 34;
    putLabel(panel, "DRIVER MONITORING", {16, y}, 0.62, kInk, 2);
    y += 22;
    putLabel(panel, usingLandmarks ? "mode: 68-pt landmarks" : "mode: haar cascade",
             {16, y}, 0.4, kMuted);
    y += 24;

    // Status banner.
    cv::rectangle(panel, {12, y}, {kPanelWidth - 12, y + 44}, faceColor, cv::FILLED);
    putLabel(panel, alert.headline, {22, y + 29}, 0.66, cv::Scalar(20, 20, 20), 2);
    y += 68;

    auto row = [&](const std::string& k, const std::string& v, const cv::Scalar& vc) {
        putLabel(panel, k, {16, y}, 0.46, kMuted);
        putLabel(panel, v, {150, y}, 0.46, vc, 1);
        y += 24;
    };

    row("Face", obs.faceDetected ? "detected" : "not found",
        obs.faceDetected ? kGreen : kRed);
    row("Eyes", drowsy.eyesClosed ? "CLOSED" : "open",
        drowsy.eyesClosed ? kRed : kGreen);
    if (obs.ear >= 0.0) row("EAR", fmt("%.2f", obs.ear), kInk);
    row("Closure", fmt("%.1fs", drowsy.closureSeconds),
        drowsy.closureSeconds > 1.0 ? kRed : kInk);
    y += 6;

    drawMeter(panel, 16, y, kPanelWidth - 60, "PERCLOS  " + fmt("%.0f%%", drowsy.perclos * 100),
              drowsy.perclos, 0.15, 0.30);
    y += 34;

    row("Blinks", std::to_string(drowsy.blinkCount) + "  (" +
                      fmt("%.0f", drowsy.blinkRate) + "/min)", kInk);
    if (obs.mar >= 0.0) {
        row("Yawns", std::to_string(drowsy.yawnCount), drowsy.yawning ? kAmber : kInk);
    }

    std::string attn = distract.direction;
    if (obs.hasHeadPose) {
        attn += "  " + fmt("y%.0f", obs.yaw) + fmt(" p%.0f", obs.pitch);
    }
    row("Attention", attn, distract.lookingAway ? kAmber : kGreen);
    y += 8;

    // Active reasons.
    putLabel(panel, "alerts:", {16, y}, 0.44, kMuted);
    y += 20;
    if (alert.reasons.empty()) {
        putLabel(panel, "- none", {24, y}, 0.44, kGreen);
        y += 20;
    } else {
        for (const auto& r : alert.reasons) {
            putLabel(panel, "- " + r, {24, y}, 0.44, levelColor(alert.level));
            y += 20;
        }
    }

    // Footer.
    putLabel(panel, fmt("%.0f fps", fps), {16, panel.rows - 34}, 0.44, kMuted);
    putLabel(panel, "q / ESC : quit", {16, panel.rows - 14}, 0.44, kMuted);

    cv::Mat canvas;
    cv::hconcat(view, panel, canvas);
    return canvas;
}

} // namespace dms
