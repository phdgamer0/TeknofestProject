#include "scenario_common.hpp"

// =============================================================================
// Scenario 39: Extractor Empty Stream EOF Handling
static constexpr char TEST_PIPE_FILE_39[] = "./yazdan-backend/build/test_packet_39.bin";

static void test() {
   {
      std::ofstream out(TEST_PIPE_FILE_39, std::ios::out | std::ios::binary | std::ios::trunc);
   }
   Extractor<TEST_PIPE_FILE_39, Object> extractor;
   auto res = extractor.extract();
   assert(!res.has_value() && "Extractor must return nullopt on empty pipe");
   Object dummy;
   bool ok = extractor.extract(dummy);
   assert(!ok && "Extractor must return false when extracting into reference from empty stream");
}

int main() {
   return run_scenario("39", "Extractor Empty Stream EOF Handling", test);
}
