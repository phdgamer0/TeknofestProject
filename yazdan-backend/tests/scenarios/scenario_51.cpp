#include "scenario_common.hpp"

// =============================================================================
// Scenario 51: Zombie Target Resurrection Prevention
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 35; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 130;
      enemy.isAlive = true;
      f->detectedObjects[130] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(!rig.sys.getTrackedObjects().at(130).isAlive && "Target 130 must be marked dead");

   for (int i = 35; i < 45; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemyDebris(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemyDebris.ID = 130;
      enemyDebris.isAlive = true;
      f->detectedObjects[130] = enemyDebris;
      rig.sys.tick(0.1f);
   }
   assert(!rig.sys.getTrackedObjects().at(130).isAlive && "Target 130 must NOT be resurrected by debris detections");
   assert(!rig.laser.isOn && "Laser must not fire on confirmed destroyed target");
}

int main() {
   return run_scenario("51", "Zombie Target Resurrection Prevention", test);
}
