#include "scenario_common.hpp"
#include <cmath>

// =============================================================================
// Scenario 42: Orthogonal Velocity Snap Stability
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 3; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(5.0 + i * 1.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 95;
      enemy.isAlive = true;
      f->detectedObjects[95] = enemy;
      rig.sys.tick(0.1f);
   }
   for (int i = 3; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(7.0, (i - 2) * 1.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 95;
      enemy.isAlive = true;
      f->detectedObjects[95] = enemy;
      rig.sys.tick(0.1f);
   }
   const auto& obj = rig.sys.getTrackedObjects().at(95);
   assert(std::isfinite(obj.vel.x) && std::isfinite(obj.vel.y) && "Velocities must stay finite during sharp 90-deg turn");
   assert(std::abs(obj.vel.x) < 50.0 && std::abs(obj.vel.y) < 50.0 && "Smoothing must prevent runaway filter divergence");
}

int main() {
   return run_scenario("42", "Orthogonal Velocity Snap Stability", test);
}
