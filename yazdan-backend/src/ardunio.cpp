#include "../include/ardunio.hpp"
#include "../include/constants.hpp"
#include "../include/logger.hpp"
#include "../include/physichs.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// --- ImageFrame Stream Operator ---
std::ostream& operator<<(std::ostream& os, const ImageFrame& frame) {
   os << "ImageFrame [Timestamp: " << frame.timestamp << ", Center: " << frame.centerPos
      << ", Objects Count: " << frame.detectedObjects.size() << "]";
   return os;
}

// --- Camera Implementation ---
void Camera::calibrate(vec newPos) {
   CamCenterPos = newPos;
}

void Camera::reset() {
   CamCenterPos = vec(0, 0, 0);
}

void Camera::updateCamera([[maybe_unused]] float deltaTime) {
   // Camera position update logic based on movement
}

std::ostream& operator<<(std::ostream& os, const Camera& cam) {
   os << "Camera [Center: " << cam.CamCenterPos << ", Width: " << cam.frame_width
      << ", Height: " << cam.frame_height << "]";
   return os;
}

// --- Laser Implementation ---
void Laser::turnOn() {
   isOn = true;
}

void Laser::turnOff() {
   isOn = false;
}

std::ostream& operator<<(std::ostream& os, const Laser& laser) {
   os << "Laser [State: " << (laser.isOn ? "ON" : "OFF") << ", Pos: " << laser.laserPos << "]";
   return os;
}

// --- Engine Implementation ---
void Engine::move(vec vel, vec acc) {
   velocity = vel;
   acceleration = acc;
   isActive = true;
}

void Engine::stop() {
   velocity = vec(0, 0, 0);
   acceleration = vec(0, 0, 0);
   isActive = false;
}

void Engine::calibrate(vec vel, vec acc) {
   velocity = vel;
   acceleration = acc;
}

void Engine::reset() {
   stop();
}

void Engine::updateEngine(float deltaTime) {
   if (isActive) {
      velocity += acceleration * deltaTime;
   }
}

std::ostream& operator<<(std::ostream& os, const Engine& engine) {
   os << "Engine [Active: " << (engine.isActive ? "YES" : "NO") << ", Vel: " << engine.velocity << ", Acc: " << engine.acceleration << "]";
   return os;
}

// --- System Implementation ---
System::System(Camera& camera, Engine& engineLeftYaw, Engine& engineRightYaw, Engine& enginePitch, Laser& laser, Brain& brain, Logger& logger) : camera(camera), engineLeftYaw(engineLeftYaw), engineRightYaw(engineRightYaw), enginePitch(enginePitch), laser(laser), brain(brain), logger(logger), yawAngle(0.0), pitchAngle(0.0), isShutDown(false), frameCount(0) {
   std::fill(std::begin(HistoryFrames), std::end(HistoryFrames), nullptr);
}

ImageFrame* System::pushFrame(vec centerPos, uint64_t timestamp) {
   ImageFrame* targetFrame = nullptr;
   if (frameCount < HISTORY_FRAME_COUNTS) {
      targetFrame = &framePool[frameCount];
      HistoryFrames[frameCount] = targetFrame;
      frameCount++;
   }
   else {
      // Recycle the oldest frame pointer (circular ring buffer)
      targetFrame = HistoryFrames[0];
      for (size_t i = 0; i < HISTORY_FRAME_COUNTS - 1; ++i) {
         HistoryFrames[i] = HistoryFrames[i + 1];
      }
      HistoryFrames[HISTORY_FRAME_COUNTS - 1] = targetFrame;
   }
   targetFrame->centerPos = centerPos;
   targetFrame->timestamp = timestamp;
   targetFrame->detectedObjects.clear(); // clears entries without deallocating bucket memory
   return targetFrame;
}

void System::updateKinematics() {
   if (frameCount > 0) {
      size_t latestIdx = (frameCount <= HISTORY_FRAME_COUNTS) ? (frameCount - 1) : (HISTORY_FRAME_COUNTS - 1);
      if (HistoryFrames[latestIdx]) {
         const auto& latestDetections = HistoryFrames[latestIdx]->detectedObjects;
         for (const auto& [id, obj] : latestDetections) {
            if (destroyedTargets.count(id)) {
               continue; // Zombie balloon prevention: do not resurrect destroyed targets
            }
            auto it = trackedObjects.find(id);
            if (it != trackedObjects.end()) {
               double prevTimeUnderFire = it->second.timeUnderFire;
               it->second = obj;
               it->second.timeUnderFire = prevTimeUnderFire;
               it->second.staleCount = 0;
            }
            else {
               trackedObjects.emplace(id, obj);
            }
         }

         // Target aging: prune ghost targets not seen for MAX_STALE_FRAMES
         std::vector<uint8_t> lostIDs;
         for (auto& [id, obj] : trackedObjects) {
            if (!latestDetections.count(id)) {
               obj.staleCount++;
               if (obj.staleCount >= MAX_STALE_FRAMES) {
                  lostIDs.push_back(id);
               }
            }
         }
         for (uint8_t lostId : lostIDs) {
            trackedObjects.erase(lostId);
            executeSignal(sig(SIGNALS::OBJECT_LOST));
         }
      }
   }
   brain.CalculateVelocity(HistoryFrames, trackedObjects);
   brain.CalculateAcceleration(HistoryFrames, trackedObjects);
   brain.SmoothKinematics(trackedObjects, HistoryFrames);
}

std::unordered_map<uint8_t, Object>& System::getTrackedObjects() {
   return trackedObjects;
}

const std::unordered_map<uint8_t, Object>& System::getTrackedObjects() const {
   return trackedObjects;
}

ImageFrame** System::getHistoryFrames() {
   return HistoryFrames;
}

ImageFrame* const* System::getHistoryFrames() const {
   return HistoryFrames;
}

void System::shutdown() {
   isShutDown = true;
}

void System::startup() {
   isShutDown = false;
}

void System::reset() {
   isShutDown = false;
   frameCount = 0;
   std::fill(std::begin(HistoryFrames), std::end(HistoryFrames), nullptr);
   trackedObjects.clear();
   destroyedTargets.clear();
   camera.reset();
   engineLeftYaw.reset();
   engineRightYaw.reset();
   enginePitch.reset();
}

void System::tick(float deltaTime) {
   if (isShutDown) {
      return;
   }

   // 1. Drain and execute pending signals from the communication channel
   while (auto pendingSignal = COMM_CHANNEL.popSignal()) {
      executeSignal(*pendingSignal);
   }

   // 2. Advance subsystem physics for current deltaTime
   camera.updateCamera(deltaTime);
   engineLeftYaw.updateEngine(deltaTime);
   engineRightYaw.updateEngine(deltaTime);
   enginePitch.updateEngine(deltaTime);

   // Update quarantine timers for destroyed target IDs to allow safe tracker ID recycling
   std::vector<uint8_t> expiredQuarantines;
   for (auto& [id, remainingTime] : destroyedTargets) {
      remainingTime -= deltaTime;
      if (remainingTime <= 0.0) {
         expiredQuarantines.push_back(id);
      }
   }
   for (uint8_t id : expiredQuarantines) {
      destroyedTargets.erase(id);
   }

   // 3. Update kinematics filter (velocity, acceleration, smoothing) on all tracked objects
   updateKinematics();

   // 4. Select the highest threat target using greedy threat-scoring
   obj* targetPtr = brain.pickBestTarget(trackedObjects, camera.CamCenterPos, yawAngle, pitchAngle);
   if (!targetPtr) {
      if (laser.isOn) {
         executeSignal(sig(SIGNALS::LASER_OFF));
      }
      engineLeftYaw.stop();
      engineRightYaw.stop();
      enginePitch.stop();
      return;
   }
   obj& target = *targetPtr;

   // 6. Calculate lead interception point based on target trajectory and turret rotation time
   const vec aimOrigin = camera.CamCenterPos + CAMERA_LASER_OFFSET;
   const vec interceptionPoint = brain.CalculateInterceptionPoint(target, camera.CamCenterPos, yawAngle, pitchAngle);

   // 7. Compute target Yaw and Pitch angles towards the interception point
   const double targetYaw = aimOrigin.YawTo(interceptionPoint);
   const double targetPitch = aimOrigin.PitchTo(interceptionPoint);

   // Calculate angular errors and wrap yaw to [-180, 180] degrees
   double deltaYaw = targetYaw - yawAngle;
   while (deltaYaw > 180.0)
      deltaYaw -= 360.0;
   while (deltaYaw < -180.0)
      deltaYaw += 360.0;
   double deltaPitch = targetPitch - pitchAngle;

   // 8. Command engines towards the target with velocity clamped to maximum limits
   double yawVel = (deltaTime > 1e-6f) ? std::clamp(deltaYaw / deltaTime, -MAX_VELOCUTY_X, MAX_VELOCUTY_X) : 0.0;
   double pitchVel = (deltaTime > 1e-6f) ? std::clamp(deltaPitch / deltaTime, -MAX_VELOCUTY_Y, MAX_VELOCUTY_Y) : 0.0;

   vec yawVelVec(yawVel, 0.0, 0.0);
   vec pitchVelVec(0.0, pitchVel, 0.0);

   engineLeftYaw.move(yawVelVec);
   engineRightYaw.move(yawVelVec);
   enginePitch.move(pitchVelVec);

    // Update turret angular position with wrap and mechanical pitch soft limits
   yawAngle += yawVel * deltaTime;
   while (yawAngle > 180.0)
      yawAngle -= 360.0;
   while (yawAngle < -180.0)
      yawAngle += 360.0;
   pitchAngle = std::clamp(pitchAngle + pitchVel * deltaTime, MIN_PITCH_DEG, MAX_PITCH_DEG);

   // 9. Safety Check: Verify line of fire does not intersect any friendly unit
   bool shotSafe = brain.isShotSafe(interceptionPoint, MAX_LASER_RADIUS, trackedObjects, camera.CamCenterPos);
   if (!shotSafe) {
      executeSignal(sig(SIGNALS::OBJECT_FRIENDLY_FIRE));
      return;
   }

   // 10. Fire Control: Schmitt-Trigger hysteresis to prevent relay chatter
   // Compute live post-motion error to avoid 1-frame fire control latency
   double postDeltaYaw = targetYaw - yawAngle;
   while (postDeltaYaw > 180.0)
      postDeltaYaw -= 360.0;
   while (postDeltaYaw < -180.0)
      postDeltaYaw += 360.0;
   double postDeltaPitch = targetPitch - pitchAngle;

   const double tolerance = laser.isOn ? AIM_HOLD_TOLERANCE_DEG : AIM_ENGAGE_TOLERANCE_DEG;
   if (std::abs(postDeltaYaw) <= tolerance && std::abs(postDeltaPitch) <= tolerance) {
      if (!laser.isOn) {
         executeSignal(sig(SIGNALS::CAM_ON_TARGET));
      }
      target.timeUnderFire += deltaTime;

      // Confirm destruction if target was sustained under fire for required duration
      if (target.timeUnderFire >= TIME_UNDER_FIRE) {
         target.isAlive = false;
         destroyedTargets[target.ID] = ZOMBIE_QUARANTINE_SECONDS;
         executeSignal(sig(SIGNALS::OBJECT_DESTROYED));
      }
   }
   else {
      if (laser.isOn) {
         executeSignal(sig(SIGNALS::LASER_OFF));
      }
   }
}

void System::executeSignal(sig signal) {
   switch (signal.signal) {
   case SIGNALS::SYSTEM_START:
      startup();
      break;

   case SIGNALS::SYSTEM_STOP:
      laser.turnOff();
      engineLeftYaw.stop();
      engineRightYaw.stop();
      enginePitch.stop();
      break;

   case SIGNALS::SYSTEM_RESTART:
      reset();
      startup();
      break;

   case SIGNALS::SYSTEM_SHUTDOWN:
   case SIGNALS::SYSTEM_UNRESPONSIVE:
      laser.turnOff();
      engineLeftYaw.stop();
      engineRightYaw.stop();
      enginePitch.stop();
      shutdown();
      break;

   case SIGNALS::CAM_START:
   case SIGNALS::CAM_STOP:
      break;

   case SIGNALS::CAM_CALIBRATE:
      if (std::holds_alternative<vec>(signal.value)) {
         camera.calibrate(std::get<vec>(signal.value));
      }
      break;

   case SIGNALS::CAM_RESET:
      camera.reset();
      break;

   case SIGNALS::CAM_ON_TARGET:
      laser.turnOn();
      break;

   case SIGNALS::CAM_UNRESPONSIVE:
      laser.turnOff();
      break;

   case SIGNALS::YAW_ENGINE_MOVE:
      if (std::holds_alternative<vec>(signal.value)) {
         engineLeftYaw.move(std::get<vec>(signal.value));
         engineRightYaw.move(std::get<vec>(signal.value));
      }
      break;

   case SIGNALS::YAW_ENGINE_STOP:
   case SIGNALS::YAW_ENGINE_LIMIT_REACHED:
      engineLeftYaw.stop();
      engineRightYaw.stop();
      break;

   case SIGNALS::YAW_ENGINE_CALIBRATE:
      if (std::holds_alternative<vec>(signal.value)) {
         engineLeftYaw.calibrate(std::get<vec>(signal.value));
         engineRightYaw.calibrate(std::get<vec>(signal.value));
      }
      break;

   case SIGNALS::YAW_ENGINE_RESET:
      engineLeftYaw.reset();
      engineRightYaw.reset();
      break;

   case SIGNALS::YAW_ENGINE_UNRESPONSIVE:
      laser.turnOff();
      engineLeftYaw.stop();
      engineRightYaw.stop();
      break;

   case SIGNALS::PITCH_ENGINE_MOVE:
      if (std::holds_alternative<vec>(signal.value)) {
         enginePitch.move(std::get<vec>(signal.value));
      }
      break;

   case SIGNALS::PITCH_ENGINE_STOP:
   case SIGNALS::PITCH_ENGINE_LIMIT_REACHED:
      enginePitch.stop();
      break;

   case SIGNALS::PITCH_ENGINE_CALIBRATE:
      if (std::holds_alternative<vec>(signal.value)) {
         enginePitch.calibrate(std::get<vec>(signal.value));
      }
      break;

   case SIGNALS::PITCH_ENGINE_RESET:
      enginePitch.reset();
      break;

   case SIGNALS::PITCH_ENGINE_UNRESPONSIVE:
      laser.turnOff();
      enginePitch.stop();
      break;

   case SIGNALS::LASER_ON:
      laser.turnOn();
      break;

   case SIGNALS::LASER_OFF:
   case SIGNALS::LASER_UNRESPONSIVE:
   case SIGNALS::OBJECT_DESTROYED:
   case SIGNALS::OBJECT_ESCAPED:
   case SIGNALS::OBJECT_OUT_OF_RANGE:
   case SIGNALS::OBJECT_LOST:
      laser.turnOff();
      break;

   case SIGNALS::OBJECT_FRIENDLY_FIRE:
      laser.turnOff();
      logger.log("CRITICAL: Friendly fire condition detected! Laser disengaged.");
      break;

   case SIGNALS::OBJECT_DETECTED:
   case SIGNALS::OBJECT_TRACKED:
   case SIGNALS::OBJECT_SKIPPED:
   case SIGNALS::NO_SIG:
   default:
      break;
   }

   logger.logSignal(signal);
}

std::ostream& operator<<(std::ostream& os, const System& system) {
   os << "System [ShutDown: " << (system.isShutDown ? "YES" : "NO") << ", Yaw: " << system.yawAngle << ", Pitch: " << system.pitchAngle << ", Tracked Objects: " << system.trackedObjects.size() << "]";
   return os;
}
