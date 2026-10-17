#include "scenario_common.hpp"

// =============================================================================
// Scenario 14: System Restart & State Clearance
// =============================================================================
static void test() {
   TestRig rig;
   rig.sys.pushFrame(vec(1, 2, 3), 100);
   rig.sys.yawAngle = 45.0;
   rig.sys.pitchAngle = 20.0;

   rig.sys.executeSignal(sig(SIGNALS::SYSTEM_RESTART));
   assert(rig.sys.getTrackedObjects().empty() && "Restart must clear tracked object registry");
   assert(rig.cam.CamCenterPos.magnitude() == 0.0 && "Restart must zero camera position");
}

int main() {
   return run_scenario("14", "System Restart & State Clearance", test);
}
