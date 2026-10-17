#include "scenario_common.hpp"

// =============================================================================
// Scenario 13: Emergency Stop System Freeze
// =============================================================================
static void test() {
   TestRig rig;
   rig.laser.turnOn();
   rig.leftYaw.move(vec(10, 0, 0));
   rig.pitch.move(vec(0, 10, 0));

   rig.sys.executeSignal(sig(SIGNALS::SYSTEM_STOP));
   assert(!rig.laser.isOn && "E-Stop must cut laser power");
   assert(!rig.leftYaw.isActive && !rig.pitch.isActive && "E-Stop must halt all motor motion");
}

int main() {
   return run_scenario("13", "Emergency Stop System Freeze", test);
}
