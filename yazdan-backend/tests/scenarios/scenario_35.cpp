#include "scenario_common.hpp"

// =============================================================================
// Scenario 35: 5-Target Sequential Kill Chain
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   for (uint8_t i = 1; i <= 5; ++i) {
      Object o(vec(4.0 + i * 2.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      o.ID = i;
      o.isAlive = true;
      targets[i] = o;
   }
   for (uint8_t k = 1; k <= 5; ++k) {
      obj* best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
      assert(best != nullptr && "Target must be selected");
      assert(best->ID == k && "Sequential selection must proceed from closest to farthest");
      best->isAlive = false;
   }
   obj* noneLeft = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(noneLeft == nullptr && "All targets destroyed, none should be selected");
}

int main() {
   return run_scenario("35", "5-Target Sequential Kill Chain", test);
}
