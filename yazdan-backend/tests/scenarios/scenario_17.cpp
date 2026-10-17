#include "scenario_common.hpp"

// =============================================================================
// Scenario 17: History Frame Ring Buffer Recycling
// =============================================================================
static void test() {
   TestRig rig;
   for (uint64_t t = 0; t < 20; ++t) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), t);
      assert(f != nullptr && "pushFrame must always return valid memory from pool");
   }
   ImageFrame* const* history = rig.sys.getHistoryFrames();
   assert(history[HISTORY_FRAME_COUNTS - 1]->timestamp == 19 && "Most recent frame timestamp must match last push");
}

int main() {
   return run_scenario("17", "History Frame Ring Buffer Recycling", test);
}
