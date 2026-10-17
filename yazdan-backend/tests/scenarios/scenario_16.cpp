#include "scenario_common.hpp"

// =============================================================================
// Scenario 16: RingBuffer Capacity Bound & Overwrite
// =============================================================================
static void test() {
   RingBuffer<sig, 4> buf;
   buf.push(sig(SIGNALS::SYSTEM_START));
   buf.push(sig(SIGNALS::CAM_START));
   buf.push(sig(SIGNALS::LASER_ON));
   buf.push(sig(SIGNALS::CAM_ON_TARGET));
   assert(buf.full() && "Buffer should be full at capacity 4");

   buf.push(sig(SIGNALS::SYSTEM_STOP)); // Overwrites oldest (SYSTEM_START)
   assert(buf.size() == 4 && "Size must remain bounded to capacity");
   sig val;
   buf.pop(val);
   assert(val.signal == SIGNALS::CAM_START && "Oldest element should be CAM_START after SYSTEM_START was overwritten");
}

int main() {
   return run_scenario("16", "RingBuffer Capacity Bound & Overwrite", test);
}
