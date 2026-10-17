#include "scenario_common.hpp"

// =============================================================================
// Scenario 06: Friendly Unit Line of Fire Safety
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = 10;
      friendly.isAlive = true;
      f->detectedObjects[10] = friendly;

      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 11;
      enemy.isAlive = true;
      f->detectedObjects[11] = enemy;

      rig.sys.tick(0.1f);
   }
   assert(!rig.laser.isOn && "Laser must NEVER fire through a friendly asset");
}

int main() {
   return run_scenario("06", "Friendly Unit Line of Fire Safety", test);
}
