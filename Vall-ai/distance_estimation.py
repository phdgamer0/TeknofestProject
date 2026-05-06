
from __future__ import annotations

from typing import Dict, Iterable, List, Optional, Tuple

from object import DetectedObject, DistanceEstimationMethod, TargetType


# Approximate real target dimensions in meters.
# These values are placeholders for monocular distance estimation and
# should be replaced with the exact dimensions you measure from your
# competition targets or printed mock targets.
TARGET_REAL_DIMENSIONS_M: Dict[TargetType, Dict[str, float]] = {
    TargetType.F16: {"width": 0.70, "height": 0.45},
    TargetType.HELICOPTER: {"width": 0.65, "height": 0.50},
    TargetType.BALLISTIC_MISSILE: {"width": 0.30, "height": 0.85},
    TargetType.MINI_MICRO_UAV: {"width": 0.40, "height": 0.18},
}


def estimate_distance_from_known_width(
    bbox_width_px: Optional[float],
    focal_length_px: float,
    real_width_m: float,
) -> Optional[float]:
    """
    Monocular pinhole-model distance estimation with known object width.

    distance_m = (real_width_m * focal_length_px) / bbox_width_px
    """

    if bbox_width_px is None or bbox_width_px <= 0:
        return None

    if focal_length_px <= 0 or real_width_m <= 0:
        return None

    return (real_width_m * focal_length_px) / bbox_width_px



def estimate_distance_from_known_height(
    bbox_height_px: Optional[float],
    focal_length_px: float,
    real_height_m: float,
) -> Optional[float]:
    """
    Monocular pinhole-model distance estimation with known object height.

    distance_m = (real_height_m * focal_length_px) / bbox_height_px
    """

    if bbox_height_px is None or bbox_height_px <= 0:
        return None

    if focal_length_px <= 0 or real_height_m <= 0:
        return None

    return (real_height_m * focal_length_px) / bbox_height_px



def estimate_distance_confidence(
    obj: DetectedObject,
    image_width_px: Optional[int] = None,
    image_height_px: Optional[int] = None,
) -> float:
    """
    Simple heuristic confidence for range estimation.

    Higher when:
    - object has a bbox
    - bbox is large enough in the image
    - overall confidence is high
    """

    score = 0.0

    if obj.has_bbox:
        score += 0.35

    if obj.overall_conf is not None:
        score += min(max(obj.overall_conf, 0.0), 1.0) * 0.45

    if obj.bbox_area is not None and obj.bbox_area > 0:
        if image_width_px is not None and image_height_px is not None and image_width_px > 0 and image_height_px > 0:
            image_area = image_width_px * image_height_px
            if image_area > 0:
                bbox_ratio = obj.bbox_area / image_area
                score += min(bbox_ratio * 12.0, 0.20)
        else:
            score += 0.10

    return min(score, 1.0)



def estimate_object_distance(
    obj: DetectedObject,
    focal_length_px: float,
    target_dimensions_m: Optional[Dict[TargetType, Dict[str, float]]] = None,
    image_size: Optional[Tuple[int, int]] = None,
) -> Optional[float]:
    """
    Estimates distance for a detected object from its bbox size.

    Priority:
    1. width-based estimation when width info exists
    2. height-based estimation when height info exists

    Updates the object in-place:
    - distance_m
    - distance_conf
    - distance_method
    """

    if obj is None or obj.target_type is None:
        return None

    dimensions_source = target_dimensions_m or TARGET_REAL_DIMENSIONS_M
    real_dims = dimensions_source.get(obj.target_type)

    if real_dims is None:
        return None

    image_width_px: Optional[int] = None
    image_height_px: Optional[int] = None
    if image_size is not None:
        image_width_px, image_height_px = image_size

    distance_m: Optional[float] = None
    method: Optional[DistanceEstimationMethod] = None

    real_width_m = real_dims.get("width")
    real_height_m = real_dims.get("height")

    if real_width_m is not None:
        distance_m = estimate_distance_from_known_width(
            bbox_width_px=obj.bbox_width,
            focal_length_px=focal_length_px,
            real_width_m=real_width_m,
        )
        if distance_m is not None:
            method = DistanceEstimationMethod.BBOX_WIDTH

    if distance_m is None and real_height_m is not None:
        distance_m = estimate_distance_from_known_height(
            bbox_height_px=obj.bbox_height,
            focal_length_px=focal_length_px,
            real_height_m=real_height_m,
        )
        if distance_m is not None:
            method = DistanceEstimationMethod.BBOX_HEIGHT

    if distance_m is None:
        return None

    confidence = estimate_distance_confidence(
        obj=obj,
        image_width_px=image_width_px,
        image_height_px=image_height_px,
    )

    obj.update_distance(
        distance_m=distance_m,
        confidence=confidence,
        method=method,
    )

    return distance_m



def estimate_distances_for_objects(
    objects: Iterable[DetectedObject],
    focal_length_px: float,
    target_dimensions_m: Optional[Dict[TargetType, Dict[str, float]]] = None,
    image_size: Optional[Tuple[int, int]] = None,
) -> List[DetectedObject]:
    """
    Batch distance estimation for all detected objects.
    Returns the same objects after updating them in-place.
    """

    updated_objects: List[DetectedObject] = []

    for obj in objects:
        if obj is None:
            continue

        estimate_object_distance(
            obj=obj,
            focal_length_px=focal_length_px,
            target_dimensions_m=target_dimensions_m,
            image_size=image_size,
        )
        updated_objects.append(obj)

    return updated_objects



def calculate_focal_length_px(
    measured_distance_m: float,
    real_width_m: float,
    measured_width_px: float,
) -> Optional[float]:
    """
    Calibration helper:
    Use one sample image with known distance and known target width.

    focal_length_px = (measured_width_px * measured_distance_m) / real_width_m
    """

    if measured_distance_m <= 0 or real_width_m <= 0 or measured_width_px <= 0:
        return None

    return (measured_width_px * measured_distance_m) / real_width_m