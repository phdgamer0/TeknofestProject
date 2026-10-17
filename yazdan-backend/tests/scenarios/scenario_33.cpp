#include "scenario_common.hpp"

// =============================================================================
// Scenario 33: Dual Yaw Engine Symmetrical Drive
// =============================================================================
static void test() {
   TestRig rig;
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(5.0, 5.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 70;
   enemy.isAlive = true;
   f->detectedObjects[70] = enemy;
   rig.sys.tick(0.1f);

   assert(rig.leftYaw.velocity.x == rig.rightYaw.velocity.x && "Both yaw engines must have symmetrical velocities");
   assert(rig.leftYaw.velocity.x != 0.0 && "Yaw velocity must be non-zero when driving to target");
}

int main() {
   return run_scenario("33", "Dual Yaw Engine Symmetrical Drive", test);
}
