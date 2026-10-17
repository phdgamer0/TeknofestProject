#include "scenario_common.hpp"

// =============================================================================
// Scenario 26: Laser Parallax Offset Compensation
// =============================================================================
static void test() {
   TestRig rig;
   vec lateralTarget(10.0, 5.0, 0.0);
   vec aimOrigin = rig.cam.CamCenterPos + CAMERA_LASER_OFFSET;
   double camYaw = rig.cam.CamCenterPos.YawTo(lateralTarget);
   double offsetYaw = aimOrigin.YawTo(lateralTarget);
   // CAMERA_LASER_OFFSET has x = 0.1, so distance along X is 9.9m instead of 10.0m
   assert(offsetYaw > camYaw && "Parallax offset must alter the computed aiming angle to target");
}

int main() {
   return run_scenario("26", "Laser Parallax Offset Compensation", test);
}
