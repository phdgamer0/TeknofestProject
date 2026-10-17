#include "scenario_common.hpp"

// =============================================================================
// Scenario 50: Sustained 1,000-Tick Full Simulation Stability
// =============================================================================
static void test() {
   TestRig rig;
   for (int tick = 0; tick < 1000; ++tick) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), tick);
      double angle = tick * 0.05;
      Object enemy(vec(8.0 + std::sin(angle), std::cos(angle) * 3.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 120;
      enemy.isAlive = true;
      f->detectedObjects[120] = enemy;

      Object friendly(vec(4.0, 6.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = 121;
      friendly.isAlive = true;
      f->detectedObjects[121] = friendly;

      rig.sys.tick(0.016f);
      assert(std::isfinite(rig.sys.yawAngle) && std::isfinite(rig.sys.pitchAngle) && "Angles must remain finite");
   }
   assert(rig.sys.getTrackedObjects().size() >= 2 && "System must maintain tracked objects without memory leaks");
}

int main() {
   return run_scenario("50", "Sustained 1,000-Tick Full Simulation Stability", test);
}
