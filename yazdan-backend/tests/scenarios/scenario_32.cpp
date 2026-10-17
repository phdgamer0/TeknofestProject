#include "scenario_common.hpp"

// =============================================================================
// Scenario 32: Negative Pitch / Depression Tracking
// =============================================================================
static void test() {
   TestRig rig;
   vec origin(0, 0, 0);
   vec lowTarget(10.0, 0.0, -4.0);
   double pitch = origin.PitchTo(lowTarget);
   assert(pitch < -15.0 && pitch > -90.0 && "Target below horizon must produce negative pitch angle");

   ImageFrame* f = rig.sys.pushFrame(origin, 1);
   Object enemy(lowTarget, vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 65;
   enemy.isAlive = true;
   f->detectedObjects[65] = enemy;
   rig.sys.tick(0.1f);
   assert(rig.pitch.velocity.y < 0.0 && "Engine pitch velocity must drive downwards for depressed target");
}

int main() {
   return run_scenario("32", "Negative Pitch / Depression Tracking", test);
}
