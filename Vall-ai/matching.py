from __future__ import annotations

from dataclasses import dataclass
from typing import List, Optional, Tuple, Dict

from object import DetectedObject


@dataclass
class MatchResult:
    balloon: DetectedObject
    target: Optional[DetectedObject]
    score: float
    center_inside: bool
    containment_ratio: float
    normalized_distance: float
    tracking_consistency: float

    def to_dict(self) -> Dict:
        return {
            "balloon_object_id": self.balloon.object_id,
            "balloon_track_id": self.balloon.track_id,
            "target_object_id": self.target.object_id if self.target else None,
            "target_track_id": self.target.track_id if self.target else None,
            "score": self.score,
            "center_inside": self.center_inside,
            "containment_ratio": self.containment_ratio,
            "normalized_distance": self.normalized_distance,
            "tracking_consistency": self.tracking_consistency,
        }


def point_inside_bbox(point: Tuple[float, float], bbox: Tuple[float, float, float, float]) -> bool:
    x, y = point
    x1, y1, x2, y2 = bbox
    return x1 <= x <= x2 and y1 <= y <= y2


def bbox_area(bbox: Tuple[float, float, float, float]) -> float:
    x1, y1, x2, y2 = bbox
    return max(0.0, x2 - x1) * max(0.0, y2 - y1)


def intersection_area(
    bbox1: Tuple[float, float, float, float],
    bbox2: Tuple[float, float, float, float]
) -> float:
    x11, y11, x12, y12 = bbox1
    x21, y21, x22, y22 = bbox2

    inter_x1 = max(x11, x21)
    inter_y1 = max(y11, y21)
    inter_x2 = min(x12, x22)
    inter_y2 = min(y12, y22)

    if inter_x2 <= inter_x1 or inter_y2 <= inter_y1:
        return 0.0

    return (inter_x2 - inter_x1) * (inter_y2 - inter_y1)


def euclidean_distance(p1: Tuple[float, float], p2: Tuple[float, float]) -> float:
    return ((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2) ** 0.5


def get_object_center(obj: DetectedObject) -> Optional[Tuple[float, float]]:
    if obj.object_center_px is not None:
        return obj.object_center_px

    if obj.bbox is not None:
        x1, y1, x2, y2 = obj.bbox
        return ((x1 + x2) / 2.0, (y1 + y2) / 2.0)

    return None


def get_balloon_center(obj: DetectedObject) -> Optional[Tuple[float, float]]:
    if obj.balloon_center_px is not None:
        return obj.balloon_center_px

    if obj.aim_point_px is not None:
        return obj.aim_point_px

    if obj.object_center_px is not None:
        return obj.object_center_px

    if obj.bbox is not None:
        x1, y1, x2, y2 = obj.bbox
        return ((x1 + x2) / 2.0, (y1 + y2) / 2.0)

    return None


def compute_tracking_consistency(
    balloon: DetectedObject,
    target: DetectedObject,
    previous_matches: Optional[Dict[int, int]] = None
) -> float:
    
    #LOGIC
    """
    Returns 1.0 if previous frame matched this balloon to this target,
    otherwise 0.0.
    """
    
    if previous_matches is None:
        return 0.0

    if balloon.track_id is None or target.track_id is None:
        return 0.0

    previous_target_track_id = previous_matches.get(balloon.track_id)
    return 1.0 if previous_target_track_id == target.track_id else 0.0


def compute_match_score(
    balloon: DetectedObject,
    target: DetectedObject,
    image_size: Optional[Tuple[int, int]] = None,
    previous_matches: Optional[Dict[int, int]] = None,
) -> MatchResult:
    
    #LOGIC
    """
    Score components:
    - balloon center inside target bbox
    - containment ratio: overlap / balloon bbox area
    - normalized center distance
    - temporal consistency with previous frame
    """

    balloon_center = get_balloon_center(balloon)
    target_center = get_object_center(target)

    center_inside = False
    containment_ratio = 0.0
    normalized_distance = 1.0
    tracking_consistency = compute_tracking_consistency(balloon, target, previous_matches)

    if balloon_center is not None and target.bbox is not None:
        center_inside = point_inside_bbox(balloon_center, target.bbox)

    if balloon.bbox is not None and target.bbox is not None:
        balloon_area = bbox_area(balloon.bbox)
        if balloon_area > 0:
            containment_ratio = intersection_area(balloon.bbox, target.bbox) / balloon_area

    if balloon_center is not None and target_center is not None:
        distance = euclidean_distance(balloon_center, target_center)

        if image_size is not None:
            width, height = image_size
            image_diagonal = (width ** 2 + height ** 2) ** 0.5
            if image_diagonal > 0:
                normalized_distance = min(distance / image_diagonal, 1.0)
        elif target.bbox is not None:
            x1, y1, x2, y2 = target.bbox
            target_diag = ((x2 - x1) ** 2 + (y2 - y1) ** 2) ** 0.5
            if target_diag > 0:
                normalized_distance = min(distance / target_diag, 1.0)

    score = (
        50.0 * float(center_inside) +
        25.0 * containment_ratio +
        20.0 * (1.0 - normalized_distance) +
        15.0 * tracking_consistency
    )

    return MatchResult(
        balloon=balloon,
        target=target,
        score=score,
        center_inside=center_inside,
        containment_ratio=containment_ratio,
        normalized_distance=normalized_distance,
        tracking_consistency=tracking_consistency,
    )


def match_single_balloon_to_target(
    balloon: DetectedObject,
    targets: List[DetectedObject],
    image_size: Optional[Tuple[int, int]] = None,
    previous_matches: Optional[Dict[int, int]] = None,
    min_score_threshold: float = 20.0,
) -> MatchResult:
    
    #LOGIC
    """
    Matches one balloon to the best target.
    Returns MatchResult with target=None if no target is good enough.
    """

    valid_targets = [t for t in targets if t is not None]

    if not valid_targets:
        return MatchResult(
            balloon=balloon,
            target=None,
            score=-1.0,
            center_inside=False,
            containment_ratio=0.0,
            normalized_distance=1.0,
            tracking_consistency=0.0,
        )

    scored_results = [
        compute_match_score(
            balloon=balloon,
            target=target,
            image_size=image_size,
            previous_matches=previous_matches,
        )
        for target in valid_targets
    ]

    best_result = max(scored_results, key=lambda r: r.score)

    if best_result.score < min_score_threshold:
        return MatchResult(
            balloon=balloon,
            target=None,
            score=best_result.score,
            center_inside=best_result.center_inside,
            containment_ratio=best_result.containment_ratio,
            normalized_distance=best_result.normalized_distance,
            tracking_consistency=best_result.tracking_consistency,
        )

    return best_result


def match_balloons_to_targets(
    balloons: List[DetectedObject],
    targets: List[DetectedObject],
    image_size: Optional[Tuple[int, int]] = None,
    previous_matches: Optional[Dict[int, int]] = None,
    min_score_threshold: float = 20.0,
    unique_target_assignment: bool = True,
) -> List[MatchResult]:
    
    #LOGIC
    """
    Full matching algorithm.

    Parameters:
        balloons:
            List of balloon detections.
        targets:
            List of target detections.
        image_size:
            Optional (width, height) of image for distance normalization.
        previous_matches:
            Dict mapping balloon_track_id -> target_track_id from previous frame.
        min_score_threshold:
            Minimum score needed to accept a match.
        unique_target_assignment:
            If True, one target can only be assigned to one balloon.

    Returns:
        List[MatchResult], one result for each balloon.
    """


    valid_balloons = [b for b in balloons if b is not None]
    valid_targets = [t for t in targets if t is not None]

    if not valid_balloons:
        return []

    if not valid_targets:
        return [
            MatchResult(
                balloon=b,
                target=None,
                score=-1.0,
                center_inside=False,
                containment_ratio=0.0,
                normalized_distance=1.0,
                tracking_consistency=0.0,
            )
            for b in valid_balloons
        ]

    all_candidate_results: List[MatchResult] = []

    for balloon in valid_balloons:
        for target in valid_targets:
            result = compute_match_score(
                balloon=balloon,
                target=target,
                image_size=image_size,
                previous_matches=previous_matches,
            )
            all_candidate_results.append(result)

    all_candidate_results.sort(key=lambda r: r.score, reverse=True)

    used_balloon_ids = set()
    used_target_ids = set()
    final_matches: List[MatchResult] = []

    for result in all_candidate_results:
        balloon_id = id(result.balloon)
        target_id = id(result.target) if result.target is not None else None

        if result.score < min_score_threshold:
            continue

        if balloon_id in used_balloon_ids:
            continue

        if unique_target_assignment and target_id in used_target_ids:
            continue

        final_matches.append(result)
        used_balloon_ids.add(balloon_id)

        if unique_target_assignment and target_id is not None:
            used_target_ids.add(target_id)

    matched_balloon_ids = {id(r.balloon) for r in final_matches}

    for balloon in valid_balloons:
        if id(balloon) not in matched_balloon_ids:
            final_matches.append(
                MatchResult(
                    balloon=balloon,
                    target=None,
                    score=-1.0,
                    center_inside=False,
                    containment_ratio=0.0,
                    normalized_distance=1.0,
                    tracking_consistency=0.0,
                )
            )

    final_matches.sort(
        key=lambda r: (
            r.balloon.track_id if r.balloon.track_id is not None else 10**9,
            r.balloon.object_id if r.balloon.object_id is not None else 10**9,
        )
    )

    return final_matches


def build_previous_match_map(matches: List[MatchResult]) -> Dict[int, int]:

    #LOGIC
    """
    Builds:
        {balloon_track_id: target_track_id}
    from previous frame's accepted matches.
    """

    mapping: Dict[int, int] = {}

    for result in matches:
        if (
            result.balloon.track_id is not None and
            result.target is not None and
            result.target.track_id is not None
        ):
            mapping[result.balloon.track_id] = result.target.track_id

    return mapping