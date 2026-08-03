#include "dms/Dashboard.hpp"

#include <opencv2/imgproc.hpp>

#include <cstdio>
#include <string>

namespace dms {

namespace {

const cv::Scalar kGreen(90, 200, 90);
const cv::Scalar kAmber(40, 190, 250);
const cv::Scalar kOrange(30, 130, 250);
const cv::Scalar kRed(60, 60, 240);
const cv::Scalar kInk(235, 235, 235);
const cv::Scalar kMuted(155, 150, 145);
const cv::Scalar kPanelBg(30, 27, 24);
const cv::Scalar kCard(46, 42, 38);

cv::Scalar levelColor(int level) {
    switch (level) {
        case 3: return kRed;
        case 2: return kOrange;
        case 1: return kAmber;
        default: return kGreen;
    }
}

cv::Scalar stateColor(DriverState s) {
    switch (s) {
        case DriverState::Safe: return kGreen;
        case DriverState::AttentionRequired: return kAmber;
        case DriverState::Drowsy:
        case DriverState::Distracted:
        case DriverState::PhoneUsage:
        case DriverState::HighRisk: return kOrange;
        case DriverState::Critical: return kRed;
    }
    return kGreen;
}

void text(cv::Mat& img, const std::string& s, cv::Point o, double sc, const cv::Scalar& c,
          int th = 1) {
    cv::putText(img, s, o, cv::FONT_HERSHEY_SIMPLEX, sc, c, th, cv::LINE_AA);
}

std::string fmt(const char* f, double v) {
    char b[64];
    std::snprintf(b, sizeof(b), f, v);
    return b;
}

// Landmark groups for pretty overlays.
void polyline(cv::Mat& img, const std::vector<cv::Point2f>& lm, int a, int b, bool closed,
              const cv::Scalar& c) {
    std::vector<cv::Point> pts;
    for (int i = a; i <= b; ++i) pts.emplace_back(lm[i]);
    if (pts.size() > 1) cv::polylines(img, pts, closed, c, 1, cv::LINE_AA);
}

} // namespace

cv::Mat Dashboard::render(const cv::Mat& frameBGR, const Frame& f) {
    const auto& obs = *f.obs;
    const auto& drowsy = *f.drowsy;
    const auto& distract = *f.distract;
    const auto& risk = *f.risk;
    const auto& alert = *f.alert;

    cv::Mat view = frameBGR.clone();
    const cv::Scalar accent = stateColor(risk.state);

    // ---- Camera overlays ----
    if (obs.faceDetected) {
        cv::rectangle(view, obs.face, accent, 2);
        text(view, fmt("conf %.2f", obs.confidence), {obs.face.x, obs.face.y - 8}, 0.45, accent);

        if (obs.hasLandmarks && obs.landmarks.size() == 68) {
            const auto& lm = obs.landmarks;
            polyline(view, lm, 36, 41, true, kGreen);   // left eye
            polyline(view, lm, 42, 47, true, kGreen);   // right eye
            polyline(view, lm, 48, 59, true, kAmber);   // outer mouth
            polyline(view, lm, 27, 30, false, kMuted);  // nose bridge
            polyline(view, lm, 0, 16, false, kMuted);   // jaw
            for (const auto& p : lm) cv::circle(view, p, 1, cv::Scalar(0, 255, 255), -1, cv::LINE_AA);

            // Head-pose direction arrow from the nose tip.
            if (obs.hasHeadPose) {
                const cv::Point2f nose = lm[30];
                const cv::Point2f tip(nose.x - static_cast<float>(distract.yaw) * 2.0f,
                                      nose.y - static_cast<float>(distract.pitch) * 2.0f);
                cv::arrowedLine(view, nose, tip, cv::Scalar(255, 200, 0), 2, cv::LINE_AA, 0, 0.3);
            }
            // Gaze arrow.
            if (f.gaze && f.gaze->valid) {
                const cv::Point2f eyeC = 0.5f * (lm[39] + lm[42]);
                const cv::Point2f g(eyeC.x + static_cast<float>(f.gaze->dx) * 30.0f,
                                    eyeC.y + static_cast<float>(f.gaze->dy) * 30.0f);
                cv::arrowedLine(view, eyeC, g, cv::Scalar(255, 120, 255), 2, cv::LINE_AA, 0, 0.35);
            }
        }
    }
    if (f.phone && f.phone->phonePresent) cv::rectangle(view, f.phone->box, kRed, 2);
    if (obs.faceCount > 1) text(view, "MULTIPLE FACES", {12, 24}, 0.6, kAmber, 2);
    if (alert.level == 3) cv::rectangle(view, {0, 0}, {view.cols - 1, view.rows - 1}, kRed, 8);

    // Alert banner across the top of the camera.
    if (!alert.message.empty()) {
        cv::Mat strip = view(cv::Rect(0, 0, view.cols, 30));
        strip *= 0.35;
        text(view, alert.message, {12, 21}, 0.6, levelColor(alert.level), 2);
    }
    // Calibration overlay.
    if (f.calibrating) {
        cv::Mat ov = view.clone();
        cv::rectangle(ov, {0, 0}, {view.cols, view.rows}, cv::Scalar(0, 0, 0), -1);
        cv::addWeighted(ov, 0.45, view, 0.55, 0, view);
        text(view, "CALIBRATION", {view.cols / 2 - 110, view.rows / 2 - 20}, 0.9, kInk, 2);
        text(view, "Please look straight at the camera", {view.cols / 2 - 165, view.rows / 2 + 12},
             0.6, kInk);
        text(view, fmt("%.1fs", f.calibRemaining), {view.cols / 2 - 20, view.rows / 2 + 46}, 0.7,
             kAmber, 2);
    }

    // ---- Side panel ----
    cv::Mat panel(view.rows, kPanelWidth, view.type(), kPanelBg);
    int y = 30;
    text(panel, "DRIVER MONITORING", {16, y}, 0.6, kInk, 2);
    y += 18;
    text(panel, f.backend + "  |  " + fmt("%.0f fps", f.fps), {16, y},
         0.4, f.landmarksActive ? kMuted : kRed);
    y += 20;

    // Loud warning when landmarks are unavailable (why yawn/EAR say "n/a").
    if (!f.landmarksActive) {
        cv::rectangle(panel, {12, y}, {kPanelWidth - 12, y + 40}, kRed, -1);
        text(panel, "HAAR MODE - landmarks OFF", {20, y + 17}, 0.46, cv::Scalar(20, 20, 20), 1);
        text(panel, "EAR / yawn / pose disabled", {20, y + 33}, 0.42, cv::Scalar(20, 20, 20), 1);
        y += 50;
    }

    // State banner.
    cv::rectangle(panel, {12, y}, {kPanelWidth - 12, y + 50}, accent, -1);
    text(panel, toString(risk.state), {22, y + 24}, 0.62, cv::Scalar(20, 20, 20), 2);
    text(panel, fmt("RISK %.0f/100", risk.score), {22, y + 44}, 0.5, cv::Scalar(20, 20, 20), 1);
    y += 62;

    // Monitoring-quality gate: unreliable => we are NOT asserting drowsiness.
    if (!f.monitoringReliable && obs.faceDetected) {
        cv::rectangle(panel, {12, y}, {kPanelWidth - 12, y + 34}, kAmber, -1);
        text(panel, "MONITORING QUALITY LOW", {20, y + 15}, 0.44, cv::Scalar(20, 20, 20), 1);
        text(panel, f.monitoringReason.empty() ? "reduced reliability" : f.monitoringReason,
             {20, y + 30}, 0.4, cv::Scalar(20, 20, 20), 1);
        y += 42;
    }
    if (f.privacyMode) {
        text(panel, "PRIVACY MODE - no logging/images", {16, y}, 0.4, kGreen);
        y += 18;
    }

    // Risk gauge.
    {
        const int x = 16, w = kPanelWidth - 32, h = 12;
        cv::rectangle(panel, {x, y}, {x + w, y + h}, kCard, -1);
        const int fillw = static_cast<int>(w * std::clamp(risk.score / 100.0, 0.0, 1.0));
        cv::rectangle(panel, {x, y}, {x + fillw, y + h}, accent, -1);
        // threshold ticks
        for (double thr : {40.0, 65.0, 85.0}) {
            const int tx = x + static_cast<int>(w * thr / 100.0);
            cv::line(panel, {tx, y - 2}, {tx, y + h + 2}, kMuted, 1);
        }
        y += h + 16;
    }

    // Two-column metrics.
    auto metric = [&](int col, const std::string& k, const std::string& v, const cv::Scalar& vc) {
        const int x = 16 + col * (kPanelWidth - 32) / 2;
        text(panel, k, {x, y}, 0.4, kMuted);
        text(panel, v, {x, y + 16}, 0.46, vc, 1);
    };
    auto rowGap = [&]() { y += 38; };

    const bool lm = drowsy.ear >= 0.0;
    metric(0, "EAR (L/R/avg)",
           lm ? fmt("%.2f", drowsy.earLeft) + "/" + fmt("%.2f", drowsy.earRight) + "/" +
                    fmt("%.2f", drowsy.ear)
              : "n/a",
           drowsy.eyesClosed ? kRed : kInk);
    metric(1, "Eyes", drowsy.eyesClosed ? "CLOSED" : "open", drowsy.eyesClosed ? kRed : kGreen);
    rowGap();

    metric(0, "PERCLOS", fmt("%.0f%%", drowsy.perclos * 100),
           drowsy.perclos >= 0.30 ? kRed : (drowsy.perclos >= 0.15 ? kAmber : kInk));
    metric(1, "MAR / open",
           lm ? fmt("%.2f", drowsy.mar) + "/" + fmt("%.2f", drowsy.marOpenThreshold) : "n/a",
           drowsy.yawning ? kAmber : kInk);
    rowGap();

    metric(0, "Blinks", std::to_string(drowsy.blinkCount) + " (" + fmt("%.0f", drowsy.blinkRate) +
                            "/m)", kInk);
    metric(1, "Yawns", std::to_string(drowsy.yawnCount), drowsy.yawning ? kAmber : kInk);
    rowGap();

    metric(0, "Drowsiness", toString(drowsy.level),
           drowsy.level >= DrowsyLevel::Drowsy ? kRed
                                               : (drowsy.level == DrowsyLevel::Possible ? kAmber
                                                                                        : kGreen));
    metric(1, "Attention", toString(distract.level),
           distract.level >= DistractionLevel::Distracted
               ? kRed
               : (distract.level == DistractionLevel::Brief ? kAmber : kGreen));
    rowGap();

    metric(0, "Head Y/P/R",
           obs.hasHeadPose ? fmt("%.0f", distract.yaw) + "/" + fmt("%.0f", distract.pitch) + "/" +
                                 fmt("%.0f", distract.roll)
                           : "n/a",
           distract.headDir != "forward" ? kAmber : kInk);
    metric(1, "Gaze", f.gaze && f.gaze->valid ? f.gaze->direction : "n/a",
           (f.gaze && f.gaze->valid && f.gaze->direction != "forward") ? kAmber : kInk);
    rowGap();

    metric(0, "Head dir", distract.headDir, distract.headDir != "forward" ? kAmber : kGreen);
    metric(1, "Phone",
           (f.phone && f.phone->available) ? (f.phone->phonePresent ? "DETECTED" : "none")
                                           : "off",
           (f.phone && f.phone->phonePresent) ? kRed : kInk);
    rowGap();

    // Developer diagnostics.
    if (f.developer) {
        text(panel, fmt("infer %.1f ms", f.inferenceMs) + fmt("   drowsyC %.0f", risk.drowsyContribution),
             {16, y}, 0.4, kMuted);
        y += 16;
        text(panel,
             fmt("distC %.0f", risk.distractionContribution) +
                 fmt("  phoneC %.0f", risk.phoneContribution) +
                 fmt("  yawnC %.0f", risk.yawnContribution),
             {16, y}, 0.4, kMuted);
        y += 18;
    }

    // Event timeline.
    cv::line(panel, {12, y}, {kPanelWidth - 12, y}, kCard, 1);
    y += 16;
    text(panel, "EVENT TIMELINE", {16, y}, 0.42, kMuted);
    y += 18;
    if (f.events) {
        const auto& ev = *f.events;
        const int maxRows = std::max(0, (panel.rows - y - 12) / 16);
        int shown = 0;
        for (auto it = ev.rbegin(); it != ev.rend() && shown < maxRows; ++it, ++shown) {
            text(panel, it->time + "  " + it->text, {16, y}, 0.38, levelColor(it->level));
            y += 16;
        }
        if (ev.empty()) text(panel, "(no events yet)", {16, y}, 0.38, kMuted);
    }

    // Footer hint.
    text(panel, "q quit  d dev  c calibrate", {16, panel.rows - 10}, 0.38, kMuted);

    cv::Mat canvas;
    cv::hconcat(view, panel, canvas);
    return canvas;
}

} // namespace dms
