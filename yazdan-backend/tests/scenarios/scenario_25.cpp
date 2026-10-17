#include "scenario_common.hpp"

// =============================================================================
// Scenario 25: Signal Logging Audit Trail
// =============================================================================
static void test() {
   // Dedicated bounded log file for Scenario 25 audit
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
}

int main() {
   return run_scenario("25", "Signal Logging Audit Trail", test);
}
