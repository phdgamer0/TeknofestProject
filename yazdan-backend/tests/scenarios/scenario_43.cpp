#include "scenario_common.hpp"

// =============================================================================
// Scenario 43: Signal Storm Strict Priority Dequeue
// =============================================================================
static void test() {
   CommunicatorDevice comm;
   comm.Receive(sig(SIGNALS::OBJECT_DETECTED, std::monostate{}, Priority::NORMAL));
   comm.Receive(sig(SIGNALS::CAM_ON_TARGET, std::monostate{}, Priority::IMPORTANT));
   comm.Receive(sig(SIGNALS::SYSTEM_SHUTDOWN, std::monostate{}, Priority::CRITICAL));

   auto first = comm.popSignal();
   assert(first.has_value() && first->priorityLevel == Priority::CRITICAL && "Critical must pop first");
   auto second = comm.popSignal();
   assert(second.has_value() && second->priorityLevel == Priority::IMPORTANT && "Important must pop second");
   auto third = comm.popSignal();
   assert(third.has_value() && third->priorityLevel == Priority::NORMAL && "Normal must pop third");
}

int main() {
   return run_scenario("43", "Signal Storm Strict Priority Dequeue", test);
}
