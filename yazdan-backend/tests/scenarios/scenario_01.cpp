#include "scenario_common.hpp"

// =============================================================================
// Scenario 01: Stationary Enemy Destruction
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 1;
      enemy.isAlive = true;
      f->detectedObjects[1] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser should engage on aligned stationary enemy");
}

int main() {
   return run_scenario("01", "Stationary Enemy Destruction", test);
}
