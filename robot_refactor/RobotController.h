#pragma once

#include "HRSDK.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <windows.h>
#include <conio.h>

namespace hiwinrobot {

class RobotController {
public:
    RobotController(HROBOT deviceId) : device_id_(deviceId) {}

    void initialize() {
        clear_alarm(device_id_);
        set_override_ratio(device_id_, 100);
        set_acc_dec_ratio(device_id_, 30);
    }

    void moveToHome(const double home[6]) {
        ptp_pos(device_id_, 0, home);
        Sleep(500);
        waitUntilIdle();
    }

    void moveToPosition(const double target[6]) {
        ptp_pos(device_id_, 0, target);
        Sleep(500);
        waitUntilIdle();
    }

    void moveLinear(const double target[6]) {
        lin_pos(device_id_, 0, 0, target);
        Sleep(500);
        waitUntilIdle();
    }

    void waitUntilIdle() {
        while (get_motion_state(device_id_) == 5) {
            Sleep(200);
        }
    }

    void setOverrideRatio(int ratio) {
        set_override_ratio(device_id_, ratio);
    }

    bool isReachable(const double target[6]) const {
        bool reachable = true;
        motion_reachable(device_id_, const_cast<double*>(target), reachable);
        return reachable;
    }

    void recon_to_home(const double home[6], const double recon[6]) {
        ptp_pos(device_id_, 0, home);
        Sleep(2000);
        lin_pos(device_id_, 0, 0, recon);
        Sleep(2000);
    }

    void callibration(double pocket2[6], const double home[6]) {
        ptp_pos(device_id_, 0, home);
        Sleep(3000);
        ptp_pos(device_id_, 0, pocket2);
        Sleep(3000);
        set_override_ratio(device_id_, 10);

        while (true) {
            char keyNum = _getch();
            if (keyNum == 27) {
                break;
            }
            double pos[6] = { 0,0,0,0,0,0 };
            std::cout << std::endl;
            JogByKeyDown(device_id_, keyNum, 0);
            get_current_position(device_id_, pos);
            for (int i = 0; i < 6; i++) {
                std::cout << pos[i] << " ";
                pocket2[i] = pos[i];
            }
            WaitKeUp(device_id_);
        }

        set_override_ratio(device_id_, 100);
        ptp_pos(device_id_, 0, home);
        Sleep(3000);
    }

    void JogByKeyDown(HROBOT id, char keyNum, int type) {
        switch (keyNum) {
        case 'A':
        case 'a':
            jog(id, type, 0, 1);
            break;
        case 'D':
        case 'd':
            jog(id, type, 0, -1);
            break;
        case 'W':
        case 'w':
            jog(id, type, 1, 1);
            break;
        case 'S':
        case 's':
            jog(id, type, 1, -1);
            break;
        case 'R':
        case 'r':
            jog(id, type, 2, 1);
            break;
        case 'F':
        case 'f':
            jog(id, type, 2, -1);
            break;
        }
    }

    void WaitKeUp(HROBOT id) {
        while (GetAsyncKeyState(81) < 0 ||
            GetAsyncKeyState(87) < 0 ||
            GetAsyncKeyState(65) < 0 ||
            GetAsyncKeyState(83) < 0 ||
            GetAsyncKeyState(90) < 0 ||
            GetAsyncKeyState(88) < 0 ||
            GetAsyncKeyState(69) < 0 ||
            GetAsyncKeyState(82) < 0 ||
            GetAsyncKeyState(68) < 0 ||
            GetAsyncKeyState(70) < 0 ||
            GetAsyncKeyState(67) < 0 ||
            GetAsyncKeyState(86) < 0) {
            Sleep(5);
        }
        jog_stop(id);
    }

    bool readMom(double* mom, SOCKET sockClient) {
    retry:
        char recvBuf[80] = { 0 };
        recv(sockClient, recvBuf, 80, 0);
        if (strcmp(recvBuf, "a") == 0) {
            return false;
        }
        if (!recvBuf[0]) {
            goto retry;
        }
        std::cout << recvBuf << std::endl;

        float num1 = 0, num2 = 0, num3 = 0;
        sscanf(recvBuf, "%f %f %f", &num1, &num2, &num3);

        mom[0] = static_cast<double>(num1) * 10.0;
        mom[1] = static_cast<double>(num2) * -10.0;
        mom[5] = static_cast<double>(num3) * -1.0;
        return true;
    }

private:
    HROBOT device_id_;
};

} // namespace hiwinrobot
