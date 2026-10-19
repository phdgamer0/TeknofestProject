#include "scenario_common.hpp"

// =============================================================================
// Scenario 64: Zenith Gimbal Singularity Deadband Stability
// =============================================================================
static void test() {
   vec origin(0.0, 0.0, 0.0);

   // Pure zenith target directly overhead
   vec directZenith(0.0, 0.0, 10.0);
   double yawPure = origin.YawTo(directZenith);
   double pitchPure = origin.PitchTo(directZenith);

   assert(std::abs(yawPure) < 1e-4 && "Pure zenith target must produce stable 0.0 yaw without singularity jump");
   assert(std::abs(pitchPure - 90.0) < 1e-4 && "Zenith target pitch must be exactly 90.0 degrees");

   // Zenith target with sub-millimeter noise
   vec noisyZenith(1e-6, -1e-6, 12.0);
   double yawNoisy = origin.YawTo(noisyZenith);
   double pitchNoisy = origin.PitchTo(noisyZenith);

   assert(std::abs(yawNoisy) < 1e-4 && "Near-zero horizontal radius must trigger deadband clamp to 0.0 yaw");
   assert(std::abs(pitchNoisy - 90.0) < 1e-2 && "Pitch must remain within zenith limit");
}

int main() {
   return run_scenario("64", "Zenith Gimbal Singularity Deadband Stability", test);
}
