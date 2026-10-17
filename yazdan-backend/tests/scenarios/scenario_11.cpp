#include "scenario_common.hpp"

// =============================================================================
// Scenario 11: Out-of-Range Perimeter Rejection
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   Object farEnemy(vec(MAX_DISTANCE + 5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   farEnemy.ID = 17;
   farEnemy.isAlive = true;
   targets[17] = farEnemy;

   double score = rig.brain.EvaluateThreatLevel(farEnemy, rig.cam.CamCenterPos);
   assert(score == 0.0 && "Threat score must be 0 for objects outside MAX_DISTANCE");
}

int main() {
   return run_scenario("11", "Out-of-Range Perimeter Rejection", test);
}
