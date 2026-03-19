from object import DetectedObject, TargetType, Side
from priotitization import select_top_priority_object
from matching import match_balloons_to_targets

obj1 = DetectedObject(
    object_id=1,
    target_type=TargetType.F16,
    side=Side.FOE,
    distance_m=12.0,
    approaching=True,
    track_length=8,
    overall_conf=0.91,
    bbox=(100, 80, 220, 180),
    balloon_center_px=(160, 120),
    inside_valid_engagement_zone=True,
)

obj2 = DetectedObject(
    object_id=2,
    target_type=TargetType.MINI_MICRO_UAV,
    side=Side.FOE,
    distance_m=7.0,
    approaching=False,
    track_length=4,
    overall_conf=0.87,
    bbox=(300, 100, 360, 170),
    inside_valid_engagement_zone=True,
)

objects = [obj1, obj2]

best_target = select_top_priority_object(objects)

if best_target is not None:
    print(best_target.object_id)
    print(best_target.priority_score)
    print(best_target.recommended_action)



targets = []
ballons = []


matches = match_balloons_to_targets(
    balloons=[],
    targets=[],
    image_size=(), #Still not defined.
)


for match in matches:
    print(match.to_dict())