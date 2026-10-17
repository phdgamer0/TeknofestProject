#include "scenario_common.hpp"

// =============================================================================
// Scenario 20: 360-Degree Yaw Angle Shortest Path
// =============================================================================
static void test() {
   TestRig rig;
   rig.sys.yawAngle = 175.0; // Facing near +180 deg
   // Target at -175 deg (10 degrees away across 180 boundary)
   vec targetPos(-10.0, -1.0, 0.0);
   double targetYaw = rig.cam.CamCenterPos.YawTo(targetPos);

   double deltaYaw = targetYaw - rig.sys.yawAngle;
   while (deltaYaw > 180.0)
      deltaYaw -= 360.0;
   while (deltaYaw < -180.0)
      deltaYaw += 360.0;

   assert(std::abs(deltaYaw) < 30.0 && "Delta yaw must wrap to shortest turn path across +/-180 boundary");
}

int main() {
   return run_scenario("20", "360-Degree Yaw Angle Shortest Path", test);
}
