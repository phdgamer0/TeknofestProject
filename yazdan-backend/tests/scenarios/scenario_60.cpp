#include "scenario_common.hpp"

// =============================================================================
// Scenario 60: Turret Orientation Aware Slew and Aim Time
// =============================================================================
static void test() {
   TestRig rig;

   vec targetAt90(0.0, 10.0, 0.0); // 90 deg azimuth
   vec targetAt0(10.0, 0.0, 0.0);   // 0 deg azimuth

   // Case 1: Turret already oriented at 90 deg yaw
   double slewTo90 = rig.brain.CalculateTimeToAim(rig.cam.CamCenterPos, targetAt90, 90.0, 0.0);
   double slewTo0 = rig.brain.CalculateTimeToAim(rig.cam.CamCenterPos, targetAt0, 90.0, 0.0);

   assert(slewTo90 < 0.01 && "Aim time to on-axis target must be practically zero");
   assert(std::abs(slewTo0 - (90.0 / MAX_VELOCUTY_X)) < 1e-3 && "Aim time to off-axis target must reflect true 90-degree motor travel");

   // Case 2: Wraparound across +/-180 deg seam
   // Turret at +175 deg, target at -175 deg -> 10 deg shortest arc
   vec targetNearNeg180(-10.0, -0.87, 0.0);
   double slewAcrossSeam = rig.brain.CalculateTimeToAim(rig.cam.CamCenterPos, targetNearNeg180, 175.0, 0.0);
   assert(slewAcrossSeam < (20.0 / MAX_VELOCUTY_X) && "Slew across seam must take the shortest arc <= 20 degrees");
}

int main() {
   return run_scenario("60", "Turret Orientation Aware Slew and Aim Time", test);
}
