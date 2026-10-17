#include "scenario_common.hpp"

// =============================================================================
// Scenario 34: Receding Radial Velocity Threat Penalty
// =============================================================================
static void test() {
   TestRig rig;
   vec origin(0, 0, 0);
   Object closingEnemy(vec(12.0, 0.0, 0.0), vec(-10.0, 0.0, 0.0), vec(0, 0, 0), false);
   closingEnemy.ID = 75;
   closingEnemy.isAlive = true;

   Object recedingEnemy(vec(12.0, 0.0, 0.0), vec(10.0, 0.0, 0.0), vec(0, 0, 0), false);
   recedingEnemy.ID = 76;
   recedingEnemy.isAlive = true;

   double scoreClosing = rig.brain.EvaluateThreatLevel(closingEnemy, origin);
   double scoreReceding = rig.brain.EvaluateThreatLevel(recedingEnemy, origin);
   assert(scoreClosing > scoreReceding && "Closing target must have substantially higher threat score than receding target");
}

int main() {
   return run_scenario("34", "Receding Radial Velocity Threat Penalty", test);
}
