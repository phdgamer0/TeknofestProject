#include "scenario_common.hpp"

// =============================================================================
// Scenario 08: Urgent Flag Priority Override
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   // Closer standard enemy
   Object normal(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   normal.ID = 14;
   normal.isAlive = true;
   normal.isUrgent = false;
   targets[14] = normal;

   // Farther urgent enemy
   Object urgent(vec(14.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   urgent.ID = 15;
   urgent.isAlive = true;
   urgent.isUrgent = true;
   targets[15] = urgent;

   Target best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(best != nullptr && best->ID == 15 && "Urgent target must supersede non-urgent closer targets");
}

int main() {
   return run_scenario("08", "Urgent Flag Priority Override", test);
}
