#include "scenario_common.hpp"

// =============================================================================
// Scenario 27: Exact Perimeter Boundary (20m Limit)
// =============================================================================
static void test() {
   TestRig rig;
   ImageFrame* f1 = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object inside(vec(19.9, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   inside.ID = 101;
   inside.isAlive = true;
   f1->detectedObjects[101] = inside;
   rig.sys.tick(0.1f);
   assert(rig.laser.isOn && "Target inside MAX_DISTANCE (19.9m) must be engaged");

   rig.sys.reset();
   ImageFrame* f2 = rig.sys.pushFrame(vec(0, 0, 0), 2);
   Object outside(vec(20.1, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   outside.ID = 102;
   outside.isAlive = true;
   f2->detectedObjects[102] = outside;
   rig.sys.tick(0.1f);
   assert(!rig.laser.isOn && "Target beyond MAX_DISTANCE (20.1m) must not be engaged");
}

int main() {
   return run_scenario("27", "Exact Perimeter Boundary (20m Limit)", test);
}
