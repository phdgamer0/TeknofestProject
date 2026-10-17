#include "scenario_common.hpp"

// =============================================================================
// Scenario 30: High-G Vertical Pull-Up Extrapolation
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      double z = 1.0 + 2.0 * i + 0.5 * 4.0 * i * i;
      Object enemy(vec(8.0, 0.0, z), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 50;
      enemy.isAlive = true;
      f->detectedObjects[50] = enemy;
      rig.sys.tick(0.1f);
   }
   const auto& tracked = rig.sys.getTrackedObjects().at(50);
   assert(tracked.vel.z > 0.5 && "Vertical velocity must be estimated positive");
   vec futurePos = rig.brain.PredictPos(tracked, 0.5);
   assert(futurePos.z > tracked.pos.z && "Predicted future Z must be higher than current Z");
}

int main() {
   return run_scenario("30", "High-G Vertical Pull-Up Extrapolation", test);
}
