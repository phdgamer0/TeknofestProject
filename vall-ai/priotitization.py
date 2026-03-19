from __future__ import annotations

from typing import Optional, Iterable, List

from object import DetectedObject, TargetType, Side, RecommendedAction


def is_in_valid_range(obj: DetectedObject) -> Optional[bool]:

    #LOGIC
    """
    Range rules based on the current competition logic:
    - F16: 10-15 m
    - HELICOPTER / BALLISTIC_MISSILE: 5-15 m
    - MINI_MICRO_UAV: 0-15 m

    Returns:
        True / False if distance and type are known
        None if range cannot be evaluated yet
    """

    if obj.distance_m is None or obj.target_type is None:
        return None

    d = obj.distance_m

    if obj.target_type == TargetType.F16:
        return 10.0 <= d <= 15.0

    if obj.target_type in (TargetType.HELICOPTER, TargetType.BALLISTIC_MISSILE):
        return 5.0 <= d <= 15.0

    if obj.target_type == TargetType.MINI_MICRO_UAV:
        return 0.0 <= d <= 15.0

    return None


def calculate_priority_score(obj: DetectedObject) -> float:

    #LOGIC
    """
    Calculates a priority score for a detected object.

    Higher score = more important target.

    Notes:
    - Friends should never be prioritized
    - Objects in forbidden zones should be heavily penalized
    - Stronger confidence, valid range, approaching motion,
      and important target types increase score
    """
    
    if obj is None:
        return -1000.0

    score = 0.0

    # Ignore non-detected objects
    if obj.object_detected is False:
        obj.priority_score = -1000.0
        obj.recommended_action = RecommendedAction.IGNORE
        return obj.priority_score

    # Friend/foe rule
    if obj.side == Side.FRIEND:
        obj.priority_score = -1000.0
        obj.recommended_action = RecommendedAction.IGNORE
        return obj.priority_score

    # Base confidence contribution
    overall_conf = obj.overall_conf if obj.overall_conf is not None else obj.calculate_overall_conf()
    if overall_conf is not None:
        score += overall_conf * 35.0

    # Target type weights
    type_weights = {
        TargetType.F16: 30.0,
        TargetType.BALLISTIC_MISSILE: 24.0,
        TargetType.HELICOPTER: 20.0,
        TargetType.MINI_MICRO_UAV: 14.0,
        TargetType.UNKNOWN: 5.0,
    }

    if obj.target_type is not None:
        score += type_weights.get(obj.target_type, 0.0)

    # Approaching targets are more urgent
    if obj.approaching is True:
        score += 15.0

    # Longer stable tracks are generally more reliable
    if obj.track_length is not None:
        score += min(obj.track_length, 15)

    # Prefer targets with a real aim point
    if obj.has_aim_point:
        score += 8.0

    # Prefer targets with bbox
    if obj.has_bbox:
        score += 5.0

    # Range check
    valid_range = is_in_valid_range(obj)
    obj.is_in_valid_range = valid_range

    if valid_range is True:
        score += 20.0
    elif valid_range is False:
        score -= 10.0

    # Forbidden zones
    if obj.inside_motion_forbidden_zone is True:
        score -= 25.0

    if obj.inside_fire_forbidden_zone is True:
        score -= 60.0

    # Valid engagement zone bonus
    if obj.inside_valid_engagement_zone is True:
        score += 10.0
    elif obj.inside_valid_engagement_zone is False:
        score -= 20.0

    obj.priority_score = score

    # Recommended action
    if obj.side == Side.FRIEND:
        obj.recommended_action = RecommendedAction.IGNORE
    elif obj.inside_fire_forbidden_zone is True:
        obj.recommended_action = RecommendedAction.IGNORE
    elif score >= 75:
        obj.recommended_action = RecommendedAction.ENGAGE
    elif score >= 45:
        obj.recommended_action = RecommendedAction.TRACK
    elif score >= 15:
        obj.recommended_action = RecommendedAction.MONITOR
    else:
        obj.recommended_action = RecommendedAction.IGNORE

    return score


def sort_objects_by_priority(objects: Iterable[DetectedObject]) -> List[DetectedObject]:

    #LOGIC
    """
    Returns objects sorted from highest priority to lowest priority.
    """

    valid_objects = [obj for obj in objects if obj is not None]

    for obj in valid_objects:
        calculate_priority_score(obj)

    return sorted(
        valid_objects,
        key=lambda obj: obj.priority_score if obj.priority_score is not None else -1000.0,
        reverse=True
    )


def select_top_priority_object(objects: Iterable[DetectedObject]) -> Optional[DetectedObject]:

    #LOGIC
    """
    Returns the highest-priority object, or None if no valid object exists.
    """
    
    sorted_objects = sort_objects_by_priority(objects)
    return sorted_objects[0] if sorted_objects else None