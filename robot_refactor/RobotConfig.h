#pragma once

#include <array>

namespace hiwinrobot {

struct RobotConfig {
    const char* robot_ip = "169.254.176.54";
    int robot_port = 1;
    const char* socket_ip = "127.0.0.1";
    int socket_port = 8888;
    const char* serial_port = "COM5";
    int serial_baud = 9600;
    int motor_override = 100;
    int ball_dis = 45;
    double home[6] = { 0.0, 0.0, 0.0, 0.0, -90.0, 0.0 };
    double recon[6] = { 0.0, 312.846, 501.48, -180.0, -15.397, 90.0 };
    double pocket_reference[6] = { 4.138, 531.653, 60.0, -180.0, 0.0, 90.0 };
    double calibrate_offset_x = 38.0;
    double calibrate_offset_y = 38.0;
    double calibrate_rotation = 180.0;
};

inline RobotConfig DefaultRobotConfig() {
    return RobotConfig{};
}

} // namespace hiwinrobot
