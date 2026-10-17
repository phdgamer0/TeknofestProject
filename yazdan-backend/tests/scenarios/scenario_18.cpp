#include "scenario_common.hpp"

// =============================================================================
// Scenario 18: Zero-Detection Frame Stability
// =============================================================================
static void test() {
   TestRig rig;
   // Empty frame: sky clear
   rig.sys.pushFrame(vec(0, 0, 0), 1);
   rig.sys.tick(0.033f);
   assert(!rig.laser.isOn && "Laser must stay off during zero detections");
}

int main() {
   return run_scenario("18", "Zero-Detection Frame Stability", test);
}
