#include "scenario_common.hpp"

// =============================================================================
// Scenario 47: Cumulative Laser Dwell Time to Destruction
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 35; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 110;
      enemy.isAlive = true;
      f->detectedObjects[110] = enemy;
      rig.sys.tick(0.1f);
   }
   const auto& target = rig.sys.getTrackedObjects().at(110);
   assert(!target.isAlive && "Target must be destroyed after >= 3.0s under fire");
   assert(target.timeUnderFire >= TIME_UNDER_FIRE && "Dwell time must reach TIME_UNDER_FIRE");
}

int main() {
   return run_scenario("47", "Cumulative Laser Dwell Time to Destruction", test);
}
