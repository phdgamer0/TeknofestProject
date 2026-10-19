#include "scenario_common.hpp"
#include <cmath>

// =============================================================================
// Scenario 61: Single-Step Slew Zero-Latency Laser Engagement
// =============================================================================
static void test() {
   TestRig rig;

   // Target at approx 4.0 degrees yaw from (0,0,0)
   double yCoord = 10.0 * std::tan(toRadians(4.0));
   Object enemy(vec(10.0, yCoord, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 160;
   enemy.isAlive = true;

   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   f->detectedObjects[160] = enemy;

   // Advance single tick of 0.1s
   // Max motor displacement is 120 deg/s * 0.1s = 12.0 deg, so 4 deg is reached in 1 step
   rig.sys.tick(0.1f);

   assert(rig.laser.isOn && "Laser must turn ON immediately on the step the turret aligns, with 0 frame latency");
}

int main() {
   return run_scenario("61", "Single-Step Slew Zero-Latency Laser Engagement", test);
}
