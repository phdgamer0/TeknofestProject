#include "scenario_common.hpp"

// =============================================================================
// Scenario 36: Intermittent Occlusion & State Continuity
// =============================================================================
static void test() {
   TestRig rig;
   for (int i = 0; i < 3; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, i * 0.5, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 80;
      enemy.isAlive = true;
      f->detectedObjects[80] = enemy;
      rig.sys.tick(0.1f);
   }
   ImageFrame* fEmpty = rig.sys.pushFrame(vec(0, 0, 0), 3);
   (void)fEmpty;
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().count(80) > 0 && "Tracked object must not vanish immediately upon 1 dropped frame");

   ImageFrame* fReappear = rig.sys.pushFrame(vec(0, 0, 0), 4);
   Object enemyRe(vec(10.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemyRe.ID = 80;
   enemyRe.isAlive = true;
   fReappear->detectedObjects[80] = enemyRe;
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().at(80).isAlive && "Target tracking must continue successfully after occlusion");
}

int main() {
   return run_scenario("36", "Intermittent Occlusion & State Continuity", test);
}
