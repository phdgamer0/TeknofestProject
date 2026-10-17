#include "scenario_common.hpp"

// =============================================================================
// Scenario 15: Critical Signal Priority Queue Preemption
// =============================================================================
static void test() {
   CommunicatorDevice comm;
   sig normalSig(SIGNALS::CAM_START, std::monostate{}, Priority::NORMAL);
   sig criticalSig(SIGNALS::SYSTEM_STOP, std::monostate{}, Priority::CRITICAL);

   comm.Receive(normalSig);
   comm.Receive(criticalSig);

   auto firstOut = comm.popSignal();
   assert(firstOut.has_value() && firstOut->signal == SIGNALS::SYSTEM_STOP && "CRITICAL signals must preempt NORMAL signals");
}

int main() {
   return run_scenario("15", "Critical Signal Priority Queue Preemption", test);
}
