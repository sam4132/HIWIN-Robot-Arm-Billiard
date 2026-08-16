#pragma once

#include "HRSDK.h"
#include <cmath>
#include <iostream>

#define M_PI 3.14159265358979323846

namespace hiwinrobot {

class MotionPlanner {
public:
    MotionPlanner() = default;

    void applyTargetOffset(double target[6], const double mom[6], const double base_pose[6], double pocket2c[6]) {
        target[0] = base_pose[0] - 346.0;
        for (int i = 0; i < 6; ++i) {
            target[i] = pocket2c[i] + mom[i];
        }
    }

    void offset(double pocket2[6], const double mom[6]) {
        const double radian = mom[5] * (M_PI / 180.0);
        const double cos_value = std::cos(radian);
        const double sin_value = std::sin(radian);

        double offsetValue[6] = { 0,0,0,0,0,0 };
        offsetValue[0] = (45.0 * cos_value) - (38.0 * sin_value);
        offsetValue[1] = (45.0 * sin_value) + (38.0 * cos_value);

        pocket2[0] = offsetValue[0] + pocket2[0];
        pocket2[1] = offsetValue[1] + pocket2[1];
    }

    void hit_ball(HROBOT device_id, double* mom, double pocket2[6], const double home[6], const double pocket2c[6]) {
        bool is_reachable = true;
        ptp_pos(device_id, 0, home);
        Sleep(1000);
        pocket2[0] = pocket2[0] - 346;

        for (int i = 0; i < 6; i++) {
            pocket2[i] = pocket2[i] + mom[i];
            std::cout << pocket2[i] << std::endl;
        }

        offset(pocket2, mom);

    retry:
        motion_reachable(device_id, pocket2, is_reachable);
        if (is_reachable) {
            ptp_pos(device_id, 0, pocket2);
            std::cout << "yes its reachable\n";
            Sleep(5000);
            return;
        }
        else if (pocket2[1] < 400 && pocket2[0] > 19 && pocket2[0] < 64) {
            pocket2[1] = pocket2[1] + 1;
            pocket2[2] = pocket2[2] - 0.1;
            std::cout << "cant go there pocket2[1] < 400\n";
            goto retry;
        }
        else if (pocket2[1] > 400 && pocket2[0] > 19 && pocket2[0] < 64) {
            pocket2[1] = pocket2[1] - 1;
            pocket2[2] = pocket2[2] - 0.1;
            std::cout << "cant go there pocket2[1] < 400\n";
            goto retry;
        }
        else {
            if (pocket2[1] < 400 && pocket2[0] > 19)
            {
                for (int i = 0; i < 20; i++)
                {
                    pocket2[0] = pocket2[0] - 1;
                    pocket2[1] = pocket2[1] + 1;
                    motion_reachable(device_id, pocket2, is_reachable);
                    if (is_reachable) {
                        goto retry;
                    }
                }
            }
            else if (pocket2[1] < 400 && pocket2[0] < 19)
            {
                for (int i = 0; i < 20; i++)
                {
                    pocket2[0] = pocket2[0] + 1;
                    pocket2[1] = pocket2[1] + 1;
                    motion_reachable(device_id, pocket2, is_reachable);
                    if (is_reachable) {
                        goto retry;
                    }
                }
            }
            else if (pocket2[1] > 400 && pocket2[0] < 19)
            {
                for (int i = 0; i < 20; i++)
                {
                    pocket2[0] = pocket2[0] + 1;
                    pocket2[1] = pocket2[1] - 1;
                    motion_reachable(device_id, pocket2, is_reachable);
                    if (is_reachable) {
                        goto retry;
                    }
                }
            }
            else if (pocket2[1] > 400 && pocket2[0] > 19)
            {
                for (int i = 0; i < 20; i++)
                {
                    pocket2[0] = pocket2[0] - 1;
                    pocket2[1] = pocket2[1] - 1;
                    motion_reachable(device_id, pocket2, is_reachable);
                    if (is_reachable) {
                        goto retry;
                    }
                }
            }
            else
            {
                mom[5] = mom[5] + 1;
                std::cout << "cant go there mom[5] = " << mom[5] << "\n";
                for (int i = 0; i < 6; i++) {
                    pocket2[i] = pocket2c[i];
                }
                pocket2[0] = pocket2[0] - 347.5;
                for (int i = 0; i < 6; i++) {
                    pocket2[i] = pocket2[i] + mom[i];
                    std::cout << pocket2[i] << std::endl;
                }
                offset(pocket2, mom);
                goto retry;
            }
        }
    }

    void applyReachabilityAdjustment(HROBOT device_id, double target[6]) {
        bool reachable = true;
        int attempts = 0;

        while (attempts < 20) {
            motion_reachable(device_id, target, reachable);
            if (reachable) {
                return;
            }

            target[0] -= 1.0;
            target[1] += 1.0;
            ++attempts;
        }
    }

    void offsetForAngle(double target[6], const double mom[6]) {
        const double radian = mom[5] * (M_PI / 180.0);
        const double cos_value = std::cos(radian);
        const double sin_value = std::sin(radian);

        double offsetValue[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        offsetValue[0] = (45.0 * cos_value) - (38.0 * sin_value);
        offsetValue[1] = (45.0 * sin_value) + (38.0 * cos_value);

        target[0] += offsetValue[0];
        target[1] += offsetValue[1];
    }
};

} // namespace hiwinrobot
