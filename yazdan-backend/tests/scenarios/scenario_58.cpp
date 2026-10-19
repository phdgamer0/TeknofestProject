#include "scenario_common.hpp"

// =============================================================================
// Scenario 58: Interception Point Safety Deadlock Resolution
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> objects;

   // Enemy A at (8.0, 5.0, 0.0) moving at +5 m/s in +Y direction.
   // Its lead interception point is calculated as (8.0, 6.67, 0.0).
   Object enemyA(vec(8.0, 5.0, 0.0), vec(0.0, 5.0, 0.0), vec(0, 0, 0), false);
   enemyA.ID = 153;
   enemyA.isAlive = true;
   objects[153] = enemyA;

   // Friendly balloon at (4.05, 3.335, 0.0), situated directly on the line of fire
   // to Enemy A's lead interception point, but clear of Enemy A's instantaneous position!
   Object friendly(vec(4.05, 3.335, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendly.ID = 152;
   friendly.isAlive = true;
   objects[152] = friendly;

   // Enemy B at (8.0, -2.0, 0.0), stationary in a completely unobstructed sector
   Object enemyB(vec(8.0, -2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemyB.ID = 154;
   enemyB.isAlive = true;
   objects[154] = enemyB;

   // Verify safety difference: instantaneous pos is safe, but intercept point is unsafe
   vec interceptA = rig.brain.CalculateInterceptionPoint(enemyA, rig.cam.CamCenterPos);
   bool safeAtPos = rig.brain.isShotSafe(enemyA.pos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   bool safeAtIntercept = rig.brain.isShotSafe(interceptA, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(safeAtPos && "Enemy A instantaneous position should have clear line of fire");
   assert(!safeAtIntercept && "Enemy A lead interception point must be blocked by friendly unit");

   // Brain must select Enemy B, preventing friendly-fire targeting deadlock
   Target chosen = rig.brain.pickBestTarget(objects, rig.cam.CamCenterPos);
   assert(chosen != nullptr && "A valid target must be selected");
   assert(chosen->ID == 154 && "System must select Enemy B because Enemy A's lead interception point is obstructed");
}

int main() {
   return run_scenario("58", "Interception Point Safety Deadlock Resolution", test);
}
