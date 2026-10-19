#include "scenario_common.hpp"
#include <cmath>

// =============================================================================
// Scenario 31: Continuous 360-Degree Azimuth Sweep
// =============================================================================
static void test() {
   TestRig rig;
   constexpr int STEPS = 36;
   for (int i = 0; i < STEPS; ++i) {
      double angleRad = (i * 10.0) * M_PI / 180.0;
      double x = 8.0 * std::cos(angleRad);
      double y = 8.0 * std::sin(angleRad);
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(x, y, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 60;
      enemy.isAlive = true;
      f->detectedObjects[60] = enemy;
      rig.sys.tick(0.1f);
      assert(std::isfinite(rig.sys.yawAngle) && "Yaw angle must remain finite throughout sweep");
   }
}

int main() {
   return run_scenario("31", "Continuous 360-Degree Azimuth Sweep", test);
}
