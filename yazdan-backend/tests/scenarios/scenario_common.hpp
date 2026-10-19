#pragma once
#include "../../include/ardunio.hpp"
#include "../../include/logger.hpp"
#include "../../include/logic.hpp"
#include "../../include/physichs.hpp"
#include <cassert>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/resource.h>
#include <unistd.h>

// Common test harness rig
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

inline long getCurrentRssKb() {
   std::ifstream statm("/proc/self/statm");
   long size = 0, resident = 0;
   if (statm >> size >> resident) {
      long pageSizeKb = sysconf(_SC_PAGESIZE) / 1024;
      return resident * pageSizeKb;
   }
   return 0;
}

inline long getPeakRssKb() {
   struct rusage usage;
   getrusage(RUSAGE_SELF, &usage);
   return usage.ru_maxrss;
}

template <typename Func>
inline int run_scenario(const std::string& id, const std::string& name, Func&& testFunc) {
   std::cout << "[Scenario " << id << "] " << name << ": ";
   std::cout.flush();

   struct timespec startWall, endWall;
   struct timespec startCpu, endCpu;

   clock_gettime(CLOCK_MONOTONIC_RAW, &startWall);
   clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startCpu);

   testFunc();

   clock_gettime(CLOCK_THREAD_CPUTIME_ID, &endCpu);
   clock_gettime(CLOCK_MONOTONIC_RAW, &endWall);

   double wallMs = (endWall.tv_sec - startWall.tv_sec) * 1000.0 + (endWall.tv_nsec - startWall.tv_nsec) * 1e-6;
   double cpuMs = (endCpu.tv_sec - startCpu.tv_sec) * 1000.0 + (endCpu.tv_nsec - startCpu.tv_nsec) * 1e-6;
   long currentRss = getCurrentRssKb();
   long peakRss = getPeakRssKb();

   std::cout << "PASSED" << std::endl;
   std::cout << "   [Resource Usage] Wall: " << std::fixed << std::setprecision(3) << wallMs << " ms"
             << " | CPU: " << std::setprecision(3) << cpuMs << " ms"
             << " | Current RSS: " << std::setprecision(1) << (currentRss / 1024.0) << " MB"
             << " (Peak: " << std::setprecision(1) << (peakRss / 1024.0) << " MB)" << std::endl;
   return 0;
}
