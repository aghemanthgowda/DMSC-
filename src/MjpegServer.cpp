#include "dms/MjpegServer.hpp"

#include <opencv2/imgcodecs.hpp>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace dms {

#ifndef _WIN32

namespace {
// Send all bytes; return false on any error so the caller can drop the client.
bool sendAll(int fd, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        const ssize_t n = ::send(fd, data + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

const char kMultipartHeader[] =
    "HTTP/1.0 200 OK\r\n"
    "Server: dms-mjpeg\r\n"
    "Connection: close\r\n"
    "Cache-Control: no-cache, private\r\n"
    "Pragma: no-cache\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=--dmsframe\r\n"
    "\r\n";

// A minimal landing page that just embeds the stream, served for any other path.
const char kIndexPage[] =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<!doctype html><html><head><title>Driver Monitoring System - Live</title>"
    "<style>body{margin:0;background:#0b0e13;color:#cbd5e1;font-family:sans-serif;"
    "text-align:center}h3{padding:10px;margin:0}img{max-width:100%;height:auto}"
    "</style></head><body><h3>Driver Monitoring System &mdash; live stream</h3>"
    "<img src=\"/stream\"></body></html>";
} // namespace

MjpegServer::~MjpegServer() { stop(); }

bool MjpegServer::start(int port, int jpegQuality) {
    port_ = port;
    jpegQuality_ = jpegQuality;

    listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        std::cerr << "[MjpegServer] socket() failed: " << std::strerror(errno) << "\n";
        return false;
    }
    int yes = 1;
    ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // all interfaces (e.g. Ethernet)
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[MjpegServer] bind(port " << port << ") failed: "
                  << std::strerror(errno) << "\n";
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }
    if (::listen(listenFd_, 8) < 0) {
        std::cerr << "[MjpegServer] listen() failed: " << std::strerror(errno) << "\n";
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }

    running_.store(true);
    acceptThread_ = std::thread(&MjpegServer::acceptLoop, this);
    return true;
}

void MjpegServer::acceptLoop() {
    while (running_.load()) {
        sockaddr_in cli{};
        socklen_t clen = sizeof(cli);
        const int fd = ::accept(listenFd_, reinterpret_cast<sockaddr*>(&cli), &clen);
        if (fd < 0) {
            if (!running_.load()) break;
            continue;
        }

        // Read the (small) request line so we can route "/" vs the stream.
        char buf[1024];
        const ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
        std::string req;
        if (n > 0) { buf[n] = '\0'; req.assign(buf); }

        // Only "GET /stream" gets the live MJPEG feed; every other path (/, the
        // favicon, etc.) gets the tiny landing page that embeds /stream.
        if (req.find("GET /stream") == std::string::npos) {
            sendAll(fd, kIndexPage, sizeof(kIndexPage) - 1);
            ::close(fd);
            continue;
        }

        // Disable Nagle for lower latency, send the multipart header, and add
        // the client to the broadcast list.
        int one = 1;
        ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
        if (!sendAll(fd, kMultipartHeader, sizeof(kMultipartHeader) - 1)) {
            ::close(fd);
            continue;
        }
        {
            std::lock_guard<std::mutex> lk(clientsMtx_);
            clients_.push_back(fd);
        }
        std::cout << "[MjpegServer] viewer connected (" << clientCount() << " total)\n";
    }
}

void MjpegServer::publish(const cv::Mat& frameBGR) {
    if (!running_.load() || frameBGR.empty()) return;
    {
        // Fast path: nothing to do if nobody is watching.
        std::lock_guard<std::mutex> lk(clientsMtx_);
        if (clients_.empty()) return;
    }

    std::vector<uchar> jpeg;
    const std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, jpegQuality_};
    if (!cv::imencode(".jpg", frameBGR, jpeg, params)) return;

    std::string head = "--dmsframe\r\nContent-Type: image/jpeg\r\nContent-Length: " +
                       std::to_string(jpeg.size()) + "\r\n\r\n";

    std::lock_guard<std::mutex> lk(clientsMtx_);
    for (auto it = clients_.begin(); it != clients_.end();) {
        const int fd = *it;
        const bool ok =
            sendAll(fd, head.data(), head.size()) &&
            sendAll(fd, reinterpret_cast<const char*>(jpeg.data()), jpeg.size()) &&
            sendAll(fd, "\r\n", 2);
        if (!ok) {
            ::close(fd);
            it = clients_.erase(it);
            std::cout << "[MjpegServer] viewer disconnected (" << clients_.size()
                      << " left)\n";
        } else {
            ++it;
        }
    }
}

int MjpegServer::clientCount() {
    std::lock_guard<std::mutex> lk(clientsMtx_);
    return static_cast<int>(clients_.size());
}

void MjpegServer::stop() {
    if (!running_.exchange(false)) return;
    if (listenFd_ >= 0) {
        ::shutdown(listenFd_, SHUT_RDWR);
        ::close(listenFd_);
        listenFd_ = -1;
    }
    if (acceptThread_.joinable()) acceptThread_.join();
    std::lock_guard<std::mutex> lk(clientsMtx_);
    for (int fd : clients_) ::close(fd);
    clients_.clear();
}

#else  // _WIN32 — no BSD sockets here; compile to a no-op.

MjpegServer::~MjpegServer() {}
bool MjpegServer::start(int, int) {
    std::cerr << "[MjpegServer] network streaming is not supported on Windows.\n";
    return false;
}
void MjpegServer::publish(const cv::Mat&) {}
void MjpegServer::stop() {}
int MjpegServer::clientCount() { return 0; }
void MjpegServer::acceptLoop() {}

#endif

} // namespace dms
