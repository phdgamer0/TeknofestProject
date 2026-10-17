#include "scenario_common.hpp"

// =============================================================================
// Scenario 53: Backstop Friendly Fire Ray Protection
// =============================================================================
static void test() {
   TestRig rig;
   vec enemyPos(5.0, 0.0, 0.0);
   std::unordered_map<uint8_t, Object> objects;
   Object friendlyBehind(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendlyBehind.ID = 140;
   friendlyBehind.isAlive = true;
   objects[140] = friendlyBehind;

   bool safeBehind = rig.brain.isShotSafe(enemyPos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(!safeBehind && "Shooting at target with friendly directly in backstop must be rejected");

   objects[140].pos = vec(10.0, 3.0, 0.0);
   bool safeClear = rig.brain.isShotSafe(enemyPos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(safeClear && "Shooting must be permitted when backstop is clear");
}

int main() {
   return run_scenario("53", "Backstop Friendly Fire Ray Protection", test);
}
