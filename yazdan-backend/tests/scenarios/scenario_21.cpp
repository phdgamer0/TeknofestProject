#include "scenario_common.hpp"

// =============================================================================
// Scenario 21: Zenith Target Pitch Calculation
// =============================================================================
static void test() {
   vec turret(0, 0, 0);
   vec overheadTarget(1.0, 0.0, 10.0); // Directly high above
   double pitch = turret.PitchTo(overheadTarget);
   assert(pitch > 80.0 && pitch <= 90.0 && "Overhead target should produce steep positive pitch angle");
}

int main() {
   return run_scenario("21", "Zenith Target Pitch Calculation", test);
}
