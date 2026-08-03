#include "dms/AlertManager.hpp"
#include "dms/Config.hpp"
#include "dms/ConfigManager.hpp"
#include "dms/Dashboard.hpp"
#include "dms/DistractionDetector.hpp"
#include "dms/DrowsinessDetector.hpp"
#include "dms/EventLogger.hpp"
#include "dms/FaceTracker.hpp"
#include "dms/GazeEstimator.hpp"
#include "dms/MonitoringQuality.hpp"
#include "dms/ObjectDetector.hpp"
#include "dms/RiskEngine.hpp"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

using namespace dms;

namespace {

bool fileExists(const std::string& p) {
    struct stat st{};
    return !p.empty() && stat(p.c_str(), &st) == 0;
}

std::string dirOf(const std::string& p) {
    const auto s = p.find_last_of("/\\");
    return s == std::string::npos ? std::string(".") : p.substr(0, s);
}

std::string autoDetectCascadeDir(const std::string& argv0) {
    auto ok = [](const std::string& d) {
        return fileExists(d + "/haarcascade_frontalface_default.xml");
    };
    if (const char* env = std::getenv("OPENCV_HAAR_DIR"); env && ok(env)) return env;
    const std::string exe = dirOf(argv0);
    const std::string cands[] = {
        "data", "../data", "../../data", "../../../data",
        exe + "/data", exe + "/../data", exe + "/../../data",
        "/usr/share/opencv4/haarcascades", "/usr/local/share/opencv4/haarcascades",
        "/usr/share/opencv/haarcascades", "/opt/homebrew/share/opencv4/haarcascades",
        "C:/opencv/opencv/build/etc/haarcascades", "C:/opencv/build/etc/haarcascades",
    };
    for (const auto& c : cands) if (ok(c)) return c;
    return "";
}

void printUsage(const char* prog) {
    std::cout <<
        "Driver Monitoring System (C++ / OpenCV / dlib)\n"
        "Usage: " << prog << " [options]\n"
        "  --config <path>     config JSON (default config/config.json)\n"
        "  --camera <n>        camera index\n"
        "  --model <path>      landmark model: dlib .dat or OpenCV .yaml\n"
        "  --phone-model <p>   YOLO .onnx for phone detection (needs ONNX build)\n"
        "  --cascades <dir>    Haar cascade directory (auto-detected if omitted)\n"
        "  --dev               start in developer mode\n"
        "  --privacy           privacy mode: no logging / image storage\n"
        "  --no-mirror         do not mirror the view\n"
        "  --no-beep           disable the audible alarm\n"
        "  --no-log            do not write logs/events.csv\n"
        "  -h, --help          show this help\n"
        "Keys while running:  q/ESC quit   d developer mode   c recalibrate\n";
}

} // namespace

int main(int argc, char** argv) {
    Config cfg;

    // 1) config file (if present) overlays defaults, 2) CLI overrides both.
    std::string configPath = cfg.configPath;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--config" && i + 1 < argc) configPath = argv[i + 1];
    }
    ConfigManager::load(configPath, cfg);

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto next = [&]() { return i + 1 < argc ? std::string(argv[++i]) : std::string(); };
        if (a == "--config") next();
        else if (a == "--camera") cfg.cameraIndex = std::stoi(next());
        else if (a == "--model") cfg.facemarkModel = next();
        else if (a == "--phone-model") cfg.phoneModel = next();
        else if (a == "--cascades") cfg.cascadeDir = next();
        else if (a == "--dev") cfg.developerMode = true;
        else if (a == "--privacy") cfg.privacyMode = true;
        else if (a == "--no-mirror") cfg.mirror = false;
        else if (a == "--no-beep") cfg.beep = false;
        else if (a == "--no-log") cfg.logEvents = false;
        else if (a == "-h" || a == "--help") { printUsage(argv[0]); return 0; }
        else { std::cerr << "Unknown option: " << a << "\n"; printUsage(argv[0]); return 1; }
    }

    if (cfg.cascadeDir.empty()) cfg.cascadeDir = autoDetectCascadeDir(argv[0]);
    if (cfg.cascadeDir.empty()) {
        std::cerr << "Could not locate Haar cascades. Pass --cascades <dir>.\n";
        return 1;
    }
    if (cfg.facemarkModel.empty()) {
        if (fileExists("models/shape_predictor_68_face_landmarks.dat"))
            cfg.facemarkModel = "models/shape_predictor_68_face_landmarks.dat";
        else if (fileExists("models/lbfmodel.yaml"))
            cfg.facemarkModel = "models/lbfmodel.yaml";
    }
    // Write a default config on first run so thresholds are easy to find/tune.
    if (!fileExists(configPath)) {
#ifdef _WIN32
        _mkdir("config");
#else
        mkdir("config", 0755);
#endif
        ConfigManager::writeDefault(configPath, cfg);
        std::cout << "[info] Wrote default config to " << configPath << "\n";
    }

    FaceTracker tracker(cfg);
    if (!tracker.init()) return 1;
    std::cout << "[info] Detection backend: " << tracker.backendName() << "\n";
    if (tracker.usingLandmarks()) {
        std::cout << "[OK] Landmark mode ACTIVE - EAR, yawn (MAR), head pose and gaze ENABLED.\n";
    } else {
        std::cout << "\n"
                     "==================================================================\n"
                     "  [WARNING] Running in HAAR mode - NO landmarks.\n"
                     "  EAR, YAWN, HEAD POSE and GAZE are DISABLED in this mode.\n"
                     "  To enable them: build with dlib and download the 68-point model\n"
                     "  (see README 'Build & run'), then rerun. The dashboard will show\n"
                     "  'HAAR MODE' until landmarks are active.\n"
                     "==================================================================\n\n";
    }

    ObjectDetector phoneDet(cfg);
    phoneDet.init();

    cv::VideoCapture cap(cfg.cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Could not open camera index " << cfg.cameraIndex
                  << ". Check the webcam is connected and not in use by another app.\n";
        return 1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, cfg.captureWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.captureHeight);

    GazeEstimator gaze(cfg);
    MonitoringQuality quality(cfg);
    DrowsinessDetector drowsy(cfg);
    DistractionDetector distract(cfg);
    RiskEngine riskEngine(cfg);
    AlertManager alerts(cfg);
    EventLogger logger(cfg);
    Dashboard dashboard;

    const std::string win = "Driver Monitoring System";
    cv::namedWindow(win, cv::WINDOW_AUTOSIZE);

    const auto t0 = std::chrono::steady_clock::now();
    auto lastTick = t0;
    double fps = 0.0, inferenceMs = 0.0;
    double calibStart = 0.0;

    // Presence debounce + event-transition tracking.
    FaceObservation lastGood;
    bool haveLastGood = false;
    double lastSeen = -1e9;
    DriverState prevState = DriverState::Safe;
    int prevYawns = 0;
    bool prevPhone = false, prevPresent = true, loggedStart = false;

    std::cout << "[info] Monitoring started. Keys: q/ESC quit, d dev, c recalibrate.\n";

    cv::Mat frame;
    while (true) {
        if (!cap.read(frame) || frame.empty()) {
            std::cerr << "[warn] Empty frame from camera; stopping.\n";
            break;
        }
        if (cfg.mirror) cv::flip(frame, frame, 1);

        const auto now = std::chrono::steady_clock::now();
        const double t = std::chrono::duration<double>(now - t0).count();
        const double dt = std::chrono::duration<double>(now - lastTick).count();
        lastTick = now;
        if (dt > 1e-6) fps = 0.9 * fps + 0.1 * (1.0 / dt);

        const auto iStart = std::chrono::steady_clock::now();
        FaceObservation obs = tracker.process(frame);
        inferenceMs = 0.8 * inferenceMs +
                      0.2 * std::chrono::duration<double, std::milli>(
                                std::chrono::steady_clock::now() - iStart)
                                .count();

        // Presence with grace: reuse the last good face for brief dropouts.
        if (obs.faceDetected) { lastGood = obs; haveLastGood = true; lastSeen = t; }
        const bool withinGrace = (t - lastSeen) < cfg.faceLostGraceSeconds;
        const FaceObservation& eff =
            (obs.faceDetected || !haveLastGood || !withinGrace) ? obs : lastGood;
        const bool driverPresent = obs.faceDetected || withinGrace;

        const GazeResult g = gaze.estimate(frame, eff);
        const PhoneResult phone = phoneDet.detect(frame, t);
        const MonitoringQuality::Result mq = quality.assess(frame, eff);
        const bool monitoringReliable = eff.faceDetected ? mq.reliable : true;
        const auto dRes = drowsy.update(eff, t);
        const auto kRes = distract.update(eff, g, phone, frame.size(), t);
        const auto risk =
            riskEngine.update(dRes, kRes, phone, driverPresent, monitoringReliable, t);

        const bool calibrating = (t - calibStart) < cfg.calibrationSeconds;
        AlertManager::State alert;
        if (calibrating) {
            alert.headline = "CALIBRATING";
        } else {
            alert = alerts.update(risk, t);
        }

        // ---- Event logging on meaningful transitions ----
        if (!loggedStart && obs.faceDetected) { logger.log("Driver detected", 0); loggedStart = true; }
        if (driverPresent != prevPresent) {
            logger.log(driverPresent ? "Driver detected" : "Driver not detected",
                       driverPresent ? 0 : 3);
            prevPresent = driverPresent;
        }
        if (!calibrating && risk.state != prevState) {
            logger.log(std::string("State: ") + toString(risk.state), risk.alertLevel);
            prevState = risk.state;
        }
        if (dRes.yawnCount != prevYawns) { logger.log("Yawn detected", 1); prevYawns = dRes.yawnCount; }
        if (phone.phonePresent != prevPhone) {
            if (phone.phonePresent) logger.log("Phone detected", 2);
            prevPhone = phone.phonePresent;
        }

        // ---- Render ----
        Dashboard::Frame df;
        df.obs = &eff; df.drowsy = &dRes; df.distract = &kRes; df.gaze = &g;
        df.phone = &phone; df.risk = &risk; df.alert = &alert;
        df.events = &logger.recent();
        df.fps = fps; df.inferenceMs = inferenceMs; df.backend = tracker.backendName();
        df.landmarksActive = tracker.usingLandmarks();
        df.monitoringReliable = risk.monitoringReliable || !eff.faceDetected;
        df.monitoringReason = mq.reason;
        df.privacyMode = cfg.privacyMode;
        df.calibrating = calibrating;
        df.calibRemaining = std::max(0.0, cfg.calibrationSeconds - (t - calibStart));
        df.developer = cfg.developerMode;

        const cv::Mat canvas = dashboard.render(frame, df);
        cv::imshow(win, canvas);

        const int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) break;
        if (key == 'd') cfg.developerMode = !cfg.developerMode;
        if (key == 'c') { calibStart = t; logger.log("Recalibration started", 0); }
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "[info] Monitoring stopped.\n";
    return 0;
}
