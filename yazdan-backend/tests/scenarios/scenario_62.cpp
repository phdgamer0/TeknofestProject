#include "scenario_common.hpp"

// =============================================================================
// Scenario 62: Destroyed Target ID Quarantine Expiry & Recycling
// =============================================================================
static void test() {
   TestRig rig;
   constexpr uint8_t recycledID = 165;

   // 1. Engage and destroy target 165
   for (int i = 0; i < 20; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = recycledID;
      enemy.isAlive = true;
      f->detectedObjects[recycledID] = enemy;
      rig.sys.tick(0.1f);
   }

   assert(!rig.sys.getTrackedObjects().at(recycledID).isAlive && "Target 165 must be confirmed destroyed");

   // 2. Advance 35 ticks (3.5s) with no detections to expire the 3.0s quarantine
   for (int i = 20; i < 55; ++i) {
      rig.sys.pushFrame(vec(0, 0, 0), i);
      rig.sys.tick(0.1f);
   }

   // 3. New enemy appears with recycled ID 165 at a different location (10.0, 2.0, 0.0)
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 55);
   Object newEnemy(vec(10.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   newEnemy.ID = recycledID;
   newEnemy.isAlive = true;
   f->detectedObjects[recycledID] = newEnemy;
   rig.sys.tick(0.1f);

   // Target must be tracked as alive again after quarantine expiration
   assert(rig.sys.getTrackedObjects().count(recycledID) && "Recycled target ID must be admitted after quarantine");
   assert(rig.sys.getTrackedObjects().at(recycledID).isAlive && "New target reusing ID must be tracked as alive");
}

int main() {
   return run_scenario("62", "Destroyed Target ID Quarantine Expiry & Recycling", test);
}
