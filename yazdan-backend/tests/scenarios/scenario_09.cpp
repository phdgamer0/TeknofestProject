#include "scenario_common.hpp"

// =============================================================================
// Scenario 09: Friendly Swarm Complete Disengagement
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> friendlies;
   for (uint8_t id = 1; id <= 5; ++id) {
      Object f(vec(id * 3.0, 0, 0), vec(0, 0, 0), vec(0, 0, 0), true);
      f.ID = id;
      f.isAlive = true;
      friendlies[id] = f;
   }
   Target best = rig.brain.pickBestTarget(friendlies, rig.cam.CamCenterPos);
   assert(best == nullptr && "pickBestTarget must return nullptr for all-friendly detection field");
}

int main() {
   return run_scenario("09", "Friendly Swarm Complete Disengagement", test);
}
