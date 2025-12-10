#include "utils/IMU_tracker.hpp"

IMUTracker::IMUTracker(size_t buffer_size)
    : position(Eigen::Vector3d::Zero()),
      velocity(Eigen::Vector3d::Zero()),
      orientation(Eigen::Quaterniond::Identity()),
      last_timestamp(std::nullopt),
      buffer_size(buffer_size) {}

Eigen::Matrix4d IMUTracker::update(const Eigen::Vector3d& accel,
                                   const Eigen::Vector3d& gyro,
                                   double timestamp) {
    if (!last_timestamp.has_value()) {
        last_timestamp = timestamp;
        return get_transform();
    }

    double dt = timestamp - last_timestamp.value();
    last_timestamp = timestamp;

    // Orientation update from gyro
    Eigen::Vector3d rotvec = gyro * dt;
    double angle = rotvec.norm();
    if (angle > 1e-12) {
        Eigen::Vector3d axis = rotvec.normalized();
        Eigen::AngleAxisd delta_rot(angle, axis);
        orientation = orientation * Eigen::Quaterniond(delta_rot);
        orientation.normalize();
    }

    // Filter accelerometer data and transform to world coordinates
    Eigen::Vector3d accel_filtered = filter_accel(accel);
    Eigen::Vector3d accel_world = orientation * accel_filtered;

    // Update velocity and position using simple Euler integration
    velocity += accel_world * dt;
    position += velocity * dt;

    return get_transform();
}

Eigen::Matrix4d IMUTracker::get_transform() const {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3,3>(0,0) = orientation.toRotationMatrix();
    T.block<3,1>(0,3) = position;
    return T;
}

Eigen::Vector3d IMUTracker::filter_accel(const Eigen::Vector3d& accel) {
    accel_buffer.push_back(accel);
    if (accel_buffer.size() > buffer_size) {
        accel_buffer.pop_front();
    }

    Eigen::Vector3d sum = Eigen::Vector3d::Zero();
    for (const auto& a : accel_buffer) {
        sum += a;
    }
    return sum / static_cast<double>(accel_buffer.size());
}
