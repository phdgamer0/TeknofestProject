#include "scenario_common.hpp"

// =============================================================================
// Scenario 19: Intermittent Occlusion Handling
// =============================================================================
static void test() {
   TestRig rig;
   constexpr uint8_t id = 30;

   // Visible 3 frames
   for (int i = 0; i < 3; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      f->detectedObjects[id] = Object(vec(10.0, i * 1.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      f->detectedObjects[id].ID = id;
      rig.sys.tick(0.1f);
   }

   // Occluded 2 frames (empty)
   for (int i = 3; i < 5; ++i) {
      rig.sys.pushFrame(vec(0, 0, 0), i);
      rig.sys.tick(0.1f);
   }

   // Reappears on frame 5
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 5);
   f->detectedObjects[id] = Object(vec(10.0, 5.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   f->detectedObjects[id].ID = id;
   rig.sys.tick(0.1f);

   assert(rig.sys.getTrackedObjects().count(id) && "Target track should persist across intermittent occlusion");
}

int main() {
   return run_scenario("19", "Intermittent Occlusion Handling", test);
}
