#include "scenario_common.hpp"

// =============================================================================
// Scenario 49: Pitch Rate Limit Clamping Under Extreme Step
// =============================================================================
static void test() {
   TestRig rig;
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(0.001, 0.0, 10.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 115;
   enemy.isAlive = true;
   f->detectedObjects[115] = enemy;
   rig.sys.tick(0.001f);
   assert(std::abs(rig.pitch.velocity.y) <= MAX_VELOCUTY_Y + 1e-4 && "Pitch velocity must be clamped to MAX_VELOCUTY_Y");
}

int main() {
   return run_scenario("49", "Pitch Rate Limit Clamping Under Extreme Step", test);
}
