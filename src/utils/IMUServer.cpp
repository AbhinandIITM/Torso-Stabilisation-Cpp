#include "scripts/include/utils/IMUServer.hpp"
#include "scripts/include/utils/IMUTracker.hpp" // Include the full header for IMUTracker here

#include <iostream>
#include <functional> // For std::bind and std::placeholders

IMUServer::IMUServer(int port)
    : port(port) {
    imu_tracker = std::make_shared<IMUTracker>();
}

void IMUServer::run() {
    try {
        server_.init_asio();
        server_.set_message_handler(
            std::bind(&IMUServer::on_message, this,
                      std::placeholders::_1, std::placeholders::_2));
        server_.listen(port);
        server_.start_accept();
        std::cout << "[IMU] WebSocket server running on ws://0.0.0.0:" << port << std::endl;
        server_.run();
    } catch (const std::exception& e) {
        std::cerr << "[IMU] Server run error: " << e.what() << std::endl;
    }
    std::cout << "[IMU] WebSocket server stopped." << std::endl;
}

void IMUServer::stop() {
    if (!server_.is_listening()) {
        return;
    }
    std::cout << "[IMU] Stopping WebSocket server..." << std::endl;
    try {
        server_.stop_listening();
        server_.stop();
    } catch (const std::exception& e) {
        std::cerr << "[IMU] Server stop error: " << e.what() << std::endl;
    }
}

json IMUServer::get_latest_transform() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_tf;
}


void IMUServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {
        json data = json::parse(msg->get_payload());

        // --- Safer Parsing ---
        if (!data.contains("timestamp") || !data.contains("imu") ||
            !data["imu"].contains("Samsung Linear Acceleration Sensor") ||
            !data["imu"]["Samsung Linear Acceleration Sensor"].contains("values") ||
            !data["imu"].contains("ICM42632M Gyroscope") ||
            !data["imu"]["ICM42632M Gyroscope"].contains("values"))
        {
            std::cerr << "[IMU] Received malformed JSON message." << std::endl;
            return; // Skip this message
        }
        // --- End Safer Parsing ---

        double timestamp = data["timestamp"];
        auto accel_vals = data["imu"]["Samsung Linear Acceleration Sensor"]["values"];
        // ... rest of your code ...
    } catch (const std::exception& e) {
        // ...
    }
}
