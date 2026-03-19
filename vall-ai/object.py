from __future__ import annotations

from dataclasses import dataclass, field, asdict
from enum import Enum
from typing import Optional, Tuple, Dict, Any
import time
import math


class TargetType(str, Enum):
    F16 = "F16"
    HELICOPTER = "HELICOPTER"
    BALLISTIC_MISSILE = "BALLISTIC_MISSILE"
    MINI_MICRO_UAV = "MINI_MICRO_UAV"
    UNKNOWN = "UNKNOWN"


class Side(str, Enum):
    FRIEND = "FRIEND"
    FOE = "FOE"
    UNKNOWN = "UNKNOWN"


class RecommendedAction(str, Enum):
    ENGAGE = "ENGAGE"
    TRACK = "TRACK"
    MONITOR = "MONITOR"
    IGNORE = "IGNORE"
    NONE = "NONE"


class ObjectState(str, Enum):
    DETECTED = "DETECTED"
    TRACKED = "TRACKED"
    LOST = "LOST"
    DESTROYED = "DESTROYED"
    UNKNOWN = "UNKNOWN"


@dataclass
class DetectedObject:

    # Identity / time
    object_id: Optional[int] = None
    track_id: Optional[int] = None
    frame_index: Optional[int] = None
    timestamp: Optional[float] = field(default_factory=time.time)

    # Detection status
    object_detected: Optional[bool] = None
    state: Optional[ObjectState] = None

    # Classification
    target_type: Optional[TargetType] = None
    side: Optional[Side] = None
    shape_label: Optional[str] = None
    dominant_color: Optional[str] = None

    # Bounding box and image coordinates
    bbox: Optional[Tuple[float, float, float, float]] = None   # (x1, y1, x2, y2)
    bbox_width: Optional[float] = None
    bbox_height: Optional[float] = None
    bbox_area: Optional[float] = None

    object_center_px: Optional[Tuple[float, float]] = None
    balloon_center_px: Optional[Tuple[float, float]] = None
    aim_point_px: Optional[Tuple[float, float]] = None

    # World / targeting data
    distance_m: Optional[float] = None
    azimuth_deg: Optional[float] = None
    elevation_deg: Optional[float] = None

    # Motion / tracking data
    velocity_px_s: Optional[Tuple[float, float]] = None
    speed_px_s: Optional[float] = None
    direction_deg: Optional[float] = None
    approaching: Optional[bool] = None
    track_length: Optional[int] = None

    # Confidence scores
    detection_conf: Optional[float] = None
    tracking_conf: Optional[float] = None
    shape_conf: Optional[float] = None
    color_conf: Optional[float] = None
    target_type_conf: Optional[float] = None
    friend_foe_conf: Optional[float] = None
    balloon_conf: Optional[float] = None
    overall_conf: Optional[float] = None

    # Engagement / logic data
    is_in_valid_range: Optional[bool] = None
    inside_motion_forbidden_zone: Optional[bool] = None
    inside_fire_forbidden_zone: Optional[bool] = None
    inside_valid_engagement_zone: Optional[bool] = None

    priority_score: Optional[float] = None
    recommended_action: Optional[RecommendedAction] = None

    # Extra info
    color_features: Dict[str, Any] = field(default_factory=dict)
    shape_features: Dict[str, Any] = field(default_factory=dict)
    metadata: Dict[str, Any] = field(default_factory=dict)
    notes: Optional[str] = None

    def __post_init__(self) -> None:
        if self.bbox is not None:
            x1, y1, x2, y2 = self.bbox

            if self.bbox_width is None:
                self.bbox_width = max(0.0, x2 - x1)

            if self.bbox_height is None:
                self.bbox_height = max(0.0, y2 - y1)

            if self.bbox_area is None and self.bbox_width is not None and self.bbox_height is not None:
                self.bbox_area = self.bbox_width * self.bbox_height

            if self.object_center_px is None:
                self.object_center_px = ((x1 + x2) / 2.0, (y1 + y2) / 2.0)

        if self.aim_point_px is None:
            self.aim_point_px = self.balloon_center_px or self.object_center_px

        if self.velocity_px_s is not None and self.speed_px_s is None:
            vx, vy = self.velocity_px_s
            self.speed_px_s = math.sqrt(vx ** 2 + vy ** 2)

        if self.overall_conf is None:
            confs = [
                self.detection_conf,
                self.tracking_conf,
                self.shape_conf,
                self.color_conf,
                self.target_type_conf,
                self.friend_foe_conf,
                self.balloon_conf,
            ]
            valid_confs = [c for c in confs if c is not None]
            if valid_confs:
                self.overall_conf = sum(valid_confs) / len(valid_confs)

        if self.object_detected is None:
            self.object_detected = self.bbox is not None or self.object_center_px is not None

        if self.state is None and self.object_detected is True:
            self.state = ObjectState.DETECTED

        if self.recommended_action is None:
            self.recommended_action = RecommendedAction.NONE

    @property
    def has_bbox(self) -> bool:
        return self.bbox is not None

    @property
    def has_aim_point(self) -> bool:
        return self.aim_point_px is not None

    @property
    def is_friend(self) -> bool:
        return self.side == Side.FRIEND

    @property
    def is_foe(self) -> bool:
        return self.side == Side.FOE

    def update_bbox(self, bbox: Tuple[float, float, float, float]) -> None:
        self.bbox = bbox
        x1, y1, x2, y2 = bbox
        self.bbox_width = max(0.0, x2 - x1)
        self.bbox_height = max(0.0, y2 - y1)
        self.bbox_area = self.bbox_width * self.bbox_height
        self.object_center_px = ((x1 + x2) / 2.0, (y1 + y2) / 2.0)

        if self.balloon_center_px is None:
            self.aim_point_px = self.object_center_px

    def update_balloon_center(self, center: Tuple[float, float]) -> None:
        self.balloon_center_px = center
        self.aim_point_px = center

    def update_motion(self, previous_center_px: Tuple[float, float], dt: float) -> None:
        if dt <= 0:
            return

        current_center = self.object_center_px or self.aim_point_px
        if current_center is None:
            return

        vx = (current_center[0] - previous_center_px[0]) / dt
        vy = (current_center[1] - previous_center_px[1]) / dt

        self.velocity_px_s = (vx, vy)
        self.speed_px_s = math.sqrt(vx ** 2 + vy ** 2)
        self.direction_deg = math.degrees(math.atan2(vy, vx))

    def calculate_overall_conf(self) -> Optional[float]:
        confs = [
            self.detection_conf,
            self.tracking_conf,
            self.shape_conf,
            self.color_conf,
            self.target_type_conf,
            self.friend_foe_conf,
            self.balloon_conf,
        ]
        valid_confs = [c for c in confs if c is not None]

        if not valid_confs:
            self.overall_conf = None
            return None

        self.overall_conf = sum(valid_confs) / len(valid_confs)
        return self.overall_conf

    def to_dict(self) -> Dict[str, Any]:
        data = asdict(self)

        if self.target_type is not None:
            data["target_type"] = self.target_type.value

        if self.side is not None:
            data["side"] = self.side.value

        if self.recommended_action is not None:
            data["recommended_action"] = self.recommended_action.value

        if self.state is not None:
            data["state"] = self.state.value

        return data

    def to_controller_packet(self) -> Dict[str, Any]:
        return {
            "object_id": self.object_id,
            "track_id": self.track_id,
            "frame_index": self.frame_index,
            "timestamp": self.timestamp,
            "target_type": self.target_type.value if self.target_type else None,
            "side": self.side.value if self.side else None,
            "object_center_px": self.object_center_px,
            "balloon_center_px": self.balloon_center_px,
            "aim_point_px": self.aim_point_px,
            "distance_m": self.distance_m,
            "azimuth_deg": self.azimuth_deg,
            "elevation_deg": self.elevation_deg,
            "priority_score": self.priority_score,
            "overall_conf": self.overall_conf,
            "recommended_action": self.recommended_action.value if self.recommended_action else None,
        }

    @staticmethod
    def no_detection() -> None:
        return None