#include "scenario_common.hpp"

// =============================================================================
// Scenario 07: Multi-Enemy Threat Prioritization
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   // Slow distant enemy
   Object distant(vec(18.0, 0.0, 0.0), vec(0.0, 0.0, 0.0), vec(0, 0, 0), false);
   distant.ID = 12;
   distant.isAlive = true;
   targets[12] = distant;

   // Fast close enemy
   Object close(vec(6.0, 0.0, 0.0), vec(-5.0, 0.0, 0.0), vec(0, 0, 0), false);
   close.ID = 13;
   close.isAlive = true;
   targets[13] = close;

   Target best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(best != nullptr && best->ID == 13 && "Closer aggressive enemy must be chosen as top priority");
}

int main() {
   return run_scenario("07", "Multi-Enemy Threat Prioritization", test);
}
