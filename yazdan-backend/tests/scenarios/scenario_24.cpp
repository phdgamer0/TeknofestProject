#include "scenario_common.hpp"

// =============================================================================
// Scenario 24: FlatObjectMap Insertion, Lookup & Capacity
// =============================================================================
static void test() {
   FlatObjectMap<32> map;
   for (uint8_t i = 1; i <= 32; ++i) {
      Object o(vec(i, i, i));
      o.ID = i;
      map.insert_or_assign(i, o);
   }
   assert(map.size() == 32 && "Map should reach full capacity 32");
   assert(map.at(10).pos.x == 10.0 && "Lookup in FlatObjectMap should match stored object");

   map.erase(10);
   assert(map.size() == 31 && "Size must decrease after erase");
   assert(map.count(10) == 0 && "Erased key should not be counted");
}

int main() {
   return run_scenario("24", "FlatObjectMap Insertion, Lookup & Capacity", test);
}
