#include "RobotConfig.h"
#include "RobotController.h"
#include "MotionPlanner.h"
#include "SocketClient.h"
#include "HRSDK.h"

#include <iostream>
#include <string>
#include <winsock2.h>

using namespace hiwinrobot;
using namespace System;
using namespace System::IO::Ports;

void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int) {}

int main() {
    RobotConfig config = DefaultRobotConfig();
    char sdk_ver[50] = { 0 };
    char hrss_ver[50] = { 0 };

    get_hrsdk_version(sdk_ver);
    std::cout << "SDK version: " << sdk_ver << std::endl;

    HROBOT device_id = open_connection(config.robot_ip, config.robot_port, callBack);
    if (device_id < 0) {
        std::cout << "connect failure." << std::endl;
        return -1;
    }

    RobotController robot(device_id);
    robot.initialize();
    get_hrss_version(device_id, hrss_ver);
    std::cout << "HRSS version: " << hrss_ver << std::endl;

    if (get_motor_state(device_id) == 0) {
        set_motor_state(device_id, 1);
    }

    double mom[6] = { 0, 0, 0, 0, 0, 0 };
    double pocket2[6] = { 4.138, 625.266, -49, -180, 0, 90 };
    double pocket2c[6] = { 0,0,0,0,0,0 };

    robot.callibration(pocket2, config.home);
    for (int i = 0; i < 6; i++) {
        pocket2c[i] = pocket2[i];
    }

    SerialPort port("COM5", 9600);
    port.Open();
    port.Write("1");

    SocketClient socket(config.socket_ip, config.socket_port);
    if (!socket.connect()) {
        std::cout << "connect socket failed" << std::endl;
        disconnect(device_id);
        return -2;
    }

    std::cout << "connect socket succes" << std::endl;

    String^ receivedData;
    String^ truedata = "1";
    String^ receivedData2;
    String^ truedata2 = "2";

    while (true)
    {
        robot.recon_to_home(config.home, config.recon);
        for (int i = 0; i < 6; i++) {
            pocket2[i] = pocket2c[i];
        }

        receivedData = port.ReadExisting();
        Sleep(100);
        std::cout << "WAIT" << std::endl;

        if (truedata == receivedData)
        {
            std::cout << "BUTTON opened" << std::endl;
            socket.send("1");

            if (robot.readMom(mom, socket.socket_handle())) {
                MotionPlanner planner;
                planner.hit_ball(device_id, mom, pocket2, config.home, pocket2c);
            }

            port.Write("A");
            std::cout << "ARDUINO WRITEED" << std::endl;

            Sleep(500);
            while (true)
            {
                receivedData2 = port.ReadExisting();
                if (truedata2 == receivedData2)
                    break;
            }
            port.Write("1");
        }
    }

    socket.disconnect();
    disconnect(device_id);
    return 0;
}
