#include "scenario_common.hpp"

// =============================================================================
// Scenario 22: Motor Velocity Clamping to Limits
// =============================================================================
static void test() {
   TestRig rig;
   // Create large step target (e.g. 90 degrees away)
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(0.0, 10.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 40;
   f->detectedObjects[40] = enemy;

   rig.sys.tick(0.01f); // small deltaTime forces large requested velocity
   assert(std::abs(rig.leftYaw.velocity.x) <= MAX_VELOCUTY_X + 1e-4 && "Yaw velocity must not exceed MAX_VELOCUTY_X");
}

int main() {
   return run_scenario("22", "Motor Velocity Clamping to Limits", test);
}
