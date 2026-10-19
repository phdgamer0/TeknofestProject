#include "../../include/extractor.hpp"
#include "scenario_common.hpp"

// =============================================================================
// Scenario 63: Extractor Instance Stream State Isolation
// =============================================================================
static constexpr char TEST_PIPE_63_A[] = "./yazdan-backend/build/test_packet_63_a.bin";
static constexpr char TEST_PIPE_63_B[] = "./yazdan-backend/build/test_packet_63_b.bin";

static void test() {
   // Write distinct packets to two separate pipe files
   Object objA(vec(10.0, 0.0, 0.0), vec(2.5, 1.5, 0.0), vec(0, 0, 0), false);
   objA.ID = 71;
   objA.isAlive = true;

   Object objB(vec(15.0, 0.0, 0.0), vec(0.0, -3.0, 1.0), vec(0, 0, 0), false);
   objB.ID = 72;
   objB.isAlive = true;

   {
      std::ofstream outA(TEST_PIPE_63_A, std::ios::binary | std::ios::trunc);
      outA.write(reinterpret_cast<const char*>(&objA), sizeof(Object));
   }
   {
      std::ofstream outB(TEST_PIPE_63_B, std::ios::binary | std::ios::trunc);
      outB.write(reinterpret_cast<const char*>(&objB), sizeof(Object));
   }

   // Instantiate two separate extractors
   Extractor<TEST_PIPE_63_A, Object> extA;
   Extractor<TEST_PIPE_63_B, Object> extB;

   auto resA = extA.extract();
   auto resB = extB.extract();

   assert(resA.has_value() && resB.has_value() && "Both extractors must successfully read from distinct pipes");
   assert(resA->ID == 71 && resB->ID == 72 && "Stream isolation must preserve correct object IDs");

   // Velocity components must be preserved exactly without corruption
   assert(std::abs(resA->vel.x - 2.5) < 1e-4 && "Cartesian velocity X must be preserved");
   assert(std::abs(resA->vel.y - 1.5) < 1e-4 && "Cartesian velocity Y must be preserved");
   assert(std::abs(resB->vel.y - (-3.0)) < 1e-4 && "Cartesian velocity Y on stream B must be preserved");
}

int main() {
   return run_scenario("63", "Extractor Instance Stream State Isolation", test);
}
