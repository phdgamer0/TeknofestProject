#include "scenario_common.hpp"

// =============================================================================
// Scenario 04: Target Acceleration & Quadratic Trajectory
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      double y = 0.5 * 2.0 * i * i; // a = 2.0 m/s^2
      Object enemy(vec(10.0, y, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 5;
      enemy.isAlive = true;
      f->detectedObjects[5] = enemy;
      rig.sys.tick(0.1f);
   }
   vec future = rig.brain.PredictPos(rig.sys.getTrackedObjects().at(5), 1.0);
   assert(future.y > rig.sys.getTrackedObjects().at(5).pos.y && "Trajectory prediction must extrapolate forward motion");
}

int main() {
   return run_scenario("04", "Target Acceleration & Quadratic Trajectory", test);
}
