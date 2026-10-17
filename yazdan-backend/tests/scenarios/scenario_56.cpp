#include "scenario_common.hpp"

// =============================================================================
// Scenario 56: Reentrant Thread-Safe Fast Logging
// =============================================================================
static void test() {
   Logger testLogger("./yazdan-backend/logs.txt");
   for (int i = 0; i < 100; ++i) {
      testLogger.log("Concurrent test line " + std::to_string(i));
      testLogger.logSignal(sig(SIGNALS::CAM_ON_TARGET));
   }
}

int main() {
   return run_scenario("56", "Reentrant Thread-Safe Fast Logging", test);
}
