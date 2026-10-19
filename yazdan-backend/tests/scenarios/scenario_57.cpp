#include "scenario_common.hpp"

// =============================================================================
// Scenario 57: Behind-Shooter Friendly Balloon Laser Safety
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> objects;

   // Friendly balloon positioned behind the laser aperture (t < 0)
   Object friendlyBehind(vec(-0.5, 0.2, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendlyBehind.ID = 150;
   friendlyBehind.isAlive = true;
   objects[150] = friendlyBehind;

   // Enemy balloon directly ahead in the forward hemisphere
   vec enemyPos(10.0, 0.0, 0.0);
   Object enemy(enemyPos, vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 151;
   enemy.isAlive = true;
   objects[151] = enemy;

   bool isSafe = rig.brain.isShotSafe(enemyPos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(isSafe && "Friendly balloon strictly behind the shooter must NEVER inhibit forward laser fire");

   // Verify dynamic execution through System tick
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      f->detectedObjects[150] = friendlyBehind;
      f->detectedObjects[151] = enemy;
      rig.sys.tick(0.1f);
   }

   assert(rig.laser.isOn && "Laser must engage forward enemy despite rear-hemisphere friendly balloon");
}

int main() {
   return run_scenario("57", "Behind-Shooter Friendly Balloon Laser Safety", test);
}
