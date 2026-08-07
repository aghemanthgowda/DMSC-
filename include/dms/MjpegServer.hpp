#pragma once

#include <opencv2/core.hpp>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace dms {

// Tiny built-in MJPEG-over-HTTP server so the live annotated dashboard can be
// watched in a browser on another machine (e.g. a laptop on the same network as
// a headless board). No external dependencies: it uses plain POSIX sockets and
// OpenCV's JPEG encoder. On a host without BSD sockets (Windows) it compiles to
// a harmless no-op.
//
// Usage:
//   MjpegServer stream;
//   stream.start(8080);
//   ... each frame ...
//   stream.publish(canvas);   // encodes to JPEG and pushes to all viewers
//
// Point a browser at  http://<board-ip>:8080/  to watch the live stream.
class MjpegServer {
public:
    ~MjpegServer();

    // Start listening on the given TCP port (all interfaces). Returns false if
    // the socket cannot be created/bound. jpegQuality is 1..100.
    bool start(int port, int jpegQuality = 80);

    // Encode the frame to JPEG once and broadcast it to every connected client.
    // Dead clients are dropped. Cheap when no clients are connected.
    void publish(const cv::Mat& frameBGR);

    void stop();

    bool running() const { return running_.load(); }
    int port() const { return port_; }
    int clientCount();

private:
    void acceptLoop();

    std::atomic<bool> running_{false};
    int listenFd_ = -1;
    int port_ = 0;
    int jpegQuality_ = 80;
    std::thread acceptThread_;

    std::mutex clientsMtx_;
    std::vector<int> clients_;
};

} // namespace dms
