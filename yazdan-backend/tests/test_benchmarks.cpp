#include "../include/ardunio.hpp"
#include "../include/comms.hpp"
#include "../include/constants.hpp"
#include "../include/extractor.hpp"
#include "../include/logger.hpp"
#include "../include/logic.hpp"
#include "../include/physichs.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

using Clock = std::chrono::high_resolution_clock;

// -----------------------------------------------------------------------------
// Google Benchmark-style optimization barriers to prevent DCE & LICM under -O3
// -----------------------------------------------------------------------------
template <typename T>
inline __attribute__((always_inline)) void DoNotOptimize(const T& value) {
   asm volatile("" : : "m"(value) : "memory");
}

template <typename T>
inline __attribute__((always_inline)) void DoNotOptimize(T& value) {
   asm volatile("" : "+m"(value) : : "memory");
}

// Helper to format benchmark output
void print_bm(int id, const std::string& name, double elapsed_ms, uint64_t ops, const std::string& unit = "ops/sec") {
   double seconds = elapsed_ms / 1000.0;
   double throughput = (seconds > 1e-9) ? (static_cast<double>(ops) / seconds) : 0.0;
   double latency_ns = (ops > 0) ? ((elapsed_ms * 1e6) / static_cast<double>(ops)) : 0.0;

   std::cout << " [BM " << std::setw(2) << std::setfill('0') << id << "] "
             << std::setfill(' ') << std::left << std::setw(42) << name
             << " | " << std::right << std::setw(8) << std::fixed << std::setprecision(2) << elapsed_ms << " ms"
             << " | " << std::setw(10) << std::fixed << std::setprecision(1) << latency_ns << " ns/op"
             << " | " << std::setw(12) << std::fixed << std::setprecision(0) << throughput << " " << unit
             << std::endl;
}

// -----------------------------------------------------------------------------
// BM 01: Vector3D Basic Arithmetic (Addition & Scalar Mult)
// -----------------------------------------------------------------------------
void bm_01_vector_arithmetic() {
   constexpr uint64_t N = 10'000'000;
   vec v1(1.5, 2.5, 3.5), v2(0.5, 1.5, 2.5);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(v1);
      v1 = (v1 + v2) * 0.999999;
      DoNotOptimize(v1);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(1, "Vector3D Arithmetic (Add + Scalar)", ms, N);
}

// -----------------------------------------------------------------------------
// BM 02: Vector3D Dot and Cross Products
// -----------------------------------------------------------------------------
void bm_02_vector_dot_cross() {
   constexpr uint64_t N = 5'000'000;
   vec v1(1.0, 2.0, 3.0), v2(4.0, 5.0, 6.0);
   double dotAccum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(v1);
      dotAccum += (v1 * v2);
      v1 = v1.cross(v2) * 0.00001;
      DoNotOptimize(v1);
      DoNotOptimize(dotAccum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)dotAccum;
   print_bm(2, "Vector3D Dot & Cross Products", ms, N);
}

// -----------------------------------------------------------------------------
// BM 03: Vector3D Trigonometric Angles (YawTo & PitchTo)
// -----------------------------------------------------------------------------
void bm_03_vector_angles() {
   constexpr uint64_t N = 1'000'000;
   vec origin(0, 0, 0);
   vec target(10.0, 5.0, 2.0);
   double accum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(target);
      accum += origin.YawTo(target) + origin.PitchTo(target);
      DoNotOptimize(accum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)accum;
   print_bm(3, "Vector3D YawTo & PitchTo Trigonometry", ms, N);
}

// -----------------------------------------------------------------------------
// BM 04: Cylindrical to Cartesian Coordinate Conversion
// -----------------------------------------------------------------------------
void bm_04_cylindrical_to_cartesian() {
   constexpr uint64_t N = 2'000'000;
   vec cyl(10.0, 0.785, 5.0);
   vec cart;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(cyl);
      cart.CylindricalToCartesian(cyl);
      DoNotOptimize(cart);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(4, "CylindricalToCartesian Conversion", ms, N);
}

// -----------------------------------------------------------------------------
// BM 05: Cartesian to Cylindrical Coordinate Conversion
// -----------------------------------------------------------------------------
void bm_05_cartesian_to_cylindrical() {
   constexpr uint64_t N = 2'000'000;
   vec cart(7.07, 7.07, 5.0);
   vec cyl;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(cart);
      cyl.CartesianToCylindrical(cart);
      DoNotOptimize(cyl);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(5, "CartesianToCylindrical Conversion", ms, N);
}

// -----------------------------------------------------------------------------
// BM 06: RingBuffer Push and Pop Operations
// -----------------------------------------------------------------------------
void bm_06_ring_buffer_push_pop() {
   constexpr uint64_t N = 2'000'000;
   RingBuffer<sig, 32> buffer;
   sig s(SIGNALS::CAM_ON_TARGET);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      buffer.push(s);
      DoNotOptimize(buffer.front());
      buffer.pop();
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(6, "RingBuffer Push & Pop Cycle", ms, N);
}

// -----------------------------------------------------------------------------
// BM 07: CommunicatorDevice Multi-Priority Queue Insert & Pop
// -----------------------------------------------------------------------------
void bm_07_communicator_priority_queue() {
   constexpr uint64_t N = 500'000;
   CommunicatorDevice comm;
   sig crit(SIGNALS::SYSTEM_STOP, std::monostate{}, Priority::CRITICAL);
   sig norm(SIGNALS::CAM_ON_TARGET, std::monostate{}, Priority::NORMAL);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      comm.Receive(norm);
      comm.Receive(crit);
      auto s1 = comm.popSignal();
      auto s2 = comm.popSignal();
      DoNotOptimize(s1);
      DoNotOptimize(s2);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(7, "Communicator Multi-Priority Queuing", ms, N * 2);
}

// -----------------------------------------------------------------------------
// BM 08: FlatObjectMap Cache-Local Lookup
// -----------------------------------------------------------------------------
void bm_08_flat_object_map_lookup() {
   constexpr uint64_t N = 2'000'000;
   FlatObjectMap<32> map;
   for (uint8_t i = 0; i < 32; ++i) {
      map.insert_or_assign(i, Object(vec(i, 0, 0)));
   }
   double xAccum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      uint8_t key = static_cast<uint8_t>(i & 31);
      xAccum += map.at(key).pos.x;
      DoNotOptimize(xAccum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)xAccum;
   print_bm(8, "FlatObjectMap Cache-Local Lookup", ms, N);
}

// -----------------------------------------------------------------------------
// BM 09: FlatObjectMap Insert and Erase Churn
// -----------------------------------------------------------------------------
void bm_09_flat_object_map_insert_erase() {
   constexpr uint64_t N = 500'000;
   FlatObjectMap<32> map;
   Object o(vec(1, 2, 3));
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      uint8_t id = static_cast<uint8_t>(i & 31);
      map.insert_or_assign(id, o);
      map.erase(id);
      DoNotOptimize(map);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(9, "FlatObjectMap Insert/Erase Churn", ms, N * 2);
}

// -----------------------------------------------------------------------------
// BM 10: FlatObjectMap vs std::unordered_map Comparison
// -----------------------------------------------------------------------------
void bm_10_map_comparison() {
   constexpr uint64_t N = 1'000'000;
   std::unordered_map<uint8_t, Object> stdMap;
   for (uint8_t i = 0; i < 32; ++i) {
      stdMap[i] = Object(vec(i, 0, 0));
   }
   double accum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      uint8_t key = static_cast<uint8_t>(i & 31);
      accum += stdMap.at(key).pos.x;
      DoNotOptimize(accum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)accum;
   print_bm(10, "std::unordered_map Lookup Baseline", ms, N);
}

// -----------------------------------------------------------------------------
// BM 11: Brain::PredictPos Kinematic Extrapolation
// -----------------------------------------------------------------------------
void bm_11_predict_pos() {
   constexpr uint64_t N = 2'000'000;
   Brain brain;
   Object o(vec(10, 0, 0), vec(2, 1, 0), vec(0.5, 0, 0));
   vec res;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(o);
      res = brain.PredictPos(o, 0.5);
      DoNotOptimize(res);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)res;
   print_bm(11, "Brain::PredictPos Extrapolation", ms, N);
}

// -----------------------------------------------------------------------------
// BM 12: Brain::CalculateTimeToAim Latency
// -----------------------------------------------------------------------------
void bm_12_calculate_time_to_aim() {
   constexpr uint64_t N = 1'000'000;
   Brain brain;
   vec cam(0, 0, 0);
   vec target(10, 5, 2);
   double tAccum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(target);
      tAccum += brain.CalculateTimeToAim(cam, target);
      DoNotOptimize(tAccum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)tAccum;
   print_bm(12, "Brain::CalculateTimeToAim Latency", ms, N);
}

// -----------------------------------------------------------------------------
// BM 13: Brain::CalculateInterceptionPoint (Iterative Convergence)
// -----------------------------------------------------------------------------
void bm_13_calculate_interception_point() {
   constexpr uint64_t N = 100'000;
   Brain brain;
   Object o(vec(10, -5, 1), vec(3, 2, 0), vec(0, 0, 0));
   vec cam(0, 0, 0);
   vec p;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(o);
      p = brain.CalculateInterceptionPoint(o, cam);
      DoNotOptimize(p);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)p;
   print_bm(13, "Brain::CalculateInterceptionPoint", ms, N);
}

// -----------------------------------------------------------------------------
// BM 14: Brain::EvaluateThreatLevel Multi-Criteria Scoring
// -----------------------------------------------------------------------------
void bm_14_evaluate_threat_level() {
   constexpr uint64_t N = 1'000'000;
   Brain brain;
   Object o(vec(8, 2, 1), vec(-5, 0, 0), vec(-1, 0, 0), false);
   o.isAlive = true;
   vec cam(0, 0, 0);
   double score = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(o);
      score += brain.EvaluateThreatLevel(o, cam);
      DoNotOptimize(score);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)score;
   print_bm(14, "Brain::EvaluateThreatLevel Scoring", ms, N);
}

// -----------------------------------------------------------------------------
// BM 15: Brain::isShotSafe Ray-Sphere Collision with Friendly Obstacles
// -----------------------------------------------------------------------------
void bm_15_is_shot_safe() {
   constexpr uint64_t N = 200'000;
   Brain brain;
   std::unordered_map<uint8_t, Object> friendlies;
   for (uint8_t i = 1; i <= 8; ++i) {
      Object f(vec(i * 2.0, 1.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      f.isAlive = true;
      friendlies[i] = f;
   }
   vec cam(0, 0, 0);
   vec target(15, 0, 0);
   bool safe = false;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(target);
      safe = brain.isShotSafe(target, MAX_LASER_RADIUS, friendlies, cam);
      DoNotOptimize(safe);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)safe;
   print_bm(15, "Brain::isShotSafe Collision Check", ms, N);
}

// -----------------------------------------------------------------------------
// BM 16: Brain::CalculateVelocity Multi-Target Finite Difference
// -----------------------------------------------------------------------------
void bm_16_calculate_velocity_swarm() {
   constexpr uint64_t N = 10'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), i);
      for (uint8_t id = 1; id <= 32; ++id) {
         f->detectedObjects[id] = Object(vec(10.0, i * 0.5, id * 0.1));
      }
   }
   auto& tracked = sys.getTrackedObjects();
   for (uint8_t id = 1; id <= 32; ++id) {
      tracked[id] = Object(vec(10.0, 2.0, id * 0.1));
   }

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(tracked);
      brain.CalculateVelocity(sys.getHistoryFrames(), tracked);
      DoNotOptimize(tracked);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(16, "CalculateVelocity (32 Swarm Targets)", ms, N, "frames/sec");
}

// -----------------------------------------------------------------------------
// BM 17: Brain::CalculateAcceleration Savitzky-Golay Computation
// -----------------------------------------------------------------------------
void bm_17_calculate_acceleration_swarm() {
   constexpr uint64_t N = 10'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), i);
      for (uint8_t id = 1; id <= 32; ++id) {
         f->detectedObjects[id] = Object(vec(10.0, i * i * 0.1, id * 0.1));
      }
   }
   auto& tracked = sys.getTrackedObjects();
   for (uint8_t id = 1; id <= 32; ++id) {
      tracked[id] = Object(vec(10.0, 1.6, id * 0.1));
   }

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(tracked);
      brain.CalculateAcceleration(sys.getHistoryFrames(), tracked);
      DoNotOptimize(tracked);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(17, "CalculateAcceleration (32 Targets)", ms, N, "frames/sec");
}

// -----------------------------------------------------------------------------
// BM 18: Brain::SmoothKinematics Savitzky-Golay Smoothing Filter
// -----------------------------------------------------------------------------
void bm_18_smooth_kinematics_swarm() {
   constexpr uint64_t N = 10'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), i);
      for (uint8_t id = 1; id <= 32; ++id) {
         f->detectedObjects[id] = Object(vec(10.0 + (i % 2) * 0.1, i * 0.5, 0));
      }
   }
   auto& tracked = sys.getTrackedObjects();
   for (uint8_t id = 1; id <= 32; ++id) {
      tracked[id] = Object(vec(10.0, 2.0, 0));
   }

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(tracked);
      brain.SmoothKinematics(tracked, sys.getHistoryFrames());
      DoNotOptimize(tracked);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(18, "SmoothKinematics (32 Targets)", ms, N, "frames/sec");
}

// -----------------------------------------------------------------------------
// BM 19: Brain::pickBestTarget Selection (16 Hostiles)
// -----------------------------------------------------------------------------
void bm_19_pick_best_target_16() {
   constexpr uint64_t N = 50'000;
   Brain brain;
   std::unordered_map<uint8_t, Object> targets;
   for (uint8_t i = 1; i <= 16; ++i) {
      Object o(vec(5.0 + i * 0.5, i * 0.2, 0), vec(-1, 0, 0), vec(0, 0, 0), false);
      o.ID = i;
      o.isAlive = true;
      targets[i] = o;
   }
   vec cam(0, 0, 0);
   Target t = nullptr;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(targets);
      t = brain.pickBestTarget(targets, cam);
      DoNotOptimize(t);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)t;
   print_bm(19, "pickBestTarget Ranking (16 Targets)", ms, N);
}

// -----------------------------------------------------------------------------
// BM 20: Brain::pickBestTarget Selection (32 Hostiles)
// -----------------------------------------------------------------------------
void bm_20_pick_best_target_32() {
   constexpr uint64_t N = 50'000;
   Brain brain;
   std::unordered_map<uint8_t, Object> targets;
   for (uint8_t i = 1; i <= 32; ++i) {
      Object o(vec(4.0 + i * 0.4, i * 0.1, 0), vec(-1, 0, 0), vec(0, 0, 0), false);
      o.ID = i;
      o.isAlive = true;
      targets[i] = o;
   }
   vec cam(0, 0, 0);
   Target t = nullptr;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(targets);
      t = brain.pickBestTarget(targets, cam);
      DoNotOptimize(t);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)t;
   print_bm(20, "pickBestTarget Ranking (32 Targets)", ms, N);
}

// -----------------------------------------------------------------------------
// BM 21: System::pushFrame Ring Recycling Throughput
// -----------------------------------------------------------------------------
void bm_21_push_frame_recycling() {
   constexpr uint64_t N = 200'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), i);
      DoNotOptimize(f);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(21, "System::pushFrame Recycling Rate", ms, N, "frames/sec");
}

// -----------------------------------------------------------------------------
// BM 22: System::updateKinematics End-to-End Pipeline
// -----------------------------------------------------------------------------
void bm_22_update_kinematics_pipeline() {
   constexpr uint64_t N = 10'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int i = 0; i < 5; ++i) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), i);
      f->detectedObjects[1] = Object(vec(10, i * 0.2, 0));
   }

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.updateKinematics();
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(22, "System::updateKinematics End-to-End", ms, N, "frames/sec");
}

// -----------------------------------------------------------------------------
// BM 23: System::executeSignal Dispatch Speed
// -----------------------------------------------------------------------------
void bm_23_execute_signal_speed() {
   constexpr uint64_t N = 1'000'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   sig s(SIGNALS::NO_SIG);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(s);
      sys.executeSignal(s);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(23, "System::executeSignal Dispatch", ms, N);
}

// -----------------------------------------------------------------------------
// BM 24: System::tick Complete Update Loop (Full Pipeline)
// -----------------------------------------------------------------------------
void bm_24_system_tick_full_loop() {
   constexpr uint64_t N = 20'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   ImageFrame* f = sys.pushFrame(vec(0, 0, 0), 1);
   Object enemy(vec(10, 0, 0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 1;
   enemy.isAlive = true;
   f->detectedObjects[1] = enemy;

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.01f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(24, "System::tick End-to-End Loop Rate", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// BM 25: High-Density Swarm Simulation (32 Targets Active Tick)
// -----------------------------------------------------------------------------
void bm_25_high_density_swarm_tick() {
   constexpr uint64_t N = 10'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int fIdx = 0; fIdx < 5; ++fIdx) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), fIdx);
      for (uint8_t id = 1; id <= 32; ++id) {
         Object o(vec(8.0 + id * 0.3, id * 0.1, 0), vec(-1, 0, 0), vec(0, 0, 0), (id % 4 == 0));
         o.ID = id;
         o.isAlive = true;
         f->detectedObjects[id] = o;
      }
   }

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.01f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(25, "32-Target Swarm Full Tick Rate", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// BM 26: Vector3D Magnitude Throughput
// -----------------------------------------------------------------------------
void bm_26_vector_magnitude() {
   constexpr uint64_t N = 10'000'000;
   vec v(3.0, 4.0, 12.0);
   double sum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(v);
      sum += v.magnitude();
      DoNotOptimize(sum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)sum;
   print_bm(26, "Vector3D Magnitude Throughput", ms, N);
}

// -----------------------------------------------------------------------------
// BM 27: Vector3D Euclidean Distance (distanceTo)
// -----------------------------------------------------------------------------
void bm_27_vector_distance_to() {
   constexpr uint64_t N = 10'000'000;
   vec v1(1.0, 2.0, 3.0), v2(7.0, 10.0, 15.0);
   double sum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(v1);
      sum += v1.distanceTo(v2);
      DoNotOptimize(sum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)sum;
   print_bm(27, "Vector3D distanceTo Euclidean Math", ms, N);
}

// -----------------------------------------------------------------------------
// BM 28: Vector3D 3D Angle Calculation (angleTo)
// -----------------------------------------------------------------------------
void bm_28_vector_angle_to() {
   constexpr uint64_t N = 5'000'000;
   vec v1(1.0, 0.0, 0.0), v2(0.7071, 0.7071, 0.0);
   double sum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(v1);
      sum += v1.angleTo(v2);
      DoNotOptimize(sum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)sum;
   print_bm(28, "Vector3D 3D angleTo Arc-Cosine", ms, N);
}

// -----------------------------------------------------------------------------
// BM 29: Point-to-Ray Line of Fire Distance
// -----------------------------------------------------------------------------
void bm_29_point_to_ray_distance() {
   constexpr uint64_t N = 5'000'000;
   vec laserOrigin(0, 0, 0);
   vec beamDir(10.0, 0.0, 0.0);
   double beamLengthSq = beamDir * beamDir;
   vec friendlyPos(5.0, 0.3, 0.0);
   double distAccum = 0.0;

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(friendlyPos);
      vec toFriendly = friendlyPos - laserOrigin;
      double t = (toFriendly * beamDir) / beamLengthSq;
      t = std::clamp(t, 0.0, 1.0);
      vec closestPoint = laserOrigin + (beamDir * t);
      distAccum += friendlyPos.distanceTo(closestPoint);
      DoNotOptimize(distAccum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)distAccum;
   print_bm(29, "Point-to-Ray Collision Math (isShotSafe)", ms, N);
}

// -----------------------------------------------------------------------------
// BM 30: Savitzky-Golay Quadratic Kernel Direct Evaluation
// -----------------------------------------------------------------------------
void bm_30_savitzky_golay_kernel() {
   constexpr uint64_t N = 5'000'000;
   vec p0(1.0, 2.0, 3.0), p1(1.1, 2.1, 3.1), p2(1.2, 2.3, 3.2), p3(1.4, 2.5, 3.4), p4(1.6, 2.7, 3.6);
   vec smoothed;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(p0);
      smoothed = (p0 * -3.0 + p1 * -2.0 + p2 * 3.0 + p3 * 12.0 + p4 * 25.0) / 35.0;
      DoNotOptimize(smoothed);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)smoothed;
   print_bm(30, "Savitzky-Golay 5-Pt Smoothing Kernel", ms, N);
}

// -----------------------------------------------------------------------------
// BM 31: Multi-Point Polynomial Trajectory Extrapolation
// -----------------------------------------------------------------------------
void bm_31_multi_point_extrapolation() {
   constexpr uint64_t N = 50'000;
   Brain brain;
   std::vector<Object> targets;
   targets.reserve(32);
   for (uint8_t i = 0; i < 32; ++i) {
      targets.emplace_back(vec(10.0 + i * 0.1, i * 0.2, 1.0), vec(1.0, -0.5, 0.2), vec(0.0, 0.2, -0.1));
   }
   vec futureAccum;
   auto t0 = Clock::now();
   for (uint64_t iter = 0; iter < N; ++iter) {
      for (auto& tgt : targets) {
         DoNotOptimize(tgt);
         for (int step = 1; step <= 10; ++step) {
            futureAccum += brain.PredictPos(tgt, step * 0.1);
            DoNotOptimize(futureAccum);
         }
      }
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)futureAccum;
   print_bm(31, "Multi-Step Trajectory Extrapolations", ms, N * 32 * 10, "predictions/sec");
}

// -----------------------------------------------------------------------------
// BM 32: Iterative Interception Point Solver Convergence
// -----------------------------------------------------------------------------
void bm_32_interception_solver_convergence() {
   constexpr uint64_t N = 500'000;
   Brain brain;
   vec cameraPos(0, 0, 0);
   Object fastEnemy(vec(15.0, -8.0, 2.0), vec(-5.0, 8.0, 0.5), vec(0.5, -0.2, 0.0), false);
   fastEnemy.ID = 1;
   fastEnemy.isAlive = true;

   vec intercept;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(fastEnemy);
      intercept = brain.CalculateInterceptionPoint(fastEnemy, cameraPos);
      DoNotOptimize(intercept);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)intercept;
   print_bm(32, "Interception Point Convergence Solver", ms, N);
}

// -----------------------------------------------------------------------------
// BM 33: Batch Threat Level Evaluation (64 Targets)
// -----------------------------------------------------------------------------
void bm_33_batch_threat_evaluation_64() {
   constexpr uint64_t N = 50'000;
   Brain brain;
   vec cameraPos(0, 0, 0);
   std::vector<Object> swarm;
   swarm.reserve(64);
   for (uint8_t i = 1; i <= 64; ++i) {
      Object o(vec(5.0 + i * 0.2, i * 0.1, 0.0), vec(-1.0, 0.5, 0.0), vec(0, 0, 0), false);
      o.ID = i;
      o.isAlive = true;
      o.isUrgent = (i % 8 == 0);
      swarm.push_back(o);
   }

   double scoreSum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t iter = 0; iter < N; ++iter) {
      for (auto& tgt : swarm) {
         DoNotOptimize(tgt);
         scoreSum += brain.EvaluateThreatLevel(tgt, cameraPos);
         DoNotOptimize(scoreSum);
      }
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)scoreSum;
   print_bm(33, "Batch Threat Evaluation (64 Targets)", ms, N * 64, "evals/sec");
}

// -----------------------------------------------------------------------------
// BM 34: 16-Friendly Obstacle Collision Safety Check
// -----------------------------------------------------------------------------
void bm_34_is_shot_safe_16_friendlies() {
   constexpr uint64_t N = 100'000;
   Brain brain;
   vec cameraPos(0, 0, 0);
   vec targetPos(15.0, 0.0, 0.0);
   std::unordered_map<uint8_t, Object> friendlies;
   for (uint8_t i = 1; i <= 16; ++i) {
      Object f(vec(2.0 + i * 0.5, 2.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      f.ID = i;
      f.isAlive = true;
      friendlies[i] = f;
   }

   bool safe = true;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(targetPos);
      safe &= brain.isShotSafe(targetPos, MAX_LASER_RADIUS, friendlies, cameraPos);
      DoNotOptimize(safe);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)safe;
   print_bm(34, "isShotSafe Collision Check (16 Friendlies)", ms, N);
}

// -----------------------------------------------------------------------------
// BM 35: 64-Friendly Obstacle Collision Safety Check
// -----------------------------------------------------------------------------
void bm_35_is_shot_safe_64_friendlies() {
   constexpr uint64_t N = 50'000;
   Brain brain;
   vec cameraPos(0, 0, 0);
   vec targetPos(18.0, 0.0, 0.0);
   std::unordered_map<uint8_t, Object> friendlies;
   for (uint8_t i = 1; i <= 64; ++i) {
      Object f(vec(2.0 + (i % 16) * 0.8, 1.5 + (i / 16.0) * 0.5, 0.0), vec(0, 0, 0), vec(0, 0, 0), true);
      f.ID = i;
      f.isAlive = true;
      friendlies[i] = f;
   }

   bool safe = true;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(targetPos);
      safe &= brain.isShotSafe(targetPos, MAX_LASER_RADIUS, friendlies, cameraPos);
      DoNotOptimize(safe);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)safe;
   print_bm(35, "isShotSafe Collision Check (64 Friendlies)", ms, N);
}

// -----------------------------------------------------------------------------
// BM 36: FlatObjectMap Sequential Iteration Throughput
// -----------------------------------------------------------------------------
void bm_36_flat_object_map_iteration() {
   constexpr uint64_t N = 500'000;
   FlatObjectMap<32> map;
   for (uint8_t i = 1; i <= 32; ++i) {
      Object o(vec(i, i, i));
      o.ID = i;
      map.insert_or_assign(i, o);
   }

   double sumX = 0.0;
   auto t0 = Clock::now();
   for (uint64_t iter = 0; iter < N; ++iter) {
      DoNotOptimize(map);
      for (const auto& [id, obj] : map) {
         sumX += obj.pos.x;
         DoNotOptimize(sumX);
      }
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)sumX;
   print_bm(36, "FlatObjectMap 32-Item Sequential Iteration", ms, N * 32, "items/sec");
}

// -----------------------------------------------------------------------------
// BM 37: 64-Target Frame Insertion Churn
// -----------------------------------------------------------------------------
void bm_37_frame_insertion_churn_64() {
   constexpr uint64_t N = 50'000;
   auto t0 = Clock::now();
   for (uint64_t iter = 0; iter < N; ++iter) {
      ImageFrame f(vec(0, 0, 0), iter);
      for (uint8_t id = 1; id <= 64; ++id) {
         f.detectedObjects[id] = Object(vec(id, 0, 0));
      }
      DoNotOptimize(f);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(37, "64-Target ImageFrame Insertion Churn", ms, N, "frames/sec");
}

// -----------------------------------------------------------------------------
// BM 38: RingBuffer Circular Overwriting Under Load
// -----------------------------------------------------------------------------
void bm_38_ring_buffer_overwrite_churn() {
   constexpr uint64_t N = 5'000'000;
   RingBuffer<sig, 32> buffer;
   sig testSig(SIGNALS::CAM_ON_TARGET);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      if (buffer.full()) {
         DoNotOptimize(buffer.front());
         buffer.pop();
      }
      buffer.push(testSig);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(38, "RingBuffer Circular Overwrite Churn", ms, N);
}

// -----------------------------------------------------------------------------
// BM 39: Engine Kinematic Physics Integration
// -----------------------------------------------------------------------------
void bm_39_engine_physics_integration() {
   constexpr uint64_t N = 10'000'000;
   Engine engine;
   vec targetVel(15.0, 0.0, 0.0);
   vec targetAcc(1.0, 0.5, -0.2);
   engine.move(targetVel, targetAcc);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(engine);
      engine.updateEngine(0.001f);
      DoNotOptimize(engine.velocity);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(39, "Engine::updateEngine Kinematic Integration", ms, N);
}

// -----------------------------------------------------------------------------
// BM 40: Camera Frame Pipeline Ingestion
// -----------------------------------------------------------------------------
void bm_40_camera_pipeline_ingestion() {
   constexpr uint64_t N = 10'000'000;
   Camera cam;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(cam);
      cam.updateCamera(0.016f);
      DoNotOptimize(cam);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(40, "Camera::updateCamera Pipeline Ingestion", ms, N);
}

// -----------------------------------------------------------------------------
// BM 41: End-to-End System::tick (8 Targets)
// -----------------------------------------------------------------------------
void bm_41_system_tick_8_targets() {
   constexpr uint64_t N = 20'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int fIdx = 0; fIdx < 5; ++fIdx) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), fIdx);
      for (uint8_t id = 1; id <= 8; ++id) {
         Object o(vec(6.0 + id * 0.5, id * 0.2, 0), vec(-0.5, 0, 0), vec(0, 0, 0), false);
         o.ID = id;
         o.isAlive = true;
         f->detectedObjects[id] = o;
      }
   }
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.01f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(41, "System::tick Rate (8 Targets)", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// BM 42: End-to-End System::tick (16 Targets)
// -----------------------------------------------------------------------------
void bm_42_system_tick_16_targets() {
   constexpr uint64_t N = 15'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int fIdx = 0; fIdx < 5; ++fIdx) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), fIdx);
      for (uint8_t id = 1; id <= 16; ++id) {
         Object o(vec(6.0 + id * 0.4, id * 0.1, 0), vec(-0.5, 0, 0), vec(0, 0, 0), (id % 4 == 0));
         o.ID = id;
         o.isAlive = true;
         f->detectedObjects[id] = o;
      }
   }
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.01f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(42, "System::tick Rate (16 Targets)", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// BM 43: End-to-End System::tick (64 Targets - Stress)
// -----------------------------------------------------------------------------
void bm_43_system_tick_64_targets() {
   constexpr uint64_t N = 5'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int fIdx = 0; fIdx < 5; ++fIdx) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), fIdx);
      for (uint8_t id = 1; id <= 64; ++id) {
         double angle = (id * 5.5) * M_PI / 180.0;
         Object o(vec(5.0 * std::cos(angle), 5.0 * std::sin(angle), 0), vec(-0.2, 0, 0), vec(0, 0, 0), (id % 5 == 0));
         o.ID = id;
         o.isAlive = true;
         f->detectedObjects[id] = o;
      }
   }
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.01f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(43, "System::tick Rate (64 Targets Stress)", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// BM 44: End-to-End System::tick (128 Targets - Extreme Stress)
// -----------------------------------------------------------------------------
void bm_44_system_tick_128_targets() {
   constexpr uint64_t N = 2'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   for (int fIdx = 0; fIdx < 5; ++fIdx) {
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), fIdx);
      for (uint8_t id = 1; id <= 128; ++id) {
         double angle = (id * 2.8) * M_PI / 180.0;
         Object o(vec(6.0 * std::cos(angle), 6.0 * std::sin(angle), 0), vec(-0.1, 0, 0), vec(0, 0, 0), (id % 6 == 0));
         o.ID = id;
         o.isAlive = true;
         f->detectedObjects[id] = o;
      }
   }
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.01f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(44, "System::tick Rate (128 Targets Extreme)", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// BM 45: Communicator Multi-Priority Storm (500k Signals)
// -----------------------------------------------------------------------------
void bm_45_communicator_signal_storm() {
   constexpr uint64_t N = 500'000;
   CommunicatorDevice comm;
   sig crit(SIGNALS::SYSTEM_SHUTDOWN, std::monostate{}, Priority::CRITICAL);
   sig norm(SIGNALS::OBJECT_DETECTED, std::monostate{}, Priority::NORMAL);

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      comm.Receive((i % 2 == 0) ? crit : norm);
      auto val = comm.popSignal();
      DoNotOptimize(val);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(45, "Communicator 500k Signal Storm Throughput", ms, N);
}

// -----------------------------------------------------------------------------
// BM 46: Extractor Binary Packet Deserialization Throughput
// -----------------------------------------------------------------------------
static constexpr char TEST_PIPE_BM_46[] = "./yazdan-backend/build/bm46_packet.bin";
void bm_46_extractor_throughput() {
   constexpr uint64_t N = 100'000;
   Object testObj(vec(10.0, 0.5, 2.0), vec(1.0, 0.0, 0.0), vec(0, 0, 0), false);
   testObj.ID = 99;
   testObj.isAlive = true;
   {
      std::ofstream out(TEST_PIPE_BM_46, std::ios::out | std::ios::binary | std::ios::trunc);
      for (uint64_t i = 0; i < N; ++i) {
         out.write(reinterpret_cast<const char*>(&testObj), sizeof(Object));
      }
   }

   Extractor<TEST_PIPE_BM_46, Object> extractor;
   Object dummy;
   uint64_t readCount = 0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      if (extractor.extract(dummy)) {
         readCount++;
         DoNotOptimize(dummy);
      }
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(46, "Extractor Binary Packet Ingestion", ms, readCount, "packets/sec");
}

// -----------------------------------------------------------------------------
// BM 47: Logger Formatted Signal Emission Speed
// -----------------------------------------------------------------------------
void bm_47_logger_formatting_speed() {
   constexpr uint64_t N = 100'000;
   Logger logger("./yazdan-backend/logs.txt");
   sig s(SIGNALS::CAM_ON_TARGET);
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      logger.logSignal(s);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(47, "Logger::logSignal Emission Rate", ms, N, "logs/sec");
}

// -----------------------------------------------------------------------------
// BM 48: Shortest-Path Yaw Wrap-Around Normalizer
// -----------------------------------------------------------------------------
void bm_48_yaw_wrap_around_speed() {
   constexpr uint64_t N = 10'000'000;
   double yaw = 350.0;
   double deltaYawAccum = 0.0;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(yaw);
      double delta = yaw - 10.0;
      while (delta > 180.0)
         delta -= 360.0;
      while (delta < -180.0)
         delta += 360.0;
      deltaYawAccum += delta;
      DoNotOptimize(deltaYawAccum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)deltaYawAccum;
   print_bm(48, "Shortest-Path Yaw Wrap Normalization", ms, N);
}

// -----------------------------------------------------------------------------
// BM 49: Fallback Finite Difference Velocity Estimator
// -----------------------------------------------------------------------------
void bm_49_finite_difference_fallback() {
   constexpr uint64_t N = 5'000'000;
   vec posPrev(10.0, 2.0, 1.0), posCurr(10.0, 2.2, 1.0);
   double dt = 0.016;
   vec velAccum;
   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      DoNotOptimize(posCurr);
      velAccum += (posCurr - posPrev) / dt;
      DoNotOptimize(velAccum);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   (void)velAccum;
   print_bm(49, "Finite Difference Velocity Fallback", ms, N);
}

// -----------------------------------------------------------------------------
// BM 50: Sustained 50,000-Tick Continuous Simulation
// -----------------------------------------------------------------------------
void bm_50_sustained_simulation_throughput() {
   constexpr uint64_t N = 50'000;
   Camera cam;
   Engine l, r, p;
   Laser laser;
   Brain brain;
   Logger log("./yazdan-backend/logs.txt");
   System sys(cam, l, r, p, laser, brain, log);

   ImageFrame* f = sys.pushFrame(vec(0, 0, 0), 0);
   Object enemy(vec(8.0, 0.0, 0.0), vec(0, 0, 0), vec(0, 0, 0), false);
   enemy.ID = 1;
   enemy.isAlive = true;
   f->detectedObjects[1] = enemy;

   auto t0 = Clock::now();
   for (uint64_t i = 0; i < N; ++i) {
      sys.tick(0.016f);
      DoNotOptimize(sys);
   }
   auto t1 = Clock::now();
   double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
   print_bm(50, "Sustained Simulation (50k Ticks)", ms, N, "ticks/sec");
}

// -----------------------------------------------------------------------------
// Main Benchmark Runner
// -----------------------------------------------------------------------------
int main() {
   std::cout << "\n==========================================================================================" << std::endl;
   std::cout << "                        YAZDAN BACKEND BENCHMARK SUITE (50 BENCHMARKS)                    " << std::endl;
   std::cout << "==========================================================================================" << std::endl;

   bm_01_vector_arithmetic();
   bm_02_vector_dot_cross();
   bm_03_vector_angles();
   bm_04_cylindrical_to_cartesian();
   bm_05_cartesian_to_cylindrical();
   bm_06_ring_buffer_push_pop();
   bm_07_communicator_priority_queue();
   bm_08_flat_object_map_lookup();
   bm_09_flat_object_map_insert_erase();
   bm_10_map_comparison();
   bm_11_predict_pos();
   bm_12_calculate_time_to_aim();
   bm_13_calculate_interception_point();
   bm_14_evaluate_threat_level();
   bm_15_is_shot_safe();
   bm_16_calculate_velocity_swarm();
   bm_17_calculate_acceleration_swarm();
   bm_18_smooth_kinematics_swarm();
   bm_19_pick_best_target_16();
   bm_20_pick_best_target_32();
   bm_21_push_frame_recycling();
   bm_22_update_kinematics_pipeline();
   bm_23_execute_signal_speed();
   bm_24_system_tick_full_loop();
   bm_25_high_density_swarm_tick();
   bm_26_vector_magnitude();
   bm_27_vector_distance_to();
   bm_28_vector_angle_to();
   bm_29_point_to_ray_distance();
   bm_30_savitzky_golay_kernel();
   bm_31_multi_point_extrapolation();
   bm_32_interception_solver_convergence();
   bm_33_batch_threat_evaluation_64();
   bm_34_is_shot_safe_16_friendlies();
   bm_35_is_shot_safe_64_friendlies();
   bm_36_flat_object_map_iteration();
   bm_37_frame_insertion_churn_64();
   bm_38_ring_buffer_overwrite_churn();
   bm_39_engine_physics_integration();
   bm_40_camera_pipeline_ingestion();
   bm_41_system_tick_8_targets();
   bm_42_system_tick_16_targets();
   bm_43_system_tick_64_targets();
   bm_44_system_tick_128_targets();
   bm_45_communicator_signal_storm();
   bm_46_extractor_throughput();
   bm_47_logger_formatting_speed();
   bm_48_yaw_wrap_around_speed();
   bm_49_finite_difference_fallback();
   bm_50_sustained_simulation_throughput();

   std::cout << "==========================================================================================" << std::endl;
   std::cout << "                        ALL 50 BENCHMARKS COMPLETED                                       " << std::endl;
   std::cout << "==========================================================================================\n"
             << std::endl;
   return 0;
}
