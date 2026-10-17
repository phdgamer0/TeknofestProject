#include "scenario_common.hpp"

// =============================================================================
// Scenario 29: Dense Friendly Balloon Wall Inhibit
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> objects;
   for (int i = 0; i < 5; ++i) {
      double y = -1.0 + i * 0.5;
      Object friendly(vec(5.0, y, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = static_cast<uint8_t>(10 + i);
      friendly.isAlive = true;
      objects[friendly.ID] = friendly;
   }
   vec enemyBehind(10.0, 0.0, 0.0);
   bool safeBehind = rig.brain.isShotSafe(enemyBehind, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(!safeBehind && "Shooting through dense friendly wall must be blocked");

   vec enemyClear(10.0, 0.0, 5.0);
   bool safeClear = rig.brain.isShotSafe(enemyClear, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(safeClear && "Shooting clear above friendly wall must be safe");
}

int main() {
   return run_scenario("29", "Dense Friendly Balloon Wall Inhibit", test);
}
