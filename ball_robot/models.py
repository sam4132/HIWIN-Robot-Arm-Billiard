from dataclasses import dataclass, field


@dataclass
class Pocket:
    x1: int = 0
    y1: int = 0
    x2: int = 0
    y2: int = 0
    centerx: float = 0.0
    centery: float = 0.0


@dataclass
class Mom:
    x1: int = 0
    y1: int = 0
    x2: int = 0
    y2: int = 0
    centerx: float = 0.0
    centery: float = 0.0
    conf: float = 0.0


@dataclass
class Son:
    x1: int = 0
    y1: int = 0
    x2: int = 0
    y2: int = 0
    centerx: float = 0.0
    centery: float = 0.0
    mom_dis: float = 0.0


@dataclass
class ShotTarget:
    offset_x: float = 0.0
    offset_y: float = 0.0
    mom_ang: float = 0.0
    color_image: object | None = None


@dataclass
class DetectionState:
    label: list[int] = field(default_factory=list)
    son_array: list[Son] = field(default_factory=list)
    son_num: int = 0
    recongnized: int = 0
    count: str = "0"
    pocket: int = 0
    poc_son_dis: list[float] = field(default_factory=lambda: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
    ang: list[float] = field(default_factory=lambda: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
    val_son_x: float = 0.0
    val_son_y: float = 0.0
    val_poc_x: float = 0.0
    val_poc_y: float = 0.0
    ball_diameter: float = 0.0
    offset_x: float = 0.0
    offset_y: float = 0.0
    mom_ang: float = 0.0
