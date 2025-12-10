#include "utils/IMU_server.hpp"
#include <iostream>

namespace utils {

IMUServer::IMUServer(int port) 
    : m_port(port)
    , m_tracker(std::make_unique<IMUTracker>())
{
}

IMUServer::~IMUServer() {
    stop();
}

void IMUServer::start() {
    m_running = true;
    m_thread = std::make_unique<std::thread>([this]() { run(); });
}

void IMUServer::stop() {
    m_running = false;
    m_ioc.stop();
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
}

void IMUServer::run() {
    try {
        tcp::endpoint endpoint(tcp::v4(), m_port);
        m_acceptor = std::make_unique<tcp::acceptor>(m_ioc, endpoint);
        
        std::cout << "IMU Server listening on port " << m_port << std::endl;
        
        while (m_running) {
            tcp::socket socket(m_ioc);
            m_acceptor->accept(socket);
            
            std::thread([this, s = std::move(socket)]() mutable {
                handle_session(std::move(s));
            }).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

void IMUServer::handle_session(tcp::socket socket) {
    try {
        websocket::stream<tcp::socket> ws(std::move(socket));
        ws.accept();
        
        while (m_running) {
            beast::flat_buffer buffer;
            ws.read(buffer);
            
            std::string message = beast::buffers_to_string(buffer.data());
            on_message(message);
        }
    } catch (const beast::system_error& e) {
        if (e.code() != websocket::error::closed) {
            std::cerr << "WebSocket error: " << e.what() << std::endl;
        }
    }
}

void IMUServer::on_message(const std::string& message) {
    try {
        auto json_data = nlohmann::json::parse(message);
        
        // Extract IMU data - cast to double for IMUTracker
        Eigen::Vector3d accel(  // ✅ Change to Vector3d
            json_data["accel"][0].get<double>(),  // ✅ get<double>()
            json_data["accel"][1].get<double>(),
            json_data["accel"][2].get<double>()
        );
        
        Eigen::Vector3d gyro(  // ✅ Change to Vector3d
            json_data["gyro"][0].get<double>(),
            json_data["gyro"][1].get<double>(),
            json_data["gyro"][2].get<double>()
        );
        
        double dt = json_data["dt"].get<double>();  // ✅ double
        
        // Update tracker
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tracker->update(accel, gyro, dt);  // ✅ Now types match
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
    }
}

Eigen::Matrix4f IMUServer::getTransform() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tracker->get_transform().cast<float>();  // ✅ Cast result to float
}


} // namespace utils
