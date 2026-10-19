#include "scenario_common.hpp"
#include <cmath>

// =============================================================================
// Scenario 54: Schmitt-Trigger Laser Hysteresis Window
// =============================================================================
static void test() {
   TestRig rig;
   double y14 = 9.9 * std::tan(1.4 * M_PI / 180.0);
   ImageFrame* f1 = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy14(vec(10.0, y14, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy14.ID = 145;
   enemy14.isAlive = true;
   f1->detectedObjects[145] = enemy14;
   rig.sys.tick(0.0001f);
   assert(!rig.laser.isOn && "Laser must NOT engage at 1.4 deg (> AIM_ENGAGE_TOLERANCE_DEG 1.2)");

   double y05 = 9.9 * std::tan(0.5 * M_PI / 180.0);
   ImageFrame* f2 = rig.sys.pushFrame(vec(0, 0, 0), 2);
   Object enemy05(vec(10.0, y05, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy05.ID = 145;
   enemy05.isAlive = true;
   f2->detectedObjects[145] = enemy05;
   rig.sys.tick(0.0001f);
   assert(rig.laser.isOn && "Laser must engage at 0.5 deg (<= AIM_ENGAGE_TOLERANCE_DEG 1.2)");

   ImageFrame* f3 = rig.sys.pushFrame(vec(0, 0, 0), 3);
   f3->detectedObjects[145] = enemy14;
   rig.sys.tick(0.0001f);
   assert(rig.laser.isOn && "Laser must STAY ON within hysteresis window (1.4 <= AIM_HOLD_TOLERANCE_DEG 1.8)");

   double y40 = 9.9 * std::tan(4.0 * M_PI / 180.0);
   ImageFrame* f4 = rig.sys.pushFrame(vec(0, 0, 0), 4);
   Object enemy40(vec(10.0, y40, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy40.ID = 145;
   enemy40.isAlive = true;
   f4->detectedObjects[145] = enemy40;
   rig.sys.tick(0.0001f);
   assert(!rig.laser.isOn && "Laser must turn OFF when error exceeds AIM_HOLD_TOLERANCE_DEG (4.0 > 1.8)");
}

int main() {
   return run_scenario("54", "Schmitt-Trigger Laser Hysteresis Window", test);
}
