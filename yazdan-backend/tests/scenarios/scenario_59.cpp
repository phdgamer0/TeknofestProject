#include "scenario_common.hpp"

// =============================================================================
// Scenario 59: Pure Acceleration Lead Estimation from Rest
// =============================================================================
static void test() {
   TestRig rig;

   // Target has zero velocity but significant lateral acceleration from an off-axis position
   Object acceleratingEnemy(vec(10.0, 3.0, 0.0), vec(0.0, 0.0, 0.0), vec(0.0, 6.0, 0.0), false);
   acceleratingEnemy.ID = 155;
   acceleratingEnemy.isAlive = true;

   vec intercept = rig.brain.CalculateInterceptionPoint(acceleratingEnemy, rig.cam.CamCenterPos);

   // Slew time to 3m off-axis is ~0.14s; with a = 6 m/s^2, quadratic displacement 0.5*a*t^2 must yield delta Y > 0.05m
   double leadDisplacement = intercept.y - acceleratingEnemy.pos.y;
   assert(leadDisplacement > 0.01 && "Interception solver must compute lead for accelerating target starting from rest");
   assert(std::abs(intercept.x - 10.0) < 1e-3 && "X coordinate must remain on target trajectory plane");
}

int main() {
   return run_scenario("59", "Pure Acceleration Lead Estimation from Rest", test);
}
