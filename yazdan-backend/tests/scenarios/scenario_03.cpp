#include "scenario_common.hpp"

// =============================================================================
// Scenario 03: Radial Dive Bomber Threat Scoring
// =============================================================================
static void test() {
   TestRig rig;
   Object diveBomber(vec(15.0, 0.0, 0.0), vec(-10.0, 0.0, 0.0), vec(0, 0, 0), false);
   diveBomber.ID = 3;
   diveBomber.isAlive = true;

   Object retreatingTarget(vec(10.0, 0.0, 0.0), vec(5.0, 0.0, 0.0), vec(0, 0, 0), false);
   retreatingTarget.ID = 4;
   retreatingTarget.isAlive = true;

   double scoreInbound = rig.brain.EvaluateThreatLevel(diveBomber, rig.cam.CamCenterPos);
   double scoreRetreat = rig.brain.EvaluateThreatLevel(retreatingTarget, rig.cam.CamCenterPos);

   assert(scoreInbound > scoreRetreat && "Fast inbound closing target must have higher threat than retreating target");
}

int main() {
   return run_scenario("03", "Radial Dive Bomber Threat Scoring", test);
}
