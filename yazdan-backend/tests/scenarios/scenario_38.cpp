#include "scenario_common.hpp"

// =============================================================================
// Scenario 38: Extractor Binary Packet Deserialization
// =============================================================================
static constexpr char TEST_PIPE_FILE_38[] = "./yazdan-backend/build/test_packet_38.bin";

static void test() {
   Object testObj(vec(10.0, 0.0, 2.0), vec(1.0, 0.0, 0.0), vec(0, 0, 0), false);
   testObj.ID = 88;
   testObj.isAlive = true;
   {
      std::ofstream out(TEST_PIPE_FILE_38, std::ios::out | std::ios::binary | std::ios::trunc);
      out.write(reinterpret_cast<const char*>(&testObj), sizeof(Object));
   }
   Extractor<TEST_PIPE_FILE_38, Object> extractor;
   auto extracted = extractor.extract();
   assert(extracted.has_value() && "Extractor should successfully read binary packet");
   assert(extracted->ID == 88 && "Extracted object ID must match");
   assert(std::abs(extracted->pos.x - 10.0) < 1e-4 && "Cartesian X conversion must be correct");
}

int main() {
   return run_scenario("38", "Extractor Binary Packet Deserialization", test);
}
