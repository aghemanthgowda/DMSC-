#include "dms/AlertManager.hpp"
#include "dms/Config.hpp"
#include "dms/Dashboard.hpp"
#include "dms/DistractionDetector.hpp"
#include "dms/DrowsinessDetector.hpp"
#include "dms/FaceTracker.hpp"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/stat.h>

using namespace dms;

namespace {

bool fileExists(const std::string& p) {
    struct stat st{};
    return !p.empty() && stat(p.c_str(), &st) == 0;
}

// Directory portion of a path (handles both / and \ separators).
std::string dirOf(const std::string& p) {
    const auto s = p.find_last_of("/\\");
    return s == std::string::npos ? std::string(".") : p.substr(0, s);
}

// Find the Haar cascade directory. We first look for the copy bundled in the
// repo's data/ folder (so the demo is self-contained and cross-platform), then
// fall back to common OpenCV install layouts on Linux/macOS/Windows.
std::string autoDetectCascadeDir(const std::string& argv0) {
    auto hasCascade = [](const std::string& d) {
        return fileExists(d + "/haarcascade_frontalface_default.xml");
    };
    if (const char* env = std::getenv("OPENCV_HAAR_DIR"); env && hasCascade(env)) return env;

    const std::string exeDir = dirOf(argv0);
    const std::string candidates[] = {
        // Bundled with the project, relative to the working directory...
        "data", "../data", "../../data", "../../../data",
        // ...and relative to the executable (build/Release/dms.exe -> ../../data).
        exeDir + "/data", exeDir + "/../data", exeDir + "/../../data",
        // System installs.
        "/usr/share/opencv4/haarcascades",
        "/usr/local/share/opencv4/haarcascades",
        "/usr/share/opencv/haarcascades",
        "/opt/homebrew/share/opencv4/haarcascades",
        "/usr/local/Cellar/opencv/share/opencv4/haarcascades",
        // Common Windows prebuilt-OpenCV locations.
        "C:/opencv/opencv/build/etc/haarcascades",
        "C:/opencv/build/etc/haarcascades",
    };
    for (const auto& c : candidates) {
        if (hasCascade(c)) return c;
    }
    return "";
}

void printUsage(const char* prog) {
    std::cout <<
        "Driver Monitoring System (C++/OpenCV)\n"
        "Usage: " << prog << " [options]\n"
        "  --camera <n>        camera index (default 0)\n"
        "  --model <path>      LBF facemark model (lbfmodel.yaml) for landmark mode\n"
        "  --cascades <dir>    Haar cascade directory (auto-detected if omitted)\n"
        "  --no-mirror         do not mirror the view\n"
        "  --no-beep           disable the audible alarm\n"
        "  -h, --help          show this help\n";
}

} // namespace

int main(int argc, char** argv) {
    Config cfg;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto next = [&]() { return i + 1 < argc ? std::string(argv[++i]) : std::string(); };
        if (a == "--camera") cfg.cameraIndex = std::stoi(next());
        else if (a == "--model") cfg.facemarkModel = next();
        else if (a == "--cascades") cfg.cascadeDir = next();
        else if (a == "--no-mirror") cfg.mirror = false;
        else if (a == "--no-beep") cfg.beep = false;
        else if (a == "-h" || a == "--help") { printUsage(argv[0]); return 0; }
        else { std::cerr << "Unknown option: " << a << "\n"; printUsage(argv[0]); return 1; }
    }

    if (cfg.cascadeDir.empty()) cfg.cascadeDir = autoDetectCascadeDir(argv[0]);
    if (cfg.cascadeDir.empty()) {
        std::cerr << "Could not locate Haar cascades. Pass --cascades <dir> "
                     "(the folder containing haarcascade_frontalface_default.xml).\n";
        return 1;
    }
    // A conventional default location for the optional landmark model.
    if (cfg.facemarkModel.empty() && fileExists("models/lbfmodel.yaml")) {
        cfg.facemarkModel = "models/lbfmodel.yaml";
    }

    FaceTracker tracker(cfg);
    if (!tracker.init()) return 1;
    if (!tracker.usingLandmarks()) {
        std::cout << "[info] Running in Haar fallback mode (no landmark model). "
                     "For EAR/head-pose accuracy run scripts/download_facemark_model.sh "
                     "and pass --model models/lbfmodel.yaml\n";
    }

    cv::VideoCapture cap(cfg.cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Could not open camera index " << cfg.cameraIndex
                  << ". Check that a webcam is connected and not in use.\n";
        return 1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    DrowsinessDetector drowsy(cfg);
    DistractionDetector distract(cfg);
    AlertManager alerts(cfg);
    Dashboard dashboard;

    const std::string win = "Driver Monitoring System";
    cv::namedWindow(win, cv::WINDOW_AUTOSIZE);

    const auto t0 = std::chrono::steady_clock::now();
    double fps = 0.0;
    auto lastTick = t0;

    std::cout << "[info] Monitoring started. Press q or ESC in the window to quit.\n";

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

        const FaceObservation obs = tracker.process(frame);
        const auto dRes = drowsy.update(obs, t);
        const auto kRes = distract.update(obs, frame.size(), t);
        const auto state = alerts.combine(dRes, kRes, t);

        const cv::Mat canvas =
            dashboard.render(frame, obs, dRes, kRes, state, fps, tracker.usingLandmarks());
        cv::imshow(win, canvas);

        const int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) break;
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "[info] Monitoring stopped.\n";
    return 0;
}
