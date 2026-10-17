#include "scenario_common.hpp"

// =============================================================================
// Scenario 23: Cylindrical to Cartesian Roundtrip
// =============================================================================
static void test() {
   vec origCyl(10.0, 0.785398, 5.0); // r = 10, theta = 45 deg, z = 5
   vec cart;
   cart.CylindricalToCartesian(origCyl);

   vec recoveredCyl;
   recoveredCyl.CartesianToCylindrical(cart);

   assert(std::abs(recoveredCyl.x - origCyl.x) < 1e-5 && "Radius should match after roundtrip");
   assert(std::abs(recoveredCyl.y - origCyl.y) < 1e-5 && "Angle should match after roundtrip");
   assert(std::abs(recoveredCyl.z - origCyl.z) < 1e-5 && "Z should match after roundtrip");
}

int main() {
   return run_scenario("23", "Cylindrical to Cartesian Roundtrip", test);
}
