#include "scenario_common.hpp"
#include <cmath>

// =============================================================================
// Scenario 48: 64-Target Swarm Ranking Invariant
// =============================================================================
static void test() {
   TestRig rig;
   std::unordered_map<uint8_t, Object> swarm;
   for (uint8_t id = 1; id <= 64; ++id) {
      double dist = 2.0 + (id * 0.25);
      double angle = (id * 5.0) * M_PI / 180.0;
      Object o(vec(dist * std::cos(angle), dist * std::sin(angle), 0.0), vec(-1.0, 0.0, 0.0), vec(0, 0, 0), (id % 5 == 0));
      o.ID = id;
      o.isAlive = true;
      if (id == 42) {
         o.isUrgent = true;
         o.isFriendly = false;
      }
      swarm[id] = o;
   }
   obj* best = rig.brain.pickBestTarget(swarm, rig.cam.CamCenterPos);
   assert(best != nullptr && "A valid target must be selected from 64 targets");
   assert(!best->isFriendly && best->isAlive && "Selected target must be alive enemy");
   assert(best->ID == 42 && "Urgent priority target must be selected ahead of others");
}

int main() {
   return run_scenario("48", "64-Target Swarm Ranking Invariant", test);
}
