#include "../include/ardunio.hpp"
#include "../include/logger.hpp"
#include "../include/logic.hpp"
#include "../include/physichs.hpp"
#include <cassert>
#include <iostream>

void test_kinematics_and_tracking() {
   std::cout << "[TEST] Running Kinematics & Targeting Simulation..." << std::endl;

   Camera cam(vec(0, 0, 0));
   Engine leftYaw, rightYaw, pitch;
   Laser laser;
   Brain brain;
   Logger logger("./yazdan-backend/logs.txt");
   System sys(cam, leftYaw, rightYaw, pitch, laser, brain, logger);

   // Simulation parameters: enemy drone flying at 10m distance
   // moving along Y axis at 2.0 m/s: (x=10, y=startY, z=2)
   constexpr float dt = 0.1f; // 100ms per frame
   constexpr uint8_t enemyId = 1;
   constexpr uint8_t friendlyId = 2;

   double startY = -2.0;
   double velocityY = 2.0; // 2 m/s

   // Step 1: Feed 5 history frames to initialize kinematics filter
   for (uint8_t i = 0; i < 5; ++i) {
      uint64_t timestamp = i;
      ImageFrame* frame = sys.pushFrame(vec(0, 0, 0), timestamp);

      // Add Enemy
      Object enemy(vec(10.0, startY + i * velocityY * dt, 2.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = enemyId;
      enemy.isAlive = true;
      enemy.isFriendly = false;
      frame->detectedObjects[enemyId] = enemy;

      // Add Friendly unit nearby
      Object friendly(vec(5.0, 5.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = friendlyId;
      friendly.isAlive = true;
      friendly.isFriendly = true;
      frame->detectedObjects[friendlyId] = friendly;

      sys.tick(dt);
   }

   // Verify tracked objects are populated
   const auto& tracked = sys.getTrackedObjects();
   assert(tracked.size() >= 2 && "Tracked objects should contain enemy and friendly");
   assert(tracked.at(enemyId).vel.y > 0.0 && "Velocity in Y should be estimated positive");
   std::cout << "  -> Kinematics velocity estimated: Vy = " << tracked.at(enemyId).vel.y << " m/s" << std::endl;

   // Step 2: Run simulation loop until enemy is destroyed
   std::cout << "  -> Tracking target and engaging turret..." << std::endl;
   int ticks = 0;
   bool destroyed = false;

   while (ticks < 60) { // simulate up to 6 seconds
      uint64_t timestamp = 5 + ticks;
      ImageFrame* frame = sys.pushFrame(vec(0, 0, 0), timestamp);

      double enemyY = startY + (5 + ticks) * velocityY * dt;
      Object enemy(vec(10.0, enemyY, 2.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = enemyId;
      enemy.isAlive = !destroyed;
      enemy.isFriendly = false;
      frame->detectedObjects[enemyId] = enemy;

      sys.tick(dt);

      if (laser.isOn) {
         std::cout << "  -> [Tick " << ticks << "] Laser ENGAGED at Yaw: " << sys.yawAngle << " deg, Pitch: " << sys.pitchAngle << " deg" << std::endl;
      }

      const auto& currentTracked = sys.getTrackedObjects();
      if (currentTracked.count(enemyId) && !currentTracked.at(enemyId).isAlive) {
         destroyed = true;
         std::cout << "  -> Target successfully destroyed after " << ticks * dt << " seconds!" << std::endl;
         break;
      }

      ticks++;
   }

   assert(destroyed && "Target should have been sustained under fire and destroyed!");
   std::cout << "[PASS] Kinematics & Targeting Simulation passed successfully!\n"
             << std::endl;
}

void test_friendly_fire_safety() {
   std::cout << "[TEST] Running Friendly Fire Safety Simulation..." << std::endl;

   Camera cam(vec(0, 0, 0));
   Engine leftYaw, rightYaw, pitch;
   Laser laser;
   Brain brain;
   Logger logger("./yazdan-backend/logs.txt");
   System sys(cam, leftYaw, rightYaw, pitch, laser, brain, logger);

   constexpr float dt = 0.1f;
   constexpr uint8_t enemyId = 10;
   constexpr uint8_t friendlyId = 20;

   // Position enemy directly behind friendly unit:
   // Turret at (0,0,0) -> Friendly at (5, 0, 0) -> Enemy at (10, 0, 0)
   for (uint8_t i = 0; i < 5; ++i) {
      ImageFrame* frame = sys.pushFrame(vec(0, 0, 0), i * 100);

      Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = friendlyId;
      friendly.isAlive = true;
      frame->detectedObjects[friendlyId] = friendly;

      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = enemyId;
      enemy.isAlive = true;
      frame->detectedObjects[enemyId] = enemy;

      sys.tick(dt);
   }

   // Because friendly is directly in line of fire, laser must NOT engage
   assert(!laser.isOn && "Laser must NOT fire when a friendly is in the line of fire!");
   std::cout << "  -> Laser blocked safely due to friendly in line of fire." << std::endl;
   std::cout << "[PASS] Friendly Fire Safety Test passed successfully!\n"
             << std::endl;
}

int main() {
   std::cout << "========================================" << std::endl;
   std::cout << "   RUNNING YAZDAN BACKEND VIRTUAL SIM   " << std::endl;
   std::cout << "========================================" << std::endl;

   test_kinematics_and_tracking();
   test_friendly_fire_safety();

   std::cout << "ALL VIRTUAL SIMULATION TESTS PASSED!" << std::endl;
   return 0;
}
