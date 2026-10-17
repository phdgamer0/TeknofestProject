#include "scenario_common.hpp"

// =============================================================================
// Scenario 52: Ghost Target Aging & Expiration
// =============================================================================
static void test() {
   TestRig rig;
   ImageFrame* f0 = rig.sys.pushFrame(vec(0, 0, 0), 0);
   Object enemy(vec(7.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 135;
   enemy.isAlive = true;
   f0->detectedObjects[135] = enemy;
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().count(135) == 1 && "Target 135 must be tracked");

   for (int i = 1; i <= 4; ++i) {
      rig.sys.pushFrame(vec(0, 0, 0), i);
      rig.sys.tick(0.1f);
      assert(rig.sys.getTrackedObjects().count(135) == 1 && "Target must persist during brief drop");
   }

   rig.sys.pushFrame(vec(0, 0, 0), 5);
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().count(135) == 0 && "Ghost target must be pruned after MAX_STALE_FRAMES");
}

int main() {
   return run_scenario("52", "Ghost Target Aging & Expiration", test);
}
