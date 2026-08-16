import json
from dataclasses import dataclass, field
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[1]
CONFIG_PATH = ROOT_DIR / "config" / "ball_robot_config.json"


@dataclass
class CameraConfig:
    width: int = 640
    height: int = 480
    fps: int = 30
    rotation_angle: float = 0.0
    roi_x: int = 42
    roi_y: int = 113
    roi_width: int = 562
    roi_height: int = 302


@dataclass
class DetectionConfig:
    model_path: str = "D:/hiwin/HIWIN2/1700.pt"
    pocket_conf: float = 0.67
    son_conf: float = 0.8
    mom_conf: float = 0.7
    ball_diameter_mm: float = 31.67
    table_scale_x: float = 69.4
    table_scale_y: float = 36.4


@dataclass
class SocketConfig:
    host: str = "localhost"
    port: int = 8888
    buffer_size: int = 1024


@dataclass
class RobotConfig:
    home: list[float] = field(default_factory=lambda: [0.0, 296.5, 414.922, 180.0, -16.167, 90.0])
    recon: list[float] = field(default_factory=lambda: [0.0, 274.974, 481.085, -180.0, -20.681, 90.0])
    ball_offset: float = 45.0
    ball_margin: float = 38.0
    motor_override: int = 100


@dataclass
class AppConfig:
    camera: CameraConfig
    detection: DetectionConfig
    socket: SocketConfig
    robot: RobotConfig

    @classmethod
    def from_file(cls, path: Path | str = CONFIG_PATH) -> "AppConfig":
        config_path = Path(path)
        with config_path.open("r", encoding="utf-8") as file:
            data = json.load(file)

        camera = CameraConfig(
            width=data["camera"]["width"],
            height=data["camera"]["height"],
            fps=data["camera"]["fps"],
            rotation_angle=data["camera"]["rotation_angle"],
            roi_x=data["camera"]["roi"]["x"],
            roi_y=data["camera"]["roi"]["y"],
            roi_width=data["camera"]["roi"]["width"],
            roi_height=data["camera"]["roi"]["height"],
        )
        detection = DetectionConfig(
            model_path=data["detection"]["model_path"],
            pocket_conf=data["detection"]["pocket_conf"],
            son_conf=data["detection"]["son_conf"],
            mom_conf=data["detection"]["mom_conf"],
            ball_diameter_mm=data["detection"]["ball_diameter_mm"],
            table_scale_x=data["detection"]["table_scale"]["x"],
            table_scale_y=data["detection"]["table_scale"]["y"],
        )
        socket_cfg = SocketConfig(
            host=data["socket"]["host"],
            port=data["socket"]["port"],
            buffer_size=data["socket"]["buffer_size"],
        )
        robot = RobotConfig(
            home=data["robot"]["home"],
            recon=data["robot"]["recon"],
            ball_offset=data["robot"]["ball_offset"],
            ball_margin=data["robot"]["ball_margin"],
            motor_override=data["robot"]["motor_override"],
        )
        return cls(camera=camera, detection=detection, socket=socket_cfg, robot=robot)


def load_config(path: Path | str = CONFIG_PATH) -> AppConfig:
    return AppConfig.from_file(path)


if __name__ == "__main__":
    config = load_config()
    print(config)
