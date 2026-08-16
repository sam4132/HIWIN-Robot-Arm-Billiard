from __future__ import annotations

import cv2
import numpy as np
from ultralytics import YOLO

from .models import Pocket, Mom, Son


class BallDetector:
    def __init__(self, config):
        self.config = config
        self.model = YOLO(config.detection.model_path)
        self.font = cv2.FONT_HERSHEY_SIMPLEX
        self.font_scale = 0.5
        self.color = (255, 255, 255)
        self.thickness = 1
        self.line_type = cv2.LINE_AA

    def detect(self, image):
        results = self.model.predict(source=image, save=False)
        result = results[0]
        labels = [int(coord.item()) for coord in result.boxes.cls]
        confs = [float(coord.item()) for coord in result.boxes.conf]
        xyxy = result.boxes.xyxy

        pocket1 = Pocket(); pocket2 = Pocket(); pocket3 = Pocket(); pocket4 = Pocket(); pocket5 = Pocket(); pocket6 = Pocket()
        pockets = [pocket1, pocket2, pocket3, pocket4, pocket5, pocket6]
        mom = Mom()
        son_array = []
        biger_conf = 0.0

        for i in range(len(xyxy)):
            label = labels[i]
            conf = confs[i]
            x1, y1, x2, y2 = [int(coord.item()) for coord in xyxy[i]]

            if label == 1 and conf >= self.config.detection.pocket_conf:
                pockets = self._update_pocket(pockets, x1, y1, x2, y2)
                text = str(conf)
                org = (x1, y2)
                cv2.putText(image, text, org, self.font, self.font_scale, self.color, self.thickness, self.line_type)
            elif label == 0 and conf >= self.config.detection.mom_conf:
                y1 = y1 + 4
                y2 = y2 + 1
                cv2.rectangle(image, (x1, y1), (x2, y2), (0, 255, 0), 2)
                ball_diameter = ((x2 - x1) + (y2 - y1)) / 2 - 2
                if conf > biger_conf:
                    biger_conf = conf
                    mom = Mom(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2, conf)
                cv2.putText(image, f"mom{conf}", (x1, y2), self.font, self.font_scale, self.color, self.thickness, self.line_type)
            elif label == 2 and conf >= self.config.detection.son_conf:
                cv2.rectangle(image, (x1, y1), (x2, y2), (255, 255, 0), 2)
                cv2.putText(image, str(len(son_array)), (x1, y2), self.font, self.font_scale, self.color, self.thickness, self.line_type)
                son_array.append(Son(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2, 0.0))

        for son_obj in son_array:
            son_obj.mom_dis = np.linalg.norm(
                np.array([son_obj.centerx, son_obj.centery]) - np.array([mom.centerx, mom.centery])
            )

        return image, pockets, mom, son_array

    def _update_pocket(self, pockets, x1, y1, x2, y2):
        if x1 < 160 and y1 < 240:
            pockets[0] = Pocket(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2)
        elif x1 < 160 and y1 > 240:
            pockets[3] = Pocket(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2)
        elif 160 < x1 < 480 and y1 < 240:
            pockets[1] = Pocket(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2)
        elif x1 > 480 and y1 < 240:
            pockets[2] = Pocket(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2)
        elif 160 < x1 < 480 and y1 > 240:
            pockets[4] = Pocket(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2)
        elif x1 > 480 and y1 > 240:
            pockets[5] = Pocket(x1, y1, x2, y2, (x1 + x2) / 2, (y1 + y2) / 2)

        for pocket in pockets:
            if pocket.x1 != 0 or pocket.y1 != 0 or pocket.x2 != 0 or pocket.y2 != 0:
                cv2.rectangle(
                    self.model.model.names if False else None,
                    (0, 0),
                    (0, 0),
                    (0, 0, 0),
                    0,
                )
        return pockets
