#include "scenario_common.hpp"

// =============================================================================
// Scenario 02: Fast Transverse Target Lead Aim
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, -4.0 + i * 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 2;
      enemy.isAlive = true;
      f->detectedObjects[2] = enemy;
      rig.sys.tick(0.1f);
   }
   vec intercept = rig.brain.CalculateInterceptionPoint(rig.sys.getTrackedObjects().at(2), rig.cam.CamCenterPos);
   assert(intercept.y > rig.sys.getTrackedObjects().at(2).pos.y && "Lead interception point must be ahead of target position");
}

int main() {
   return run_scenario("02", "Fast Transverse Target Lead Aim", test);
}
