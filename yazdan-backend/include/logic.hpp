#pragma once
#include "physichs.hpp"
#include <cstdint>
#include <unordered_map>

using Target = obj*;
struct ImageFrame;

class Brain {
 public:
   Brain() = default;
   ~Brain() = default;

   // Core Kinematics (stateless: operates on passed-in objects and history frames)
   void SmoothKinematics(std::unordered_map<uint8_t, obj>& objects, ImageFrame** HistoryFrames) const;
   void CalculateVelocity(ImageFrame** HistoryFrames, std::unordered_map<uint8_t, obj>& objects) const;
   void CalculateAcceleration(ImageFrame** HistoryFrames, std::unordered_map<uint8_t, obj>& objects) const;

   // Convenience overloads for parameter flexibility
   void CalculateVelocity(std::unordered_map<uint8_t, obj>& objects, ImageFrame** HistoryFrames) const;
   void CalculateAcceleration(std::unordered_map<uint8_t, obj>& objects, ImageFrame** HistoryFrames) const;
   void SmoothKinematics(ImageFrame** HistoryFrames, std::unordered_map<uint8_t, obj>& objects) const;

   // Prediction & Targeting
   vec PredictPos(const obj& target, double timeFuture) const;                                                                                                   // predicts the position of the tracked target at the next frame
   vec CalculateInterceptionPoint(const obj& target, const vec& cameraPos, double currentYaw = 0.0, double currentPitch = 0.0) const;                           // calculates the exact point to intercept the target
   double CalculateTimeToAim(const vec& currentCameraPos, const vec& targetPos, double currentYaw = 0.0, double currentPitch = 0.0) const;                      // returns the shortest time needed to intercept

   // Decision Making
   double EvaluateThreatLevel(const obj& target, const vec& cameraPos, double currentYaw = 0.0, double currentPitch = 0.0) const;                                 // returns the highest threat level for the greedy algorithm
   Target pickBestTarget(std::unordered_map<uint8_t, obj>& objects, const vec& cameraPos, double currentYaw = 0.0, double currentPitch = 0.0);                   // returns the pointer to the best target
   bool isShotSafe(const vec& targetPos, double laserRadius, const std::unordered_map<uint8_t, obj>& allObjects, vec cameraPos);                                // checks if it is safe to shot the tracked target
};