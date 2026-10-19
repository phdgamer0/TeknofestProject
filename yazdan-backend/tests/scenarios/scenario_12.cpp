#include "scenario_common.hpp"

// =============================================================================
// Scenario 12: Destruction & Automatic Retargeting
// =============================================================================
static void test() {
   TestRig rig;
   constexpr uint8_t enemyA = 20;
   constexpr uint8_t enemyB = 21;

   // Frame 1-5: Enemy A and B present
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object a(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      a.ID = enemyA;
      a.isAlive = true;
      f->detectedObjects[enemyA] = a;

      Object b(vec(12.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      b.ID = enemyB;
      b.isAlive = true;
      f->detectedObjects[enemyB] = b;
      rig.sys.tick(0.1f);
   }

   // Target A destroyed after enough ticks to satisfy TIME_UNDER_FIRE
   int currentFrame = 5;
   while (rig.sys.getTrackedObjects().at(enemyA).isAlive) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), currentFrame++);
      Object a(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      a.ID = enemyA;
      a.isAlive = true;
      f->detectedObjects[enemyA] = a;

      Object b(vec(12.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      b.ID = enemyB;
      b.isAlive = true;
      f->detectedObjects[enemyB] = b;
      rig.sys.tick(0.1f);
   }

   assert(!rig.sys.getTrackedObjects().at(enemyA).isAlive && "Enemy A should be confirmed destroyed");
   // Advance 1 tick: System must switch focus to Enemy B
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), currentFrame++);
   Object b(vec(12.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   b.ID = enemyB;
   b.isAlive = true;
   f->detectedObjects[enemyB] = b;
   rig.sys.tick(0.1f);

   Target current = rig.brain.pickBestTarget(rig.sys.getTrackedObjects(), rig.cam.CamCenterPos);
   assert(current != nullptr && current->ID == enemyB && "System should immediately retarget Enemy B");
}

int main() {
   return run_scenario("12", "Destruction & Automatic Retargeting", test);
}
