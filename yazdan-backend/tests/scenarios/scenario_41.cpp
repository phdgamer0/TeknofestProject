#include "scenario_common.hpp"

// =============================================================================
// Scenario 41: Pure Acceleration Lead Estimation (v0=0)
// =============================================================================
static void test() {
   TestRig rig;
   Object acceleratingEnemy(vec(10.0, 0.0, 0.0), vec(0.0, 0.0, 0.0), vec(0.0, 4.0, 0.0), false);
   acceleratingEnemy.ID = 92;
   acceleratingEnemy.isAlive = true;

   vec predicted = rig.brain.PredictPos(acceleratingEnemy, 1.0);
   assert(std::abs(predicted.y - 2.0) < 1e-4 && "Prediction must accurately incorporate quadratic acceleration from rest");
}

int main() {
   return run_scenario("41", "Pure Acceleration Lead Estimation (v0=0)", test);
}
