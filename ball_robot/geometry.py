import math

import numpy as np


def angle(x1, y1, x2, y2, x3, y3):
    v1 = np.array([x3 - x1, y3 - y1], dtype=float)
    v2 = np.array([x3 - x2, y3 - y2], dtype=float)
    denom = np.linalg.norm(v1) * np.linalg.norm(v2)
    if denom == 0:
        return 0.0
    cosine = np.dot(v1, v2) / denom
    cosine = np.clip(cosine, -1.0, 1.0)
    return float(np.degrees(np.arccos(cosine)))


def cailibration(xx, yy, mom_ang, pockets_o):
    if len(pockets_o) < 4:
        return 0.0, 0.0, mom_ang

    point0 = pockets_o[0]
    point2 = pockets_o[2]
    point3 = pockets_o[3]
    offset_x = (xx - point0.x1) / (point2.x2 - point0.x1)
    offset_y = (yy - point0.y1) / (point3.y2 - point0.y1)
    return offset_x, offset_y, mom_ang


def find_mom_near_ball(son_array, mom, pockets):
    if not son_array:
        return 0.0, 0.0, 0.0, 0.0

    min_dis = son_array[0].mom_dis
    mindis_num = 0
    for i in range(1, len(son_array)):
        if son_array[i].mom_dis < min_dis:
            min_dis = son_array[i].mom_dis
            mindis_num = i

    ang = [0.0] * len(pockets)
    valid_pockets = []
    for i in range(len(pockets)):
        if pockets[i] is None:
            continue
        ang[i] = angle(mom.centerx, mom.centery, pockets[i].centerx, pockets[i].centery, son_array[mindis_num].centerx, son_array[mindis_num].centery)
        if ang[i] <= 110:
            continue
        valid_pockets.append(pockets[i])

    if not valid_pockets:
        return son_array[mindis_num].centerx, son_array[mindis_num].centery, 0.0, 0.0

    poc_son_dis = []
    for pocket in valid_pockets:
        distance = np.linalg.norm(
            np.array([son_array[mindis_num].centerx, son_array[mindis_num].centery])
            - np.array([pocket.centerx, pocket.centery])
        )
        poc_son_dis.append(distance)

    if not poc_son_dis:
        return son_array[mindis_num].centerx, son_array[mindis_num].centery, 0.0, 0.0

    min_poc_dis_num = int(np.argmin(poc_son_dis))
    target = valid_pockets[min_poc_dis_num]

    if target.x1 < 160 and target.y1 < 240:
        aimx = target.x2 - 9
        aimy = target.y2 - 9
    elif target.x1 < 160 and target.y1 > 240:
        aimx = target.x2 - 9
        aimy = target.y1 + 9
    elif 160 < target.x1 < 480 and target.y1 < 240:
        aimx = (target.x1 + target.x2) / 2
        aimy = target.y2 - 9
    elif target.x1 > 480 and target.y1 < 240:
        aimx = target.x1 + 9
        aimy = target.y2 - 9
    elif 160 < target.x1 < 480 and target.y1 > 240:
        aimx = (target.x1 + target.x2) / 2
        aimy = target.y1 + 9
    elif target.x1 > 480 and target.y1 > 240:
        aimx = target.x1 + 9
        aimy = target.y1 + 9
    else:
        aimx = target.centerx
        aimy = target.centery

    return son_array[mindis_num].centerx, son_array[mindis_num].centery, aimx, aimy


def compute_target(val_son_x, val_son_y, val_poc_x, val_poc_y, mom, pockets_o, ball_diameter):
    poc_vector = np.array([val_poc_x - val_son_x, val_poc_y - val_son_y], dtype=float)
    two_ball_vec = np.array([val_son_x - mom.centerx, val_son_y - mom.centery], dtype=float)
    if np.linalg.norm(poc_vector) == 0:
        mom_vector = np.array([val_son_x - mom.centerx, val_son_y - mom.centery], dtype=float)
        mom_unit_vector = mom_vector / np.linalg.norm(mom_vector)
        mom_pos = ball_diameter * mom_unit_vector
        hit_pos = np.array([val_son_x - mom_unit_vector[0], val_son_y - mom_unit_vector[1]], dtype=float)
    else:
        poc_unit_vector = poc_vector / np.linalg.norm(poc_vector)
        hit_pos = ball_diameter * poc_unit_vector
        if val_poc_x > val_son_x and val_poc_y > val_son_y:
            hit_pos = np.array([val_son_x - hit_pos[0], val_son_y - hit_pos[1]], dtype=float)
        elif val_poc_x > val_son_x and val_poc_y < val_son_y:
            hit_pos = np.array([val_son_x - hit_pos[0], val_son_y - hit_pos[1]], dtype=float)
        elif val_poc_x < val_son_x and val_poc_y > val_son_y:
            hit_pos = np.array([val_son_x - hit_pos[0], val_son_y - hit_pos[1]], dtype=float)
        elif val_poc_x < val_son_x and val_poc_y < val_son_y:
            hit_pos = np.array([val_son_x - hit_pos[0], val_son_y - hit_pos[1]], dtype=float)

        if val_poc_x > 0 and val_poc_y > 0:
            mom_vector = np.array([hit_pos[0] - mom.centerx, hit_pos[1] - mom.centery], dtype=float)
            mom_unit_vector = mom_vector / np.linalg.norm(mom_vector)
            mom_pos = ball_diameter * mom_unit_vector
        else:
            mom_vector = np.array([val_son_x - mom.centerx, val_son_y - mom.centery], dtype=float)
            mom_unit_vector = mom_vector / np.linalg.norm(mom_vector)
            mom_pos = ball_diameter * mom_unit_vector

    if np.linalg.norm(two_ball_vec) <= 1.5 * ball_diameter:
        mom_vector = np.array([val_son_x - mom.centerx, val_son_y - mom.centery], dtype=float)
        mom_unit_vector = mom_vector / np.linalg.norm(mom_vector)
        mom_pos = ball_diameter * mom_unit_vector
        hit_pos = np.array([val_son_x, val_son_y], dtype=float)

    if hit_pos[0] > mom.centerx and hit_pos[1] > mom.centery:
        mom_pos[0] = mom.centerx - mom_pos[0]
        mom_pos[1] = mom.centery - mom_pos[1]
    elif hit_pos[0] > mom.centerx and hit_pos[1] < mom.centery:
        mom_pos[0] = mom.centerx - mom_pos[0]
        mom_pos[1] = mom.centery - mom_pos[1]
    elif hit_pos[0] < mom.centerx and hit_pos[1] > mom.centery:
        mom_pos[0] = mom.centerx - mom_pos[0]
        mom_pos[1] = mom.centery - mom_pos[1]
    elif hit_pos[0] < mom.centerx and hit_pos[1] < mom.centery:
        mom_pos[0] = mom.centerx - mom_pos[0]
        mom_pos[1] = mom.centery - mom_pos[1]

    mom_ang = angle(mom.centerx - 10, mom.centery, hit_pos[0], hit_pos[1], mom.centerx, mom.centery)
    if val_son_y > mom.centery:
        mom_ang *= -1

    offset_x, offset_y, mom_ang = cailibration(mom_pos[0], mom_pos[1], mom_ang, pockets_o)
    return offset_x, offset_y, mom_ang


def format_shot_message(offset_x, offset_y, mom_ang, mom, pockets):
    real_mm_x = 69.4 * offset_x
    real_mm_y = 36.4 * offset_y
    a = real_mm_x
    b = real_mm_y
    c = mom_ang

    if mom.centerx < pockets[2].x2 and mom.centerx > pockets[2].x1:
        d = 0
    elif mom.centery < pockets[1].y2 or mom.centerx < pockets[1].x2 or mom.centery > pockets[4].y1 or mom.centerx > pockets[3].x1:
        d = 1
    else:
        d = 0

    if mom_ang < 0:
        e = 1
    elif mom.centerx > (pockets[2].x1 - pockets[1].x1) * 3 / 4 + pockets[1].x1:
        e = 2
    else:
        e = 0

    if mom.centerx < 0:
        f = 1
    elif mom.centery > (pockets[4].y2 - pockets[1].y1) * 3 / 4 + pockets[1].y1:
        f = 2
    else:
        f = 0

    return f"{a:.3f} {b:.3f} {c:.3f} {d} {e} {f}"
