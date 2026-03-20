from object import DetectedObject, TargetType, Side
from priotitization import select_top_priority_object
from matching import match_balloons_to_targets
from distance_estimation import estimate_object_distance, calculate_focal_length_px


obj1 = DetectedObject(
    object_id=1,
    target_type=TargetType.F16,
    side=Side.FOE,
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
    approaching=False,
    track_length=4,
    overall_conf=0.87,
    bbox=(300, 100, 360, 170),
    inside_valid_engagement_zone=True,
)


# Example focal length calculation from one measured calibration sample.
# Replace these values with your real measured setup values.
focal_length_px = calculate_focal_length_px(
    measured_distance_m=5.0,
    real_width_m=0.70,
    measured_width_px=140.0,
)

if focal_length_px is not None:
    estimate_object_distance(obj1, focal_length_px=focal_length_px, image_size=(1920, 1080))
    estimate_object_distance(obj2, focal_length_px=focal_length_px, image_size=(1920, 1080))

objects = [obj1, obj2]

best_target = select_top_priority_object(objects)

if best_target is not None:
    print(best_target.object_id)
    print(best_target.priority_score)
    print(best_target.recommended_action)
    print(best_target.distance_m)
    print(best_target.distance_conf)
    print(best_target.distance_method)


matches = match_balloons_to_targets(
    balloons=[],
    targets=[],
    image_size=(1920, 1080),
)

for match in matches:
    print(match.to_dict())