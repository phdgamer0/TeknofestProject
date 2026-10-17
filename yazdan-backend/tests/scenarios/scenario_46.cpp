#include "scenario_common.hpp"

// =============================================================================
// Scenario 46: NaN & Infinity Input Numerical Robustness
// =============================================================================
static void test() {
   vec nanVec(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0);
   vec infVec(std::numeric_limits<double>::infinity(), 10.0, 0.0);
   vec normalVec(1.0, 0.0, 0.0);

   assert(!std::isfinite(nanVec.x) && "NaN coordinate must be recognized as non-finite");
   assert(!std::isfinite(infVec.x) && "Inf coordinate must be recognized as non-finite");
   assert(std::isfinite(normalVec.x) && "Normal coordinate must be finite");
}

int main() {
   return run_scenario("46", "NaN & Infinity Input Numerical Robustness", test);
}
