#include "scenario_common.hpp"

// =============================================================================
// Scenario 05: Savitzky-Golay Smoothing Stability
// =============================================================================
static void test() {
   TestRig rig;
   double noisyY[5] = {-0.5, 0.6, -0.4, 0.5, -0.3};
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, noisyY[i], 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 6;
      enemy.isAlive = true;
      f->detectedObjects[6] = enemy;
      rig.sys.tick(0.1f);
   }
   double smoothedY = rig.sys.getTrackedObjects().at(6).pos.y;
   assert(std::abs(smoothedY) < 1.0 && "Smoothed coordinate should remain stable and bounded");
}

int main() {
   return run_scenario("05", "Savitzky-Golay Smoothing Stability", test);
}
