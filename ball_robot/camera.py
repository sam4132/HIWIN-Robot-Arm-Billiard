from __future__ import annotations

import cv2
import numpy as np
import pyrealsense2 as rs


class CameraManager:
    def __init__(self, config):
        self.config = config
        self.pipeline = rs.pipeline()
        self.config_rs = rs.config()
        self.config_rs.enable_stream(rs.stream.color, config.camera.width, config.camera.height, rs.format.bgr8, config.camera.fps)
        self.pipeline.start(self.config_rs)

    def get_frame(self):
        frames = self.pipeline.wait_for_frames()
        color_frame = frames.get_color_frame()
        if not color_frame:
            return None
        color_image = np.asanyarray(color_frame.get_data())
        rotation_matrix = cv2.getRotationMatrix2D(
            (color_image.shape[1] // 2, color_image.shape[0] // 2),
            self.config.camera.rotation_angle,
            1,
        )
        rotated = cv2.warpAffine(
            color_image,
            rotation_matrix,
            (color_image.shape[1], color_image.shape[0]),
        )
        return rotated

    def draw_roi(self, image):
        roi = self.config.camera
        cv2.rectangle(
            image,
            (roi.roi_x, roi.roi_y),
            (roi.roi_x + roi.roi_width, roi.roi_y + roi.roi_height),
            (0, 255, 0),
            1,
        )
        return image

    def stop(self):
        self.pipeline.stop()
