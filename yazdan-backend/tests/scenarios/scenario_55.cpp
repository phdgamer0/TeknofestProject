#include "scenario_common.hpp"

// =============================================================================
// Scenario 55: Turret Yaw Wrap & Pitch Soft Limit Containment
// =============================================================================
static void test() {
   TestRig rig;
   rig.sys.yawAngle = 179.0;
   rig.sys.pitchAngle = 80.0;

   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(1.0, 10.0, 20.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 150;
   enemy.isAlive = true;
   f->detectedObjects[150] = enemy;

   for (int i = 0; i < 20; ++i) {
      rig.sys.tick(0.1f);
      assert(rig.sys.yawAngle >= -180.0 && rig.sys.yawAngle <= 180.0 && "Yaw angle must remain bounded in [-180, 180]");
      assert(rig.sys.pitchAngle >= MIN_PITCH_DEG && rig.sys.pitchAngle <= MAX_PITCH_DEG && "Pitch angle must not exceed soft limits [-45, +85]");
   }
}

int main() {
   return run_scenario("55", "Turret Yaw Wrap & Pitch Soft Limit Containment", test);
}
