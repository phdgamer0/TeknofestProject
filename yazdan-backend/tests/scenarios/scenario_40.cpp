#include "scenario_common.hpp"

// =============================================================================
// Scenario 40: Non-Uniform Timestamp Kinematic Jitter
// =============================================================================
static void test() {
   TestRig rig;
   uint64_t times[5] = {0, 16, 49, 99, 115};
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), times[i]);
      Object enemy(vec(10.0, i * 0.3, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 90;
      enemy.isAlive = true;
      f->detectedObjects[90] = enemy;
      rig.sys.tick(0.016f);
   }
   const auto& obj = rig.sys.getTrackedObjects().at(90);
   assert(std::isfinite(obj.vel.y) && "Estimated velocity must remain finite with jittered timestamps");
   assert(std::isfinite(obj.acc.y) && "Estimated acceleration must remain finite with jittered timestamps");
}

int main() {
   return run_scenario("40", "Non-Uniform Timestamp Kinematic Jitter", test);
}
