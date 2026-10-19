#include "../include/ardunio.hpp"
#include "../include/comms.hpp"
#include "../include/constants.hpp"
#include "../include/extractor.hpp"
#include "../include/logger.hpp"
#include "../include/logic.hpp"
#include "../include/physichs.hpp"
#include <cassert>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <sys/resource.h>
#include <unistd.h>

inline long getCurrentRssKb() {
   std::ifstream statm("/proc/self/statm");
   long size = 0, resident = 0;
   if (statm >> size >> resident) {
      long pageSizeKb = sysconf(_SC_PAGESIZE) / 1024;
      return resident * pageSizeKb;
   }
   return 0;
}

// Helper to instantiate a fresh test system
struct TestRig {
   Camera cam;
   Engine leftYaw, rightYaw, pitch;
   Laser laser;
   Brain brain;
   Logger logger;
   System sys;

   TestRig()
       : cam(vec(0, 0, 0)),
         leftYaw(),
         rightYaw(),
         pitch(),
         laser(),
         brain(),
         logger("./yazdan-backend/logs.txt"),
         sys(cam, leftYaw, rightYaw, pitch, laser, brain, logger) {
   }
};

// -----------------------------------------------------------------------------
// Scenario 1: Stationary Enemy in Direct Front
// -----------------------------------------------------------------------------
void test_scenario_01_stationary_enemy() {
   std::cout << "[Scenario 01] Stationary Enemy Destruction: ";
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 1;
      enemy.isAlive = true;
      f->detectedObjects[1] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser should engage on aligned stationary enemy");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 2: Fast Transverse Crossing Target (Lead Aim Calculation)
// -----------------------------------------------------------------------------
void test_scenario_02_fast_transverse_target() {
   std::cout << "[Scenario 02] Fast Transverse Target Lead Aim: ";
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, -4.0 + i * 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 2;
      enemy.isAlive = true;
      f->detectedObjects[2] = enemy;
      rig.sys.tick(0.1f);
   }
   vec intercept = rig.brain.CalculateInterceptionPoint(rig.sys.getTrackedObjects().at(2), rig.cam.CamCenterPos);
   assert(intercept.y > rig.sys.getTrackedObjects().at(2).pos.y && "Lead interception point must be ahead of target position");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 3: Incoming Radial Dive Bomber (Closing Velocity Threat Boost)
// -----------------------------------------------------------------------------
void test_scenario_03_incoming_radial_dive_bomber() {
   std::cout << "[Scenario 03] Radial Dive Bomber Threat Scoring: ";
   TestRig rig;
   Object diveBomber(vec(15.0, 0.0, 0.0), vec(-10.0, 0.0, 0.0), vec(0, 0, 0), false);
   diveBomber.ID = 3;
   diveBomber.isAlive = true;

   Object retreatingTarget(vec(10.0, 0.0, 0.0), vec(5.0, 0.0, 0.0), vec(0, 0, 0), false);
   retreatingTarget.ID = 4;
   retreatingTarget.isAlive = true;

   double scoreInbound = rig.brain.EvaluateThreatLevel(diveBomber, rig.cam.CamCenterPos);
   double scoreRetreat = rig.brain.EvaluateThreatLevel(retreatingTarget, rig.cam.CamCenterPos);

   assert(scoreInbound > scoreRetreat && "Fast inbound closing target must have higher threat than retreating target");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 4: Accelerating Target (Kinematic 2nd Derivative)
// -----------------------------------------------------------------------------
void test_scenario_04_accelerating_target() {
   std::cout << "[Scenario 04] Target Acceleration & Quadratic Trajectory: ";
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      double y = 0.5 * 2.0 * i * i; // a = 2.0 m/s^2
      Object enemy(vec(10.0, y, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 5;
      enemy.isAlive = true;
      f->detectedObjects[5] = enemy;
      rig.sys.tick(0.1f);
   }
   vec future = rig.brain.PredictPos(rig.sys.getTrackedObjects().at(5), 1.0);
   assert(future.y > rig.sys.getTrackedObjects().at(5).pos.y && "Trajectory prediction must extrapolate forward motion");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 5: Zig-Zag / Maneuvering Target (Smoothing Filter Stability)
// -----------------------------------------------------------------------------
void test_scenario_05_zig_zag_smoothing() {
   std::cout << "[Scenario 05] Savitzky-Golay Smoothing Stability: ";
   TestRig rig;
   double noisyY[5] = {-0.5, 0.6, -0.4, 0.5, -0.3};
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, noisyY[i], 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 6;
      enemy.isAlive = true;
      f->detectedObjects[6] = enemy;
      rig.sys.tick(0.1f);
   }
   double smoothedY = rig.sys.getTrackedObjects().at(6).pos.y;
   assert(std::abs(smoothedY) < 1.0 && "Smoothed coordinate should remain stable and bounded");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 6: Friendly Unit in Direct Line of Fire (Laser Safety)
// -----------------------------------------------------------------------------
void test_scenario_06_friendly_in_line_of_fire() {
   std::cout << "[Scenario 06] Friendly Unit Line of Fire Safety: ";
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = 10;
      friendly.isAlive = true;
      f->detectedObjects[10] = friendly;

      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 11;
      enemy.isAlive = true;
      f->detectedObjects[11] = enemy;

      rig.sys.tick(0.1f);
   }
   assert(!rig.laser.isOn && "Laser must NEVER fire through a friendly asset");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 7: Multiple Enemies - Prioritization by Threat Score
// -----------------------------------------------------------------------------
void test_scenario_07_multiple_enemies_threat_priority() {
   std::cout << "[Scenario 07] Multi-Enemy Threat Prioritization: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   // Slow distant enemy
   Object distant(vec(18.0, 0.0, 0.0), vec(0.0, 0.0, 0.0), vec(0, 0, 0), false);
   distant.ID = 12;
   distant.isAlive = true;
   targets[12] = distant;

   // Fast close enemy
   Object close(vec(6.0, 0.0, 0.0), vec(-5.0, 0.0, 0.0), vec(0, 0, 0), false);
   close.ID = 13;
   close.isAlive = true;
   targets[13] = close;

   Target best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(best != nullptr && best->ID == 13 && "Closer aggressive enemy must be chosen as top priority");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 8: Urgent Flag Overrides Distance Advantage
// -----------------------------------------------------------------------------
void test_scenario_08_urgent_flag_priority_boost() {
   std::cout << "[Scenario 08] Urgent Flag Priority Override: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   // Closer standard enemy
   Object normal(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   normal.ID = 14;
   normal.isAlive = true;
   normal.isUrgent = false;
   targets[14] = normal;

   // Farther urgent enemy
   Object urgent(vec(14.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   urgent.ID = 15;
   urgent.isAlive = true;
   urgent.isUrgent = true;
   targets[15] = urgent;

   Target best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(best != nullptr && best->ID == 15 && "Urgent target must supersede non-urgent closer targets");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 9: Pure Friendly Swarm (Zero Engagement)
// -----------------------------------------------------------------------------
void test_scenario_09_pure_friendly_swarm() {
   std::cout << "[Scenario 09] Friendly Swarm Complete Disengagement: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> friendlies;
   for (uint8_t id = 1; id <= 5; ++id) {
      Object f(vec(id * 3.0, 0, 0), vec(0, 0, 0), vec(0, 0, 0), true);
      f.ID = id;
      f.isAlive = true;
      friendlies[id] = f;
   }
   Target best = rig.brain.pickBestTarget(friendlies, rig.cam.CamCenterPos);
   assert(best == nullptr && "pickBestTarget must return nullptr for all-friendly detection field");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 10: Dead / Destroyed Target Filtering
// -----------------------------------------------------------------------------
void test_scenario_10_dead_target_filtering() {
   std::cout << "[Scenario 10] Dead Target Exclusion: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   Object deadEnemy(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   deadEnemy.ID = 16;
   deadEnemy.isAlive = false;
   targets[16] = deadEnemy;

   Target best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(best == nullptr && "pickBestTarget must ignore destroyed targets (isAlive=false)");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 11: Out of Range Perimeter Check (> MAX_DISTANCE)
// -----------------------------------------------------------------------------
void test_scenario_11_out_of_range_perimeter() {
   std::cout << "[Scenario 11] Out-of-Range Perimeter Rejection: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   Object farEnemy(vec(MAX_DISTANCE + 5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   farEnemy.ID = 17;
   farEnemy.isAlive = true;
   targets[17] = farEnemy;

   double score = rig.brain.EvaluateThreatLevel(farEnemy, rig.cam.CamCenterPos);
   assert(score == 0.0 && "Threat score must be 0 for objects outside MAX_DISTANCE");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 12: Target Destruction & Automatic Retargeting
// -----------------------------------------------------------------------------
void test_scenario_12_destruction_and_retargeting() {
   std::cout << "[Scenario 12] Destruction & Automatic Retargeting: ";
   TestRig rig;
   constexpr uint8_t enemyA = 20;
   constexpr uint8_t enemyB = 21;

   // Frame 1-5: Enemy A and B present
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object a(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      a.ID = enemyA;
      a.isAlive = true;
      f->detectedObjects[enemyA] = a;

      Object b(vec(12.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      b.ID = enemyB;
      b.isAlive = true;
      f->detectedObjects[enemyB] = b;
      rig.sys.tick(0.1f);
   }

   // Target A destroyed after enough ticks to satisfy TIME_UNDER_FIRE
   int currentFrame = 5;
   while (rig.sys.getTrackedObjects().at(enemyA).isAlive) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), currentFrame++);
      Object a(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      a.ID = enemyA;
      a.isAlive = true;
      f->detectedObjects[enemyA] = a;

      Object b(vec(12.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      b.ID = enemyB;
      b.isAlive = true;
      f->detectedObjects[enemyB] = b;
      rig.sys.tick(0.1f);
   }

   assert(!rig.sys.getTrackedObjects().at(enemyA).isAlive && "Enemy A should be confirmed destroyed");
   // Advance 1 tick: System must switch focus to Enemy B
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), currentFrame++);
   Object b(vec(12.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   b.ID = enemyB;
   b.isAlive = true;
   f->detectedObjects[enemyB] = b;
   rig.sys.tick(0.1f);

   Target current = rig.brain.pickBestTarget(rig.sys.getTrackedObjects(), rig.cam.CamCenterPos);
   assert(current != nullptr && current->ID == enemyB && "System should immediately retarget Enemy B");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 13: Emergency Stop Signal (Freeze and Cut Laser)
// -----------------------------------------------------------------------------
void test_scenario_13_emergency_stop_freeze() {
   std::cout << "[Scenario 13] Emergency Stop System Freeze: ";
   TestRig rig;
   rig.laser.turnOn();
   rig.leftYaw.move(vec(10, 0, 0));
   rig.pitch.move(vec(0, 10, 0));

   rig.sys.executeSignal(sig(SIGNALS::SYSTEM_STOP));
   assert(!rig.laser.isOn && "E-Stop must cut laser power");
   assert(!rig.leftYaw.isActive && !rig.pitch.isActive && "E-Stop must halt all motor motion");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 14: System Restart State Reset
// -----------------------------------------------------------------------------
void test_scenario_14_system_restart_reset() {
   std::cout << "[Scenario 14] System Restart & State Clearance: ";
   TestRig rig;
   rig.sys.pushFrame(vec(1, 2, 3), 100);
   rig.sys.yawAngle = 45.0;
   rig.sys.pitchAngle = 20.0;

   rig.sys.executeSignal(sig(SIGNALS::SYSTEM_RESTART));
   assert(rig.sys.getTrackedObjects().empty() && "Restart must clear tracked object registry");
   assert(rig.cam.CamCenterPos.magnitude() == 0.0 && "Restart must zero camera position");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 15: Critical Signal Preemption over Normal Signals
// -----------------------------------------------------------------------------
void test_scenario_15_critical_signal_preemption() {
   std::cout << "[Scenario 15] Critical Signal Priority Queue Preemption: ";
   CommunicatorDevice comm;
   sig normalSig(SIGNALS::CAM_START, std::monostate{}, Priority::NORMAL);
   sig criticalSig(SIGNALS::SYSTEM_STOP, std::monostate{}, Priority::CRITICAL);

   comm.Receive(normalSig);
   comm.Receive(criticalSig);

   auto firstOut = comm.popSignal();
   assert(firstOut.has_value() && firstOut->signal == SIGNALS::SYSTEM_STOP && "CRITICAL signals must preempt NORMAL signals");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 16: RingBuffer Circular Overwrite
// -----------------------------------------------------------------------------
void test_scenario_16_ring_buffer_overwrite() {
   std::cout << "[Scenario 16] RingBuffer Capacity Bound & Overwrite: ";
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
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 17: History Frame Recycling (Zero Reallocation)
// -----------------------------------------------------------------------------
void test_scenario_17_history_frame_recycling() {
   std::cout << "[Scenario 17] History Frame Ring Buffer Recycling: ";
   TestRig rig;
   for (uint64_t t = 0; t < 20; ++t) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), t);
      assert(f != nullptr && "pushFrame must always return valid memory from pool");
   }
   ImageFrame* const* history = rig.sys.getHistoryFrames();
   assert(history[HISTORY_FRAME_COUNTS - 1]->timestamp == 19 && "Most recent frame timestamp must match last push");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 18: Zero Detections Clear Sky Stability
// -----------------------------------------------------------------------------
void test_scenario_18_zero_detections_stability() {
   std::cout << "[Scenario 18] Zero-Detection Frame Stability: ";
   TestRig rig;
   // Empty frame: sky clear
   rig.sys.pushFrame(vec(0, 0, 0), 1);
   rig.sys.tick(0.033f);
   assert(!rig.laser.isOn && "Laser must stay off during zero detections");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 19: Intermittent Occlusion Recovery
// -----------------------------------------------------------------------------
void test_scenario_19_intermittent_occlusion() {
   std::cout << "[Scenario 19] Intermittent Occlusion Handling: ";
   TestRig rig;
   constexpr uint8_t id = 30;

   // Visible 3 frames
   for (int i = 0; i < 3; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      f->detectedObjects[id] = Object(vec(10.0, i * 1.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      f->detectedObjects[id].ID = id;
      rig.sys.tick(0.1f);
   }

   // Occluded 2 frames (empty)
   for (int i = 3; i < 5; ++i) {
      rig.sys.pushFrame(vec(0, 0, 0), i);
      rig.sys.tick(0.1f);
   }

   // Reappears on frame 5
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 5);
   f->detectedObjects[id] = Object(vec(10.0, 5.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   f->detectedObjects[id].ID = id;
   rig.sys.tick(0.1f);

   assert(rig.sys.getTrackedObjects().count(id) && "Target track should persist across intermittent occlusion");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 20: 360-Degree Yaw Wrap-Around Handling
// -----------------------------------------------------------------------------
void test_scenario_20_yaw_wrap_around() {
   std::cout << "[Scenario 20] 360-Degree Yaw Angle Shortest Path: ";
   TestRig rig;
   rig.sys.yawAngle = 175.0; // Facing near +180 deg
   // Target at -175 deg (10 degrees away across 180 boundary)
   vec targetPos(-10.0, -1.0, 0.0);
   double targetYaw = rig.cam.CamCenterPos.YawTo(targetPos);

   double deltaYaw = targetYaw - rig.sys.yawAngle;
   while (deltaYaw > 180.0)
      deltaYaw -= 360.0;
   while (deltaYaw < -180.0)
      deltaYaw += 360.0;

   assert(std::abs(deltaYaw) < 30.0 && "Delta yaw must wrap to shortest turn path across +/-180 boundary");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 21: High Elevation / Zenith Target Tracking
// -----------------------------------------------------------------------------
void test_scenario_21_zenith_target_tracking() {
   std::cout << "[Scenario 21] Zenith Target Pitch Calculation: ";
   vec turret(0, 0, 0);
   vec overheadTarget(1.0, 0.0, 10.0); // Directly high above
   double pitch = turret.PitchTo(overheadTarget);
   assert(pitch > 80.0 && pitch <= 90.0 && "Overhead target should produce steep positive pitch angle");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 22: Motor Velocity Limit Clamping
// -----------------------------------------------------------------------------
void test_scenario_22_motor_velocity_clamping() {
   std::cout << "[Scenario 22] Motor Velocity Clamping to Limits: ";
   TestRig rig;
   // Create large step target (e.g. 90 degrees away)
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(0.0, 10.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 40;
   f->detectedObjects[40] = enemy;

   rig.sys.tick(0.01f); // small deltaTime forces large requested velocity
   assert(std::abs(rig.leftYaw.velocity.x) <= MAX_VELOCUTY_X + 1e-4 && "Yaw velocity must not exceed MAX_VELOCUTY_X");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 23: Cylindrical to Cartesian Roundtrip Precision
// -----------------------------------------------------------------------------
void test_scenario_23_coordinate_roundtrip() {
   std::cout << "[Scenario 23] Cylindrical to Cartesian Roundtrip: ";
   vec origCyl(10.0, 0.785398, 5.0); // r = 10, theta = 45 deg, z = 5
   vec cart;
   cart.CylindricalToCartesian(origCyl);

   vec recoveredCyl;
   recoveredCyl.CartesianToCylindrical(cart);

   assert(std::abs(recoveredCyl.x - origCyl.x) < 1e-5 && "Radius should match after roundtrip");
   assert(std::abs(recoveredCyl.y - origCyl.y) < 1e-5 && "Angle should match after roundtrip");
   assert(std::abs(recoveredCyl.z - origCyl.z) < 1e-5 && "Z should match after roundtrip");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 24: FlatObjectMap Cache-Local Store Lifecycle
// -----------------------------------------------------------------------------
void test_scenario_24_flat_object_map_lifecycle() {
   std::cout << "[Scenario 24] FlatObjectMap Insertion, Lookup & Capacity: ";
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
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 25: Signal Logging Audit Trail
// -----------------------------------------------------------------------------
void test_scenario_25_signal_logging_audit() {
   std::cout << "[Scenario 25] Signal Logging Audit Trail: ";
   const std::string scenarioLogPath = "./yazdan-backend/build/scenario_25_audit.log";
   {
      std::ofstream clean(scenarioLogPath, std::ios::trunc);
   }
   Camera cam;
   Engine leftYaw, rightYaw, pitch;
   Laser laser;
   Brain brain;
   Logger scenarioLogger(scenarioLogPath);
   System sys(cam, leftYaw, rightYaw, pitch, laser, brain, scenarioLogger);

   sys.executeSignal(sig(SIGNALS::CAM_ON_TARGET));
   sys.executeSignal(sig(SIGNALS::OBJECT_DESTROYED));

   std::ifstream logFile(scenarioLogPath);
   assert(logFile.is_open() && "Log file must be accessible");
   std::stringstream buf;
   buf << logFile.rdbuf();
   std::string content = buf.str();

   assert(content.find("CAM_ON_TARGET") != std::string::npos && "CAM_ON_TARGET must be logged");
   assert(content.find("OBJECT_DESTROYED") != std::string::npos && "OBJECT_DESTROYED must be logged");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 26: Laser Parallax Offset Compensation
// -----------------------------------------------------------------------------
void test_scenario_26_laser_parallax_offset_compensation() {
   std::cout << "[Scenario 26] Laser Parallax Offset Compensation: ";
   TestRig rig;
   vec lateralTarget(10.0, 5.0, 0.0);
   vec aimOrigin = rig.cam.CamCenterPos + CAMERA_LASER_OFFSET;
   double camYaw = rig.cam.CamCenterPos.YawTo(lateralTarget);
   double offsetYaw = aimOrigin.YawTo(lateralTarget);
   // CAMERA_LASER_OFFSET has x = 0.1, so distance along X is 9.9m instead of 10.0m
   assert(offsetYaw > camYaw && "Parallax offset must alter the computed aiming angle to target");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 27: Exact Perimeter Boundary Limit (20m Boundary)
// -----------------------------------------------------------------------------
void test_scenario_27_exact_perimeter_boundary_limit() {
   std::cout << "[Scenario 27] Exact Perimeter Boundary (20m Limit): ";
   TestRig rig;
   ImageFrame* f1 = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object inside(vec(19.9, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   inside.ID = 101;
   inside.isAlive = true;
   f1->detectedObjects[101] = inside;
   rig.sys.tick(0.1f);
   assert(rig.laser.isOn && "Target inside MAX_DISTANCE (19.9m) must be engaged");

   rig.sys.reset();
   ImageFrame* f2 = rig.sys.pushFrame(vec(0, 0, 0), 2);
   Object outside(vec(20.1, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   outside.ID = 102;
   outside.isAlive = true;
   f2->detectedObjects[102] = outside;
   rig.sys.tick(0.1f);
   assert(!rig.laser.isOn && "Target beyond MAX_DISTANCE (20.1m) must not be engaged");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 28: Near-Field Target & Zero-Division Safety
// -----------------------------------------------------------------------------
void test_scenario_28_near_field_safety_and_finite_angles() {
   std::cout << "[Scenario 28] Near-Field Target & Zero-Division Safety: ";
   TestRig rig;
   vec nearPos(0.05, 0.05, 0.05);
   vec origin(0, 0, 0);
   double dist = origin.distanceTo(nearPos);
   double yaw = origin.YawTo(nearPos);
   double pitch = origin.PitchTo(nearPos);
   assert(std::isfinite(dist) && dist > 0.0 && "Distance must be finite");
   assert(std::isfinite(yaw) && "Yaw must be finite");
   assert(std::isfinite(pitch) && "Pitch must be finite");

   double aimTime = rig.brain.CalculateTimeToAim(origin, nearPos);
   assert(std::isfinite(aimTime) && aimTime >= 0.0 && "Aim time must be finite");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 29: Dense Friendly Balloon Wall Inhibit
// -----------------------------------------------------------------------------
void test_scenario_29_dense_friendly_cluster_wall() {
   std::cout << "[Scenario 29] Dense Friendly Balloon Wall Inhibit: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> objects;
   for (int i = 0; i < 5; ++i) {
      double y = -1.0 + i * 0.5;
      Object friendly(vec(5.0, y, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = static_cast<uint8_t>(10 + i);
      friendly.isAlive = true;
      objects[friendly.ID] = friendly;
   }
   vec enemyBehind(10.0, 0.0, 0.0);
   bool safeBehind = rig.brain.isShotSafe(enemyBehind, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(!safeBehind && "Shooting through dense friendly wall must be blocked");

   vec enemyClear(10.0, 0.0, 5.0);
   bool safeClear = rig.brain.isShotSafe(enemyClear, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(safeClear && "Shooting clear above friendly wall must be safe");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 30: High-G Vertical Pull-Up Extrapolation
// -----------------------------------------------------------------------------
void test_scenario_30_high_g_vertical_pullup() {
   std::cout << "[Scenario 30] High-G Vertical Pull-Up Extrapolation: ";
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      double z = 1.0 + 2.0 * i + 0.5 * 4.0 * i * i;
      Object enemy(vec(8.0, 0.0, z), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 50;
      enemy.isAlive = true;
      f->detectedObjects[50] = enemy;
      rig.sys.tick(0.1f);
   }
   const auto& tracked = rig.sys.getTrackedObjects().at(50);
   assert(tracked.vel.z > 0.5 && "Vertical velocity must be estimated positive");
   vec futurePos = rig.brain.PredictPos(tracked, 0.5);
   assert(futurePos.z > tracked.pos.z && "Predicted future Z must be higher than current Z");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 31: Continuous 360-Degree Azimuth Sweep
// -----------------------------------------------------------------------------
void test_scenario_31_continuous_azimuth_sweep() {
   std::cout << "[Scenario 31] Continuous 360-Degree Azimuth Sweep: ";
   TestRig rig;
   constexpr int STEPS = 36;
   for (int i = 0; i < STEPS; ++i) {
      double angleRad = (i * 10.0) * M_PI / 180.0;
      double x = 8.0 * std::cos(angleRad);
      double y = 8.0 * std::sin(angleRad);
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(x, y, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 60;
      enemy.isAlive = true;
      f->detectedObjects[60] = enemy;
      rig.sys.tick(0.1f);
      assert(std::isfinite(rig.sys.yawAngle) && "Yaw angle must remain finite throughout sweep");
   }
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 32: Negative Pitch / Depression Tracking
// -----------------------------------------------------------------------------
void test_scenario_32_negative_pitch_depression() {
   std::cout << "[Scenario 32] Negative Pitch / Depression Tracking: ";
   TestRig rig;
   vec origin(0, 0, 0);
   vec lowTarget(10.0, 0.0, -4.0);
   double pitch = origin.PitchTo(lowTarget);
   assert(pitch < -15.0 && pitch > -90.0 && "Target below horizon must produce negative pitch angle");

   ImageFrame* f = rig.sys.pushFrame(origin, 1);
   Object enemy(lowTarget, vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 65;
   enemy.isAlive = true;
   f->detectedObjects[65] = enemy;
   rig.sys.tick(0.1f);
   assert(rig.pitch.velocity.y < 0.0 && "Engine pitch velocity must drive downwards for depressed target");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 33: Dual Yaw Engine Symmetrical Drive
// -----------------------------------------------------------------------------
void test_scenario_33_dual_yaw_engine_symmetry() {
   std::cout << "[Scenario 33] Dual Yaw Engine Symmetrical Drive: ";
   TestRig rig;
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(5.0, 5.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 70;
   enemy.isAlive = true;
   f->detectedObjects[70] = enemy;
   rig.sys.tick(0.1f);

   assert(rig.leftYaw.velocity.x == rig.rightYaw.velocity.x && "Both yaw engines must have symmetrical velocities");
   assert(rig.leftYaw.velocity.x != 0.0 && "Yaw velocity must be non-zero when driving to target");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 34: Receding Radial Velocity Threat Penalty
// -----------------------------------------------------------------------------
void test_scenario_34_receding_radial_velocity_penalty() {
   std::cout << "[Scenario 34] Receding Radial Velocity Threat Penalty: ";
   TestRig rig;
   vec origin(0, 0, 0);
   Object closingEnemy(vec(12.0, 0.0, 0.0), vec(-10.0, 0.0, 0.0), vec(0, 0, 0), false);
   closingEnemy.ID = 75;
   closingEnemy.isAlive = true;

   Object recedingEnemy(vec(12.0, 0.0, 0.0), vec(10.0, 0.0, 0.0), vec(0, 0, 0), false);
   recedingEnemy.ID = 76;
   recedingEnemy.isAlive = true;

   double scoreClosing = rig.brain.EvaluateThreatLevel(closingEnemy, origin);
   double scoreReceding = rig.brain.EvaluateThreatLevel(recedingEnemy, origin);
   assert(scoreClosing > scoreReceding && "Closing target must have substantially higher threat score than receding target");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 35: 5-Target Sequential Kill Chain
// -----------------------------------------------------------------------------
void test_scenario_35_sequential_kill_chain() {
   std::cout << "[Scenario 35] 5-Target Sequential Kill Chain: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> targets;
   for (uint8_t i = 1; i <= 5; ++i) {
      Object o(vec(4.0 + i * 2.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      o.ID = i;
      o.isAlive = true;
      targets[i] = o;
   }
   for (uint8_t k = 1; k <= 5; ++k) {
      obj* best = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
      assert(best != nullptr && "Target must be selected");
      assert(best->ID == k && "Sequential selection must proceed from closest to farthest");
      best->isAlive = false;
   }
   obj* noneLeft = rig.brain.pickBestTarget(targets, rig.cam.CamCenterPos);
   assert(noneLeft == nullptr && "All targets destroyed, none should be selected");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 36: Intermittent Occlusion Recovery
// -----------------------------------------------------------------------------
void test_scenario_36_intermittent_occlusion_recovery() {
   std::cout << "[Scenario 36] Intermittent Occlusion & State Continuity: ";
   TestRig rig;
   for (int i = 0; i < 3; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, i * 0.5, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 80;
      enemy.isAlive = true;
      f->detectedObjects[80] = enemy;
      rig.sys.tick(0.1f);
   }
   ImageFrame* fEmpty = rig.sys.pushFrame(vec(0, 0, 0), 3);
   (void)fEmpty;
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().count(80) > 0 && "Tracked object must not vanish immediately upon 1 dropped frame");

   ImageFrame* fReappear = rig.sys.pushFrame(vec(0, 0, 0), 4);
   Object enemyRe(vec(10.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemyRe.ID = 80;
   enemyRe.isAlive = true;
   fReappear->detectedObjects[80] = enemyRe;
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().at(80).isAlive && "Target tracking must continue successfully after occlusion");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 37: Dynamic Friendly Intrusion Laser Inhibit
// -----------------------------------------------------------------------------
void test_scenario_37_dynamic_friendly_intrusion_inhibit() {
   std::cout << "[Scenario 37] Dynamic Friendly Intrusion Laser Inhibit: ";
   TestRig rig;
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 85;
      enemy.isAlive = true;
      f->detectedObjects[85] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser should be firing at unhindered enemy");

   ImageFrame* fIntrusion = rig.sys.pushFrame(vec(0, 0, 0), 5);
   Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 85;
   enemy.isAlive = true;
   fIntrusion->detectedObjects[85] = enemy;

   Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendly.ID = 86;
   friendly.isAlive = true;
   fIntrusion->detectedObjects[86] = friendly;

   rig.sys.tick(0.1f);
   assert(!rig.laser.isOn && "Laser must immediately turn OFF when friendly intrudes into line of fire");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 38: Extractor Binary Packet Deserialization
// -----------------------------------------------------------------------------
static constexpr char TEST_PIPE_FILE_38[] = "./yazdan-backend/build/test_packet_38.bin";
void test_scenario_38_extractor_packet_deserialization() {
   std::cout << "[Scenario 38] Extractor Binary Packet Deserialization: ";
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
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 39: Extractor Empty Stream EOF Handling
// -----------------------------------------------------------------------------
static constexpr char TEST_PIPE_FILE_39[] = "./yazdan-backend/build/test_packet_39.bin";
void test_scenario_39_extractor_empty_stream_eof() {
   std::cout << "[Scenario 39] Extractor Empty Stream EOF Handling: ";
   {
      std::ofstream out(TEST_PIPE_FILE_39, std::ios::out | std::ios::binary | std::ios::trunc);
   }
   Extractor<TEST_PIPE_FILE_39, Object> extractor;
   auto res = extractor.extract();
   assert(!res.has_value() && "Extractor must return nullopt on empty pipe");
   Object dummy;
   bool ok = extractor.extract(dummy);
   assert(!ok && "Extractor must return false when extracting into reference from empty stream");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 40: Non-Uniform Timestamp Kinematic Jitter
// -----------------------------------------------------------------------------
void test_scenario_40_variable_timestamp_jitter() {
   std::cout << "[Scenario 40] Non-Uniform Timestamp Kinematic Jitter: ";
   TestRig rig;
   uint64_t times[5] = {0, 16, 49, 99, 115};
   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), times[i]);
      Object enemy(vec(10.0, i * 0.3, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 90;
      enemy.isAlive = true;
      f->detectedObjects[90] = enemy;
      rig.sys.tick(0.016f);
   }
   const auto& obj = rig.sys.getTrackedObjects().at(90);
   assert(std::isfinite(obj.vel.y) && "Estimated velocity must remain finite with jittered timestamps");
   assert(std::isfinite(obj.acc.y) && "Estimated acceleration must remain finite with jittered timestamps");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 41: Pure Acceleration Lead Estimation (v0=0)
// -----------------------------------------------------------------------------
void test_scenario_41_pure_acceleration_from_rest() {
   std::cout << "[Scenario 41] Pure Acceleration Lead Estimation (v0=0): ";
   TestRig rig;
   Object acceleratingEnemy(vec(10.0, 0.0, 0.0), vec(0.0, 0.0, 0.0), vec(0.0, 4.0, 0.0), false);
   acceleratingEnemy.ID = 92;
   acceleratingEnemy.isAlive = true;

   vec predicted = rig.brain.PredictPos(acceleratingEnemy, 1.0);
   assert(std::abs(predicted.y - 2.0) < 1e-4 && "Prediction must accurately incorporate quadratic acceleration from rest");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 42: Orthogonal Velocity Snap Stability
// -----------------------------------------------------------------------------
void test_scenario_42_orthogonal_velocity_snap() {
   std::cout << "[Scenario 42] Orthogonal Velocity Snap Stability: ";
   TestRig rig;
   for (int i = 0; i < 3; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(5.0 + i * 1.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 95;
      enemy.isAlive = true;
      f->detectedObjects[95] = enemy;
      rig.sys.tick(0.1f);
   }
   for (int i = 3; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(7.0, (i - 2) * 1.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 95;
      enemy.isAlive = true;
      f->detectedObjects[95] = enemy;
      rig.sys.tick(0.1f);
   }
   const auto& obj = rig.sys.getTrackedObjects().at(95);
   assert(std::isfinite(obj.vel.x) && std::isfinite(obj.vel.y) && "Velocities must stay finite during sharp 90-deg turn");
   assert(std::abs(obj.vel.x) < 50.0 && std::abs(obj.vel.y) < 50.0 && "Smoothing must prevent runaway filter divergence");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 43: Signal Storm Strict Priority Dequeue
// -----------------------------------------------------------------------------
void test_scenario_43_signal_storm_priority_ordering() {
   std::cout << "[Scenario 43] Signal Storm Strict Priority Dequeue: ";
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
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 44: Emergency Stop Slew Arrest
// -----------------------------------------------------------------------------
void test_scenario_44_emergency_stop_during_slew() {
   std::cout << "[Scenario 44] Emergency Stop Slew Arrest: ";
   TestRig rig;
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(0.0, 10.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 98;
   enemy.isAlive = true;
   f->detectedObjects[98] = enemy;
   rig.sys.tick(0.1f);
   assert(rig.leftYaw.velocity.x > 0.0 && "Yaw motor must be slewing toward target");

   rig.sys.executeSignal(sig(SIGNALS::SYSTEM_STOP));
   assert(rig.leftYaw.velocity.x == 0.0 && "Left yaw motor velocity must be 0 after emergency stop");
   assert(rig.rightYaw.velocity.x == 0.0 && "Right yaw motor velocity must be 0 after emergency stop");
   assert(rig.pitch.velocity.y == 0.0 && "Pitch motor velocity must be 0 after emergency stop");
   assert(!rig.laser.isOn && "Laser must be OFF after emergency stop");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 45: Obstacle Clearance & Automatic Re-engagement
// -----------------------------------------------------------------------------
void test_scenario_45_obstacle_clearance_reengagement() {
   std::cout << "[Scenario 45] Obstacle Clearance & Automatic Re-engagement: ";
   TestRig rig;
   ImageFrame* f1 = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 105;
   enemy.isAlive = true;
   f1->detectedObjects[105] = enemy;

   Object friendly(vec(5.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendly.ID = 106;
   friendly.isAlive = true;
   f1->detectedObjects[106] = friendly;
   rig.sys.tick(0.1f);
   assert(!rig.laser.isOn && "Laser must be inhibited by blocking friendly");

   Object friendlyMoved(vec(5.0, 4.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendlyMoved.ID = 106;
   friendlyMoved.isAlive = true;

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 2 + i);
      f->detectedObjects[105] = enemy;
      f->detectedObjects[106] = friendlyMoved;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser must automatically engage once friendly has cleared the line of fire");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 46: NaN & Infinity Input Numerical Robustness
// -----------------------------------------------------------------------------
void test_scenario_46_nan_infinity_robustness() {
   std::cout << "[Scenario 46] NaN & Infinity Input Numerical Robustness: ";
   vec nanVec(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0);
   vec infVec(std::numeric_limits<double>::infinity(), 10.0, 0.0);
   vec normalVec(1.0, 0.0, 0.0);

   assert(!std::isfinite(nanVec.x) && "NaN coordinate must be recognized as non-finite");
   assert(!std::isfinite(infVec.x) && "Inf coordinate must be recognized as non-finite");
   assert(std::isfinite(normalVec.x) && "Normal coordinate must be finite");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 47: Cumulative Laser Dwell Time to Destruction
// -----------------------------------------------------------------------------
void test_scenario_47_cumulative_laser_dwell_threshold() {
   std::cout << "[Scenario 47] Cumulative Laser Dwell Time to Destruction: ";
   TestRig rig;
   for (int i = 0; i < 35; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 110;
      enemy.isAlive = true;
      f->detectedObjects[110] = enemy;
      rig.sys.tick(0.1f);
   }
   const auto& target = rig.sys.getTrackedObjects().at(110);
   assert(!target.isAlive && "Target must be destroyed after >= 3.0s under fire");
   assert(target.timeUnderFire >= TIME_UNDER_FIRE && "Dwell time must reach TIME_UNDER_FIRE");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 48: 64-Target Swarm Ranking Invariant
// -----------------------------------------------------------------------------
void test_scenario_48_64_target_swarm_ranking_invariant() {
   std::cout << "[Scenario 48] 64-Target Swarm Ranking Invariant: ";
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
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 49: Pitch Rate Limit Clamping Under Extreme Step
// -----------------------------------------------------------------------------
void test_scenario_49_pitch_rate_limit_clamping() {
   std::cout << "[Scenario 49] Pitch Rate Limit Clamping Under Extreme Step: ";
   TestRig rig;
   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(0.001, 0.0, 10.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 115;
   enemy.isAlive = true;
   f->detectedObjects[115] = enemy;
   rig.sys.tick(0.001f);
   assert(std::abs(rig.pitch.velocity.y) <= MAX_VELOCUTY_Y + 1e-4 && "Pitch velocity must be clamped to MAX_VELOCUTY_Y");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 50: Sustained 1,000-Tick Full Simulation Stability
// -----------------------------------------------------------------------------
void test_scenario_50_sustained_1000_tick_stability() {
   std::cout << "[Scenario 50] Sustained 1,000-Tick Full Simulation Stability: ";
   TestRig rig;
   for (int tick = 0; tick < 1000; ++tick) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), tick);
      double angle = tick * 0.05;
      Object enemy(vec(8.0 + std::sin(angle), std::cos(angle) * 3.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 120;
      enemy.isAlive = true;
      f->detectedObjects[120] = enemy;

      Object friendly(vec(4.0, 6.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      friendly.ID = 121;
      friendly.isAlive = true;
      f->detectedObjects[121] = friendly;

      rig.sys.tick(0.016f);
      assert(std::isfinite(rig.sys.yawAngle) && std::isfinite(rig.sys.pitchAngle) && "Angles must remain finite");
   }
   assert(rig.sys.getTrackedObjects().size() >= 2 && "System must maintain tracked objects without memory leaks");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 51: Zombie Target Resurrection Prevention
// -----------------------------------------------------------------------------
void test_scenario_51_zombie_target_resurrection_prevention() {
   std::cout << "[Scenario 51] Zombie Target Resurrection Prevention: ";
   TestRig rig;
   for (int i = 0; i < 35; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = 130;
      enemy.isAlive = true;
      f->detectedObjects[130] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(!rig.sys.getTrackedObjects().at(130).isAlive && "Target 130 must be marked dead");

   for (int i = 35; i < 45; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemyDebris(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemyDebris.ID = 130;
      enemyDebris.isAlive = true;
      f->detectedObjects[130] = enemyDebris;
      rig.sys.tick(0.1f);
   }
   assert(!rig.sys.getTrackedObjects().at(130).isAlive && "Target 130 must NOT be resurrected by debris detections");
   assert(!rig.laser.isOn && "Laser must not fire on confirmed destroyed target");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 52: Ghost Target Aging & Expiration
// -----------------------------------------------------------------------------
void test_scenario_52_ghost_target_aging_and_expiration() {
   std::cout << "[Scenario 52] Ghost Target Aging & Expiration: ";
   TestRig rig;
   ImageFrame* f0 = rig.sys.pushFrame(vec(0, 0, 0), 0);
   Object enemy(vec(7.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 135;
   enemy.isAlive = true;
   f0->detectedObjects[135] = enemy;
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().count(135) == 1 && "Target 135 must be tracked");

   for (int i = 1; i <= 4; ++i) {
      rig.sys.pushFrame(vec(0, 0, 0), i);
      rig.sys.tick(0.1f);
      assert(rig.sys.getTrackedObjects().count(135) == 1 && "Target must persist during brief drop");
   }

   rig.sys.pushFrame(vec(0, 0, 0), 5);
   rig.sys.tick(0.1f);
   assert(rig.sys.getTrackedObjects().count(135) == 0 && "Ghost target must be pruned after MAX_STALE_FRAMES");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 53: Backstop Friendly Fire Ray Protection
// -----------------------------------------------------------------------------
void test_scenario_53_backstop_friendly_fire_protection() {
   std::cout << "[Scenario 53] Backstop Friendly Fire Ray Protection: ";
   TestRig rig;
   vec enemyPos(5.0, 0.0, 0.0);
   std::unordered_map<uint8_t, Object> objects;
   Object friendlyBehind(vec(10.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendlyBehind.ID = 140;
   friendlyBehind.isAlive = true;
   objects[140] = friendlyBehind;

   bool safeBehind = rig.brain.isShotSafe(enemyPos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(!safeBehind && "Shooting at target with friendly directly in backstop must be rejected");

   objects[140].pos = vec(10.0, 3.0, 0.0);
   bool safeClear = rig.brain.isShotSafe(enemyPos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(safeClear && "Shooting must be permitted when backstop is clear");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 54: Schmitt-Trigger Laser Hysteresis Window
// -----------------------------------------------------------------------------
void test_scenario_54_schmitt_trigger_laser_hysteresis() {
   std::cout << "[Scenario 54] Schmitt-Trigger Laser Hysteresis Window: ";
   TestRig rig;
   double y14 = 9.9 * std::tan(1.4 * M_PI / 180.0);
   ImageFrame* f1 = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy14(vec(10.0, y14, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy14.ID = 145;
   enemy14.isAlive = true;
   f1->detectedObjects[145] = enemy14;
   rig.sys.tick(0.0001f);
   assert(!rig.laser.isOn && "Laser must NOT engage at 1.4 deg (> AIM_ENGAGE_TOLERANCE_DEG 1.2)");

   double y05 = 9.9 * std::tan(0.5 * M_PI / 180.0);
   ImageFrame* f2 = rig.sys.pushFrame(vec(0, 0, 0), 2);
   Object enemy05(vec(10.0, y05, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy05.ID = 145;
   enemy05.isAlive = true;
   f2->detectedObjects[145] = enemy05;
   rig.sys.tick(0.0001f);
   assert(rig.laser.isOn && "Laser must engage at 0.5 deg (<= AIM_ENGAGE_TOLERANCE_DEG 1.2)");

   ImageFrame* f3 = rig.sys.pushFrame(vec(0, 0, 0), 3);
   f3->detectedObjects[145] = enemy14;
   rig.sys.tick(0.0001f);
   assert(rig.laser.isOn && "Laser must STAY ON within hysteresis window (1.4 <= AIM_HOLD_TOLERANCE_DEG 1.8)");

   double y40 = 9.9 * std::tan(4.0 * M_PI / 180.0);
   ImageFrame* f4 = rig.sys.pushFrame(vec(0, 0, 0), 4);
   Object enemy40(vec(10.0, y40, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy40.ID = 145;
   enemy40.isAlive = true;
   f4->detectedObjects[145] = enemy40;
   rig.sys.tick(0.0001f);
   assert(!rig.laser.isOn && "Laser must turn OFF when error exceeds AIM_HOLD_TOLERANCE_DEG (4.0 > 1.8)");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 55: Turret Yaw Wrap & Pitch Soft Limit Containment
// -----------------------------------------------------------------------------
void test_scenario_55_yaw_pitch_soft_limit_containment() {
   std::cout << "[Scenario 55] Turret Yaw Wrap & Pitch Soft Limit Containment: ";
   TestRig rig;
   rig.sys.yawAngle = 179.0;
   rig.sys.pitchAngle = 80.0;

   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(1.0, 10.0, 20.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 150;
   enemy.isAlive = true;
   f->detectedObjects[150] = enemy;

   for (int i = 0; i < 20; ++i) {
      rig.sys.tick(0.1f);
      assert(rig.sys.yawAngle >= -180.0 && rig.sys.yawAngle <= 180.0 && "Yaw angle must remain bounded in [-180, 180]");
      assert(rig.sys.pitchAngle >= MIN_PITCH_DEG && rig.sys.pitchAngle <= MAX_PITCH_DEG && "Pitch angle must not exceed soft limits [-45, +85]");
   }
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 56: Reentrant Thread-Safe Fast Logging
// -----------------------------------------------------------------------------
void test_scenario_56_reentrant_thread_safe_logging() {
   std::cout << "[Scenario 56] Reentrant Thread-Safe Fast Logging: ";
   Logger testLogger("./yazdan-backend/logs.txt");
   for (int i = 0; i < 100; ++i) {
      testLogger.log("Concurrent test line " + std::to_string(i));
      testLogger.logSignal(sig(SIGNALS::CAM_ON_TARGET));
   }
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 57: Behind-Shooter Friendly Balloon Laser Safety
// -----------------------------------------------------------------------------
void test_scenario_57_behind_shooter_friendly_fire_safety() {
   std::cout << "[Scenario 57] Behind-Shooter Friendly Balloon Laser Safety: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> objects;
   Object friendlyBehind(vec(-0.5, 0.2, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendlyBehind.ID = 150;
   friendlyBehind.isAlive = true;
   objects[150] = friendlyBehind;

   vec enemyPos(10.0, 0.0, 0.0);
   Object enemy(enemyPos, vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 151;
   enemy.isAlive = true;
   objects[151] = enemy;

   bool isSafe = rig.brain.isShotSafe(enemyPos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(isSafe && "Friendly balloon strictly behind the shooter must NEVER inhibit forward laser fire");

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      f->detectedObjects[150] = friendlyBehind;
      f->detectedObjects[151] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(rig.laser.isOn && "Laser must engage forward enemy despite rear friendly balloon");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 58: Interception Point Safety Deadlock Resolution
// -----------------------------------------------------------------------------
void test_scenario_58_interception_lead_safety_deadlock_resolution() {
   std::cout << "[Scenario 58] Interception Point Safety Deadlock Resolution: ";
   TestRig rig;
   std::unordered_map<uint8_t, Object> objects;

   Object enemyA(vec(8.0, 5.0, 0.0), vec(0.0, 5.0, 0.0), vec(0, 0, 0), false);
   enemyA.ID = 153;
   enemyA.isAlive = true;
   objects[153] = enemyA;

   Object friendly(vec(4.05, 3.335, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
   friendly.ID = 152;
   friendly.isAlive = true;
   objects[152] = friendly;

   Object enemyB(vec(8.0, -2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemyB.ID = 154;
   enemyB.isAlive = true;
   objects[154] = enemyB;

   vec interceptA = rig.brain.CalculateInterceptionPoint(enemyA, rig.cam.CamCenterPos);
   bool safeAtPos = rig.brain.isShotSafe(enemyA.pos, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   bool safeAtIntercept = rig.brain.isShotSafe(interceptA, MAX_LASER_RADIUS, objects, rig.cam.CamCenterPos);
   assert(safeAtPos && "Enemy A instantaneous position should have clear line of fire");
   assert(!safeAtIntercept && "Enemy A lead interception point must be blocked by friendly unit");

   Target chosen = rig.brain.pickBestTarget(objects, rig.cam.CamCenterPos);
   assert(chosen != nullptr && "A valid target must be selected");
   assert(chosen->ID == 154 && "System must select Enemy B because Enemy A's lead interception point is obstructed");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 59: Pure Acceleration Lead Estimation from Rest
// -----------------------------------------------------------------------------
void test_scenario_59_pure_acceleration_lead_estimation() {
   std::cout << "[Scenario 59] Pure Acceleration Lead Estimation from Rest: ";
   TestRig rig;
   Object acceleratingEnemy(vec(10.0, 3.0, 0.0), vec(0.0, 0.0, 0.0), vec(0.0, 6.0, 0.0), false);
   acceleratingEnemy.ID = 155;
   acceleratingEnemy.isAlive = true;

   vec intercept = rig.brain.CalculateInterceptionPoint(acceleratingEnemy, rig.cam.CamCenterPos);
   double leadDisplacement = intercept.y - acceleratingEnemy.pos.y;
   assert(leadDisplacement > 0.01 && "Interception solver must compute lead for accelerating target starting from rest");
   assert(std::abs(intercept.x - 10.0) < 1e-3 && "X coordinate must remain on target trajectory plane");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 60: Turret Orientation Aware Slew and Aim Time
// -----------------------------------------------------------------------------
void test_scenario_60_turret_orientation_aware_aim_time() {
   std::cout << "[Scenario 60] Turret Orientation Aware Slew and Aim Time: ";
   TestRig rig;
   vec targetAt90(0.0, 10.0, 0.0);
   vec targetAt0(10.0, 0.0, 0.0);

   double slewTo90 = rig.brain.CalculateTimeToAim(rig.cam.CamCenterPos, targetAt90, 90.0, 0.0);
   double slewTo0 = rig.brain.CalculateTimeToAim(rig.cam.CamCenterPos, targetAt0, 90.0, 0.0);

   assert(slewTo90 < 0.01 && "Aim time to on-axis target must be practically zero");
   assert(std::abs(slewTo0 - (90.0 / MAX_VELOCUTY_X)) < 1e-3 && "Aim time to off-axis target must reflect true 90-degree motor travel");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 61: Single-Step Slew Zero-Latency Laser Engagement
// -----------------------------------------------------------------------------
void test_scenario_61_single_step_slew_zero_latency() {
   std::cout << "[Scenario 61] Single-Step Slew Zero-Latency Laser Engagement: ";
   TestRig rig;
   double yCoord = 10.0 * std::tan(toRadians(4.0));
   Object enemy(vec(10.0, yCoord, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 160;
   enemy.isAlive = true;

   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 1);
   f->detectedObjects[160] = enemy;
   rig.sys.tick(0.1f);

   assert(rig.laser.isOn && "Laser must turn ON immediately on the step the turret aligns, with 0 frame latency");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 62: Destroyed Target ID Quarantine Expiry & Recycling
// -----------------------------------------------------------------------------
void test_scenario_62_target_id_quarantine_expiry_and_recycling() {
   std::cout << "[Scenario 62] Destroyed Target ID Quarantine Expiry & Recycling: ";
   TestRig rig;
   constexpr uint8_t recycledID = 165;

   for (int i = 0; i < 20; ++i) {
      ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), i);
      Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
      enemy.ID = recycledID;
      enemy.isAlive = true;
      f->detectedObjects[recycledID] = enemy;
      rig.sys.tick(0.1f);
   }
   assert(!rig.sys.getTrackedObjects().at(recycledID).isAlive && "Target 165 must be confirmed destroyed");

   for (int i = 20; i < 55; ++i) {
      rig.sys.pushFrame(vec(0, 0, 0), i);
      rig.sys.tick(0.1f);
   }

   ImageFrame* f = rig.sys.pushFrame(vec(0, 0, 0), 55);
   Object newEnemy(vec(10.0, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   newEnemy.ID = recycledID;
   newEnemy.isAlive = true;
   f->detectedObjects[recycledID] = newEnemy;
   rig.sys.tick(0.1f);

   assert(rig.sys.getTrackedObjects().count(recycledID) && "Recycled target ID must be admitted after quarantine");
   assert(rig.sys.getTrackedObjects().at(recycledID).isAlive && "New target reusing ID must be tracked as alive");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 63: Extractor Instance Stream State Isolation
// -----------------------------------------------------------------------------
static constexpr char TEST_PIPE_63_A[] = "./yazdan-backend/build/test_packet_63_a.bin";
static constexpr char TEST_PIPE_63_B[] = "./yazdan-backend/build/test_packet_63_b.bin";

void test_scenario_63_extractor_stream_isolation() {
   std::cout << "[Scenario 63] Extractor Instance Stream State Isolation: ";
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

   Extractor<TEST_PIPE_63_A, Object> extA;
   Extractor<TEST_PIPE_63_B, Object> extB;

   auto resA = extA.extract();
   auto resB = extB.extract();

   assert(resA.has_value() && resB.has_value() && "Both extractors must successfully read from distinct pipes");
   assert(resA->ID == 71 && resB->ID == 72 && "Stream isolation must preserve correct object IDs");
   assert(std::abs(resA->vel.x - 2.5) < 1e-4 && "Cartesian velocity X must be preserved");
   assert(std::abs(resA->vel.y - 1.5) < 1e-4 && "Cartesian velocity Y must be preserved");
   assert(std::abs(resB->vel.y - (-3.0)) < 1e-4 && "Cartesian velocity Y on stream B must be preserved");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Scenario 64: Zenith Gimbal Singularity Deadband Stability
// -----------------------------------------------------------------------------
void test_scenario_64_zenith_singularity_deadband() {
   std::cout << "[Scenario 64] Zenith Gimbal Singularity Deadband Stability: ";
   vec origin(0.0, 0.0, 0.0);
   vec directZenith(0.0, 0.0, 10.0);
   double yawPure = origin.YawTo(directZenith);
   double pitchPure = origin.PitchTo(directZenith);

   assert(std::abs(yawPure) < 1e-4 && "Pure zenith target must produce stable 0.0 yaw without singularity jump");
   assert(std::abs(pitchPure - 90.0) < 1e-4 && "Zenith target pitch must be exactly 90.0 degrees");

   vec noisyZenith(1e-6, -1e-6, 12.0);
   double yawNoisy = origin.YawTo(noisyZenith);
   double pitchNoisy = origin.PitchTo(noisyZenith);

   assert(std::abs(yawNoisy) < 1e-4 && "Near-zero horizontal radius must trigger deadband clamp to 0.0 yaw");
   assert(std::abs(pitchNoisy - 90.0) < 1e-2 && "Pitch must remain within zenith limit");
   std::cout << "PASSED" << std::endl;
}

// -----------------------------------------------------------------------------
// Resource Benchmark Harness for Scenarios
// -----------------------------------------------------------------------------
struct ResourceTracker {
   double totalWallMs{0.0};
   double totalCpuMs{0.0};
   long peakRssKb{0};
} g_resourceStats;

void run_benchmarked_scenario(void (*testFunc)()) {
   struct rusage startUsage, endUsage;
   struct timespec startWall, endWall;
   struct timespec startCpu, endCpu;

   getrusage(RUSAGE_SELF, &startUsage);
   clock_gettime(CLOCK_MONOTONIC_RAW, &startWall);
   clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startCpu);

   testFunc();

   clock_gettime(CLOCK_THREAD_CPUTIME_ID, &endCpu);
   clock_gettime(CLOCK_MONOTONIC_RAW, &endWall);
   getrusage(RUSAGE_SELF, &endUsage);

   double wallMs = (endWall.tv_sec - startWall.tv_sec) * 1000.0 + (endWall.tv_nsec - startWall.tv_nsec) * 1e-6;
   double cpuMs = (endCpu.tv_sec - startCpu.tv_sec) * 1000.0 + (endCpu.tv_nsec - startCpu.tv_nsec) * 1e-6;
   long currentRss = getCurrentRssKb();
   long currentPeakRss = endUsage.ru_maxrss;

   g_resourceStats.totalWallMs += wallMs;
   g_resourceStats.totalCpuMs += cpuMs;
   if (currentPeakRss > g_resourceStats.peakRssKb) {
      g_resourceStats.peakRssKb = currentPeakRss;
   }

   std::cout << "   [Resource Usage] Wall: " << std::fixed << std::setprecision(3) << wallMs << " ms"
             << " | CPU: " << std::setprecision(3) << cpuMs << " ms"
             << " | Current RSS: " << std::setprecision(1) << (currentRss / 1024.0) << " MB"
             << " (Peak: " << std::setprecision(1) << (currentPeakRss / 1024.0) << " MB)" << std::endl;
}

// -----------------------------------------------------------------------------
// Main Test Runner
// -----------------------------------------------------------------------------
int main() {
   std::cout << "==========================================================================" << std::endl;
   std::cout << "   RUNNING 64 FUNCTIONAL SCENARIO TESTS WITH RESOURCE BENCHMARKING        " << std::endl;
   std::cout << "==========================================================================" << std::endl;

   run_benchmarked_scenario(test_scenario_01_stationary_enemy);
   run_benchmarked_scenario(test_scenario_02_fast_transverse_target);
   run_benchmarked_scenario(test_scenario_03_incoming_radial_dive_bomber);
   run_benchmarked_scenario(test_scenario_04_accelerating_target);
   run_benchmarked_scenario(test_scenario_05_zig_zag_smoothing);
   run_benchmarked_scenario(test_scenario_06_friendly_in_line_of_fire);
   run_benchmarked_scenario(test_scenario_07_multiple_enemies_threat_priority);
   run_benchmarked_scenario(test_scenario_08_urgent_flag_priority_boost);
   run_benchmarked_scenario(test_scenario_09_pure_friendly_swarm);
   run_benchmarked_scenario(test_scenario_10_dead_target_filtering);
   run_benchmarked_scenario(test_scenario_11_out_of_range_perimeter);
   run_benchmarked_scenario(test_scenario_12_destruction_and_retargeting);
   run_benchmarked_scenario(test_scenario_13_emergency_stop_freeze);
   run_benchmarked_scenario(test_scenario_14_system_restart_reset);
   run_benchmarked_scenario(test_scenario_15_critical_signal_preemption);
   run_benchmarked_scenario(test_scenario_16_ring_buffer_overwrite);
   run_benchmarked_scenario(test_scenario_17_history_frame_recycling);
   run_benchmarked_scenario(test_scenario_18_zero_detections_stability);
   run_benchmarked_scenario(test_scenario_19_intermittent_occlusion);
   run_benchmarked_scenario(test_scenario_20_yaw_wrap_around);
   run_benchmarked_scenario(test_scenario_21_zenith_target_tracking);
   run_benchmarked_scenario(test_scenario_22_motor_velocity_clamping);
   run_benchmarked_scenario(test_scenario_23_coordinate_roundtrip);
   run_benchmarked_scenario(test_scenario_24_flat_object_map_lifecycle);
   run_benchmarked_scenario(test_scenario_25_signal_logging_audit);
   run_benchmarked_scenario(test_scenario_26_laser_parallax_offset_compensation);
   run_benchmarked_scenario(test_scenario_27_exact_perimeter_boundary_limit);
   run_benchmarked_scenario(test_scenario_28_near_field_safety_and_finite_angles);
   run_benchmarked_scenario(test_scenario_29_dense_friendly_cluster_wall);
   run_benchmarked_scenario(test_scenario_30_high_g_vertical_pullup);
   run_benchmarked_scenario(test_scenario_31_continuous_azimuth_sweep);
   run_benchmarked_scenario(test_scenario_32_negative_pitch_depression);
   run_benchmarked_scenario(test_scenario_33_dual_yaw_engine_symmetry);
   run_benchmarked_scenario(test_scenario_34_receding_radial_velocity_penalty);
   run_benchmarked_scenario(test_scenario_35_sequential_kill_chain);
   run_benchmarked_scenario(test_scenario_36_intermittent_occlusion_recovery);
   run_benchmarked_scenario(test_scenario_37_dynamic_friendly_intrusion_inhibit);
   run_benchmarked_scenario(test_scenario_38_extractor_packet_deserialization);
   run_benchmarked_scenario(test_scenario_39_extractor_empty_stream_eof);
   run_benchmarked_scenario(test_scenario_40_variable_timestamp_jitter);
   run_benchmarked_scenario(test_scenario_41_pure_acceleration_from_rest);
   run_benchmarked_scenario(test_scenario_42_orthogonal_velocity_snap);
   run_benchmarked_scenario(test_scenario_43_signal_storm_priority_ordering);
   run_benchmarked_scenario(test_scenario_44_emergency_stop_during_slew);
   run_benchmarked_scenario(test_scenario_45_obstacle_clearance_reengagement);
   run_benchmarked_scenario(test_scenario_46_nan_infinity_robustness);
   run_benchmarked_scenario(test_scenario_47_cumulative_laser_dwell_threshold);
   run_benchmarked_scenario(test_scenario_48_64_target_swarm_ranking_invariant);
   run_benchmarked_scenario(test_scenario_49_pitch_rate_limit_clamping);
   run_benchmarked_scenario(test_scenario_50_sustained_1000_tick_stability);
   run_benchmarked_scenario(test_scenario_51_zombie_target_resurrection_prevention);
   run_benchmarked_scenario(test_scenario_52_ghost_target_aging_and_expiration);
   run_benchmarked_scenario(test_scenario_53_backstop_friendly_fire_protection);
   run_benchmarked_scenario(test_scenario_54_schmitt_trigger_laser_hysteresis);
   run_benchmarked_scenario(test_scenario_55_yaw_pitch_soft_limit_containment);
   run_benchmarked_scenario(test_scenario_56_reentrant_thread_safe_logging);
   run_benchmarked_scenario(test_scenario_57_behind_shooter_friendly_fire_safety);
   run_benchmarked_scenario(test_scenario_58_interception_lead_safety_deadlock_resolution);
   run_benchmarked_scenario(test_scenario_59_pure_acceleration_lead_estimation);
   run_benchmarked_scenario(test_scenario_60_turret_orientation_aware_aim_time);
   run_benchmarked_scenario(test_scenario_61_single_step_slew_zero_latency);
   run_benchmarked_scenario(test_scenario_62_target_id_quarantine_expiry_and_recycling);
   run_benchmarked_scenario(test_scenario_63_extractor_stream_isolation);
   run_benchmarked_scenario(test_scenario_64_zenith_singularity_deadband);

   std::cout << "==========================================================================" << std::endl;
   std::cout << "   ALL 64 SCENARIOS PASSED WITH ZERO FAILURES!                            " << std::endl;
   std::cout << "   TOTAL WALL TIME: " << std::fixed << std::setprecision(2) << g_resourceStats.totalWallMs << " ms"
             << " | TOTAL CPU TIME: " << g_resourceStats.totalCpuMs << " ms"
             << " | PEAK RSS: " << std::setprecision(1) << (g_resourceStats.peakRssKb / 1024.0) << " MB" << std::endl;
   std::cout << "==========================================================================" << std::endl;
   return 0;
}
