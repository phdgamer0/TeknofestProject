#include "scenario_common.hpp"

// =============================================================================
// Scenario 45: Obstacle Clearance & Automatic Re-engagement
// =============================================================================
static void test() {
   TestRig rig;
   ImageFrame* f1 = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 105;
   enemy.isAlive = true;
   f1->detectedObjects[105] = enemy;

   Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendly.ID = 106;
   friendly.isAlive = true;
   f1->detectedObjects[106] = friendly;
   rig.sys.tick(0.1f);
   assert(!rig.laser.isOn && "Laser must be inhibited by blocking friendly");

   Object friendlyMoved(vec(5.0, 4.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendlyMoved.ID = 106;
   friendlyMoved.isAlive = true;

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 2 + i);
      f->detectedObjects[105] = enemy;
      f->detectedObjects[106] = friendlyMoved;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser must automatically engage once friendly has cleared the line of fire");
}

int main() {
   return run_scenario("45", "Obstacle Clearance & Automatic Re-engagement", test);
}
