#include "../include/ardunio.hpp"
#include "../include/constants.hpp"
#include "../include/logger.hpp"
#include "../include/physichs.hpp"
#include <algorithm>
#include <iostream>

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
   os << "Engine [Active: " << (engine.isActive ? "YES" : "NO") << ", Vel: " << engine.velocity
      << ", Acc: " << engine.acceleration << "]";
   return os;
}

// --- System Implementation ---
System::System(Camera& camera, Engine& engineLeftYaw, Engine& engineRightYaw, Engine& enginePitch, Laser& laser, Brain& brain) : camera(camera), engineLeftYaw(engineLeftYaw), engineRightYaw(engineRightYaw), enginePitch(enginePitch), laser(laser), brain(brain), yawAngle(0.0), pitchAngle(0.0), isShutDown(false), frameCount(0) {
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
   camera.reset();
   engineLeftYaw.reset();
   engineRightYaw.reset();
   enginePitch.reset();
}

void System::executeSignal(sig signal) {
   handleSystemSignal(signal);
   handleCameraSignal(signal);
   handleEngineSignal(signal);
   handleLaserSignal(signal);
   handleObjectSignal(signal);
}

void System::handleCameraSignal([[maybe_unused]] sig signal) {
}
void System::handleEngineSignal([[maybe_unused]] sig signal) {
}
void System::handleLaserSignal([[maybe_unused]] sig signal) {
}
void System::handleSystemSignal(sig signal) {
   if (signal.signal == SIGNALS::SYSTEM_SHUTDOWN) {
      shutdown();
   }
   else if (signal.signal == SIGNALS::SYSTEM_RESTART) {
      reset();
   }
}
void System::handleObjectSignal([[maybe_unused]] sig signal) {
}

std::ostream& operator<<(std::ostream& os, const System& system) {
   os << "System [ShutDown: " << (system.isShutDown ? "YES" : "NO")
      << ", Yaw: " << system.yawAngle << ", Pitch: " << system.pitchAngle
      << ", Tracked Objects: " << system.trackedObjects.size() << "]";
   return os;
}
