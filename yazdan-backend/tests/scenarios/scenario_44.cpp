#include "scenario_common.hpp"

// =============================================================================
// Scenario 44: Emergency Stop Slew Arrest
// =============================================================================
static void test() {
   TestRig rig;
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(0.0, 10.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 98;
   enemy.isAlive = true;
   f->detectedObjects[98] = enemy;
   rig.sys.tick(0.1f);
   assert(rig.leftYaw.velocity.x > 0.0 && "Yaw motor must be slewing toward target");

   rig.sys.executeSignal(sig(SIGNALS::SYSTEM_STOP));
   assert(rig.leftYaw.velocity.x == 0.0 && "Left yaw motor velocity must be 0 after emergency stop");
   assert(rig.rightYaw.velocity.x == 0.0 && "Right yaw motor velocity must be 0 after emergency stop");
   assert(rig.pitch.velocity.y == 0.0 && "Pitch motor velocity must be 0 after emergency stop");
   assert(!rig.laser.isOn && "Laser must be OFF after emergency stop");
}

int main() {
   return run_scenario("44", "Emergency Stop Slew Arrest", test);
}
