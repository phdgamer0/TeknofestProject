#include "scenario_common.hpp"

// =============================================================================
// Scenario 28: Near-Field Target & Zero-Division Safety
// =============================================================================
static void test() {
   TestRig rig;
   vec nearPos(0.05, 0.05, 0.05);
   vec origin(0, 0, 0);
   double dist = origin.distanceTo(nearPos);
   double yaw = origin.YawTo(nearPos);
   double pitch = origin.PitchTo(nearPos);
   assert(std::isfinite(dist) && dist > 0.0 && "Distance must be finite");
   assert(std::isfinite(yaw) && "Yaw must be finite");
   assert(std::isfinite(pitch) && "Pitch must be finite");

   double aimTime = rig.brain.CalculateTimeToAim(origin, nearPos);
   assert(std::isfinite(aimTime) && aimTime >= 0.0 && "Aim time must be finite");
}

int main() {
   return run_scenario("28", "Near-Field Target & Zero-Division Safety", test);
}
