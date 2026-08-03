#include "dms/Dashboard.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <string>

namespace dms {

namespace {

// ---- Palette (BGR), modern dark automotive theme ----
const cv::Scalar kBg(30, 26, 22);
const cv::Scalar kCard(48, 42, 38);
const cv::Scalar kCard2(60, 53, 48);
const cv::Scalar kInk(240, 240, 240);
const cv::Scalar kMuted(150, 144, 138);
const cv::Scalar kAccent(210, 180, 70);   // teal/cyan brand accent
const cv::Scalar kGreen(110, 210, 120);
const cv::Scalar kAmber(60, 195, 250);
const cv::Scalar kOrange(45, 135, 250);
const cv::Scalar kRed(75, 70, 240);

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
cv::Scalar levelColor(int l) {
    return l >= 3 ? kRed : l == 2 ? kOrange : l == 1 ? kAmber : kGreen;
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

// Filled rounded rectangle.
void roundRect(cv::Mat& img, cv::Rect r, int rad, const cv::Scalar& col) {
    rad = std::min(rad, std::min(r.width, r.height) / 2);
    cv::rectangle(img, {r.x + rad, r.y}, {r.x + r.width - rad, r.y + r.height}, col, -1);
    cv::rectangle(img, {r.x, r.y + rad}, {r.x + r.width, r.y + r.height - rad}, col, -1);
    cv::circle(img, {r.x + rad, r.y + rad}, rad, col, -1, cv::LINE_AA);
    cv::circle(img, {r.x + r.width - rad, r.y + rad}, rad, col, -1, cv::LINE_AA);
    cv::circle(img, {r.x + rad, r.y + r.height - rad}, rad, col, -1, cv::LINE_AA);
    cv::circle(img, {r.x + r.width - rad, r.y + r.height - rad}, rad, col, -1, cv::LINE_AA);
}

// 270-degree gauge arc, opening at the bottom.
void arcGauge(cv::Mat& img, cv::Point c, int rad, double v, const cv::Scalar& col) {
    v = std::clamp(v, 0.0, 1.0);
    cv::ellipse(img, c, {rad, rad}, 0, 135, 135 + 270, kCard2, 7, cv::LINE_AA);
    cv::ellipse(img, c, {rad, rad}, 0, 135, 135 + static_cast<int>(270 * v), col, 7, cv::LINE_AA);
}

// HUD corner brackets for the face box.
void brackets(cv::Mat& img, cv::Rect r, const cv::Scalar& col, int len = 26, int th = 2) {
    const cv::Point tl(r.x, r.y), tr(r.x + r.width, r.y);
    const cv::Point bl(r.x, r.y + r.height), br(r.x + r.width, r.y + r.height);
    auto L = [&](cv::Point p, int dx, int dy) {
        cv::line(img, p, {p.x + dx, p.y}, col, th, cv::LINE_AA);
        cv::line(img, p, {p.x, p.y + dy}, col, th, cv::LINE_AA);
    };
    L(tl, len, len);
    L(tr, -len, len);
    L(bl, len, -len);
    L(br, -len, -len);
}

void polyline(cv::Mat& img, const std::vector<cv::Point2f>& lm, int a, int b, bool closed,
              const cv::Scalar& c) {
    std::vector<cv::Point> p;
    for (int i = a; i <= b; ++i) p.emplace_back(lm[i]);
    if (p.size() > 1) cv::polylines(img, p, closed, c, 1, cv::LINE_AA);
}

std::string clockStr() {
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char b[16];
    std::strftime(b, sizeof(b), "%H:%M:%S", &tm);
    return b;
}

} // namespace

cv::Mat Dashboard::render(const cv::Mat& frameBGR, const Frame& f) {
    const auto& obs = *f.obs;
    const auto& drowsy = *f.drowsy;
    const auto& distract = *f.distract;
    const auto& risk = *f.risk;
    const auto& alert = *f.alert;
    const cv::Scalar accent = stateColor(risk.state);

    // ---------- Camera view + overlays (drawn at native res, scaled after) ----------
    cv::Mat cam = frameBGR.clone();
    if (obs.faceDetected) {
        brackets(cam, obs.face, accent);
        text(cam, fmt("%.0f%%", obs.confidence * 100) + " face", {obs.face.x, obs.face.y - 8},
             0.45, accent);
        if (obs.hasLandmarks && obs.landmarks.size() == 68) {
            const auto& lm = obs.landmarks;
            polyline(cam, lm, 36, 41, true, kGreen);
            polyline(cam, lm, 42, 47, true, kGreen);
            polyline(cam, lm, 48, 59, true, kAmber);
            polyline(cam, lm, 27, 30, false, kMuted);
            polyline(cam, lm, 0, 16, false, kMuted);
            for (const auto& p : lm) cv::circle(cam, p, 1, cv::Scalar(0, 255, 255), -1, cv::LINE_AA);
            if (obs.hasHeadPose) {
                const cv::Point n(cvRound(lm[30].x), cvRound(lm[30].y));
                cv::arrowedLine(cam, n,
                                {n.x - cvRound(distract.yaw * 2.0), n.y - cvRound(distract.pitch * 2.0)},
                                kAccent, 2, cv::LINE_AA, 0, 0.3);
            }
            if (f.gaze && f.gaze->valid) {
                const cv::Point2f ef = 0.5f * (lm[39] + lm[42]);
                const cv::Point e(cvRound(ef.x), cvRound(ef.y));
                cv::arrowedLine(cam, e,
                                {e.x + cvRound(f.gaze->dx * 30.0), e.y + cvRound(f.gaze->dy * 30.0)},
                                cv::Scalar(255, 120, 255), 2, cv::LINE_AA, 0, 0.35);
            }
            if (f.gaze && f.gaze->hasPupils) {
                cv::circle(cam, f.gaze->leftPupil, 3, cv::Scalar(255, 120, 255), -1, cv::LINE_AA);
                cv::circle(cam, f.gaze->rightPupil, 3, cv::Scalar(255, 120, 255), -1, cv::LINE_AA);
            }
        }
    }
    if (f.hand && f.hand->handNearFace)
        for (const auto& b : f.hand->boxes) cv::rectangle(cam, b, cv::Scalar(0, 200, 255), 2);
    if (f.phone && f.phone->phonePresent) cv::rectangle(cam, f.phone->box, kRed, 2);
    if (obs.faceCount > 1) text(cam, "MULTIPLE FACES", {12, 26}, 0.6, kAmber, 2);
    if (alert.level == 3) cv::rectangle(cam, {0, 0}, {cam.cols - 1, cam.rows - 1}, kRed, 8);
    if (!alert.message.empty()) {
        cv::Mat strip = cam(cv::Rect(0, 0, cam.cols, 32));
        strip *= 0.35;
        text(cam, alert.message, {14, 22}, 0.62, levelColor(alert.level), 2);
    }
    if (f.calibrating) {
        cv::Mat ov = cam.clone();
        ov.setTo(cv::Scalar(0, 0, 0));
        cv::addWeighted(ov, 0.5, cam, 0.5, 0, cam);
        text(cam, "CALIBRATION", {cam.cols / 2 - 120, cam.rows / 2 - 16}, 0.95, kInk, 2);
        text(cam, "Please look straight at the camera",
             {cam.cols / 2 - 175, cam.rows / 2 + 16}, 0.6, kInk);
        text(cam, fmt("%.1fs", f.calibRemaining), {cam.cols / 2 - 22, cam.rows / 2 + 50}, 0.75,
             kAccent, 2);
    }

    // Scale the annotated camera up to a crisp display height.
    const int dispH = 600;
    const double sc = static_cast<double>(dispH) / cam.rows;
    cv::Mat view;
    cv::resize(cam, view, cv::Size(static_cast<int>(cam.cols * sc), dispH), 0, 0, cv::INTER_LINEAR);

    // ---------- Side panel ----------
    const int PW = kPanelWidth;
    cv::Mat panel(dispH, PW, view.type(), kBg);

    // Header
    cv::rectangle(panel, {0, 0}, {PW, 52}, kCard, -1);
    cv::rectangle(panel, {0, 0}, {6, 52}, accent, -1);
    text(panel, "DRIVER MONITORING", {18, 25}, 0.6, kInk, 2);
    text(panel, "DDAWS prototype", {18, 43}, 0.4, kMuted);
    text(panel, clockStr(), {PW - 92, 25}, 0.5, kAccent, 1);
    text(panel, fmt("%.0f fps", f.fps), {PW - 92, 43}, 0.42,
         f.landmarksActive ? kMuted : kRed);

    int y = 68;

    // Hero status card with arc gauge
    const int heroH = 96;
    roundRect(panel, {12, y, PW - 24, heroH}, 12, kCard);
    cv::rectangle(panel, {12, y}, {19, y + heroH}, accent, -1);
    text(panel, toString(risk.state), {28, y + 34}, 0.72, accent, 2);
    text(panel, f.landmarksActive ? "68-pt landmarks" : "HAAR mode (no landmarks)",
         {28, y + 58}, 0.42, f.landmarksActive ? kMuted : kRed);
    text(panel, "risk", {28, y + 82}, 0.42, kMuted);
    text(panel, fmt("%.0f", risk.score), {60, y + 84}, 0.5, kInk, 1);
    const cv::Point gc(PW - 60, y + heroH / 2);
    arcGauge(panel, gc, 34, risk.score / 100.0, accent);
    {
        const cv::Size ts = cv::getTextSize(fmt("%.0f", risk.score), cv::FONT_HERSHEY_SIMPLEX,
                                            0.7, 2, nullptr);
        text(panel, fmt("%.0f", risk.score), {gc.x - ts.width / 2, gc.y + 8}, 0.7, kInk, 2);
    }
    y += heroH + 10;

    // Condition pills
    auto pill = [&](const std::string& s, const cv::Scalar& col) {
        const int w = static_cast<int>(cv::getTextSize(s, cv::FONT_HERSHEY_SIMPLEX, 0.42, 1,
                                                        nullptr).width) + 22;
        roundRect(panel, {12, y, w, 24}, 12, col);
        text(panel, s, {24, y + 16}, 0.42, cv::Scalar(20, 20, 20), 1);
        y += 30;
    };
    if (!f.landmarksActive) pill("HAAR MODE - landmarks OFF", kRed);
    if (!f.monitoringReliable && obs.faceDetected)
        pill("MONITORING LOW: " + (f.monitoringReason.empty() ? "quality" : f.monitoringReason),
             kAmber);
    if (f.privacyMode) pill("PRIVACY MODE", kGreen);

    // Metrics grid of cards
    text(panel, "VITALS", {14, y + 12}, 0.44, kMuted, 1);
    y += 22;
    const int pad = 12, cardH = 46;
    const int colW = (PW - 3 * pad) / 2;
    int idx = 0;
    const int gridTop = y;
    auto card = [&](const std::string& label, const std::string& value, const cv::Scalar& vc) {
        const int col = idx % 2, row = idx / 2;
        const int x = pad + col * (colW + pad);
        const int cy = gridTop + row * (cardH + 8);
        roundRect(panel, {x, cy, colW, cardH}, 8, kCard);
        text(panel, label, {x + 12, cy + 17}, 0.4, kMuted);
        text(panel, value, {x + 12, cy + 37}, 0.5, vc, 1);
        ++idx;
    };

    const bool lmk = drowsy.ear >= 0.0;
    card("EAR (avg)", lmk ? fmt("%.2f", drowsy.ear) : "n/a", drowsy.eyesClosed ? kRed : kInk);
    card("EYES", drowsy.eyesClosed ? "CLOSED" : "open", drowsy.eyesClosed ? kRed : kGreen);
    card("PERCLOS", fmt("%.0f%%", drowsy.perclos * 100),
         drowsy.perclos >= 0.30 ? kRed : drowsy.perclos >= 0.15 ? kAmber : kInk);
    card("MAR / open", lmk ? fmt("%.2f", drowsy.mar) + "/" + fmt("%.2f", drowsy.marOpenThreshold)
                           : "n/a",
         drowsy.yawning ? kAmber : kInk);
    card("BLINKS", std::to_string(drowsy.blinkCount) + " (" + fmt("%.0f", drowsy.blinkRate) + "/m)",
         kInk);
    card("YAWNS", std::to_string(drowsy.yawnCount), drowsy.yawning ? kAmber : kInk);
    card("DROWSINESS", toString(drowsy.level),
         drowsy.level >= DrowsyLevel::Drowsy ? kRed
             : drowsy.level == DrowsyLevel::Possible ? kAmber : kGreen);
    card("ATTENTION", toString(distract.level),
         distract.level >= DistractionLevel::Distracted ? kRed
             : distract.level == DistractionLevel::Brief ? kAmber : kGreen);
    card("HEAD Y/P/R",
         obs.hasHeadPose ? fmt("%.0f", distract.yaw) + "/" + fmt("%.0f", distract.pitch) + "/" +
                               fmt("%.0f", distract.roll)
                         : "n/a",
         distract.headDir != "forward" ? kAmber : kInk);
    card("GAZE", f.gaze && f.gaze->valid ? f.gaze->direction : "n/a",
         f.gaze && f.gaze->valid && f.gaze->direction != "forward" ? kAmber : kInk);
    card("PHONE", (f.phone && f.phone->phonePresent)
                      ? (f.phoneInferred ? "USE (infer)" : "DETECTED")
                      : (f.phone && f.phone->available ? "none" : "off"),
         (f.phone && f.phone->phonePresent) ? kRed : kInk);
    card("HAND / EYE",
         std::string(f.hand && f.hand->handNearFace ? "near-face" : "clear") + " / " +
             (f.gaze && f.gaze->hasPupils ? "iris" : "-"),
         (f.hand && f.hand->handNearFace) ? kAmber : kGreen);
    y = gridTop + ((idx + 1) / 2) * (cardH + 8) + 6;

    if (f.developer) {
        text(panel,
             fmt("infer %.1fms", f.inferenceMs) + fmt("  D%.0f", risk.drowsyContribution) +
                 fmt(" K%.0f", risk.distractionContribution) + fmt(" P%.0f", risk.phoneContribution),
             {14, y + 12}, 0.38, kMuted);
        y += 20;
    }

    // Event log
    cv::line(panel, {12, y}, {PW - 12, y}, kCard2, 1);
    y += 18;
    text(panel, "EVENT LOG", {14, y}, 0.44, kMuted, 1);
    y += 18;
    if (f.events) {
        const int maxRows = std::max(0, (panel.rows - y - 26) / 18);
        int shown = 0;
        for (auto it = f.events->rbegin(); it != f.events->rend() && shown < maxRows; ++it, ++shown) {
            cv::circle(panel, {18, y - 4}, 3, levelColor(it->level), -1, cv::LINE_AA);
            text(panel, it->time, {28, y}, 0.38, kMuted);
            text(panel, it->text, {92, y}, 0.38, kInk);
            y += 18;
        }
        if (f.events->empty()) text(panel, "no events yet", {28, y}, 0.38, kMuted);
    }

    // Footer key hints
    text(panel, "q quit   d dev   c calibrate", {14, panel.rows - 12}, 0.4, kMuted);

    cv::Mat canvas;
    cv::hconcat(view, panel, canvas);
    return canvas;
}

} // namespace dms
