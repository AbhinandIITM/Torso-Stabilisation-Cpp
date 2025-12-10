#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include "utils/IMU_tracker.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace utils {

class IMUServer {
public:
    IMUServer(int port);
    ~IMUServer();
    
    void start();
    void stop();
    
    // Get latest IMU data
    Eigen::Matrix4f getTransform() const;
    
private:
    void run();
    void handle_session(tcp::socket socket);
    void on_message(const std::string& message);
    
    int m_port;
    std::unique_ptr<IMUTracker> m_tracker;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{false};
    mutable std::mutex m_mutex;
    
    net::io_context m_ioc;
    std::unique_ptr<tcp::acceptor> m_acceptor;
};

} // namespace utils
