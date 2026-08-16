from __future__ import annotations

import cv2

from ball_robot.camera import CameraManager
from ball_robot.config import load_config
from ball_robot.geometry import compute_target, format_shot_message, find_mom_near_ball
from ball_robot.socket_comm import SocketServer
from ball_robot.vision import BallDetector


def main():
    config = load_config()
    camera = CameraManager(config)
    detector = BallDetector(config)
    socket_server = SocketServer(config.socket.host, config.socket.port, config.socket.buffer_size)

    socket_server.start()
    print("Socket connected.")

    while True:
        frame = camera.get_frame()
        if frame is None:
            continue
        frame = camera.draw_roi(frame)
        image, pockets, mom, son_array = detector.detect(frame)
        cv2.imshow("yolo", image)

        key = cv2.waitKey(10) & 0xFF
        if key == 27:
            break

        if not son_array or not mom.centerx:
            continue

        val_son_x, val_son_y, val_poc_x, val_poc_y = find_mom_near_ball(son_array, mom, pockets)
        if val_son_x == 0 and val_son_y == 0:
            continue

        offset_x, offset_y, mom_ang = compute_target(val_son_x, val_son_y, val_poc_x, val_poc_y, mom, pockets, config.detection.ball_diameter_mm)
        message = format_shot_message(offset_x, offset_y, mom_ang, mom, pockets)
        socket_server.send_message(message)
        print(message)

    socket_server.close()
    camera.stop()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
