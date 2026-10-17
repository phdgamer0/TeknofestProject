#include "scenario_common.hpp"

// =============================================================================
// Scenario 37: Dynamic Friendly Intrusion Laser Inhibit
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 85;
      enemy.isAlive = true;
      f->detectedObjects[85] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser should be firing at unhindered enemy");

   ImageFrame* fIntrusion = rig.sys.pushFrame(vec(0, 0, 0), 5);
   Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 85;
   enemy.isAlive = true;
   fIntrusion->detectedObjects[85] = enemy;

   Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendly.ID = 86;
   friendly.isAlive = true;
   fIntrusion->detectedObjects[86] = friendly;

   rig.sys.tick(0.1f);
   assert(!rig.laser.isOn && "Laser must immediately turn OFF when friendly intrudes into line of fire");
}

int main() {
   return run_scenario("37", "Dynamic Friendly Intrusion Laser Inhibit", test);
}
