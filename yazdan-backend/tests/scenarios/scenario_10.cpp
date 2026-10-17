#include "scenario_common.hpp"

// =============================================================================
// Scenario 10: Dead Target Exclusion
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   Object deadEnemy(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   deadEnemy.ID = 16;
   deadEnemy.isAlive = false;
   targets[16] = deadEnemy;

   Target best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(best == nullptr && "pickBestTarget must ignore destroyed targets (isAlive=false)");
}

int main() {
   return run_scenario("10", "Dead Target Exclusion", test);
}
