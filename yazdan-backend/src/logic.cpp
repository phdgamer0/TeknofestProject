#include "../include/logic.hpp"
#include "../include/ardunio.hpp"
#include "../include/constants.hpp"
#include "../include/physichs.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

extern const double TIME_UNDER_FIRE;
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
extern const vec CAMERA_LASER_OFFSET;
extern struct ImageFrame* HistoryFrames[];

void Brain::SmoothKinematics(std::unordered_map<uint8_t, obj>& objects) const {
   for (auto& [id, target] : objects) {
      // Collect past positions from HistoryFrames for this object
      std::vector<vec> pastPositions;
      for (uint8_t i = 0; i < HISTORY_FRAME_COUNTS; ++i) {
         if (HistoryFrames[i] != nullptr && HistoryFrames[i]->detectedObjects.count(id)) {
            pastPositions.push_back(HistoryFrames[i]->detectedObjects.at(id).pos);
         }
      }
      if (pastPositions.size() == HISTORY_FRAME_COUNTS) {
         // Savitzky-Golay end-point smoothing filter for 5 points (degree 2 polynomial):
         // Coefficients: [-3, -2, 3, 12, 25] / 35
         vec smoothedPos = (pastPositions[0] * -3.0 + pastPositions[1] * -2.0 + pastPositions[2] * 3.0 + pastPositions[3] * 12.0 + pastPositions[4] * 25.0) / 35.0;
         target.pos = smoothedPos;
      }
      else if (!pastPositions.empty()) {
         // Weighted moving average giving more weight to recent detections
         vec weightedSum(0.0, 0.0, 0.0);
         double totalWeight = 0.0;
         for (size_t i = 0; i < pastPositions.size(); ++i) {
            double weight = static_cast<double>(i + 1);
            weightedSum += pastPositions[i] * weight;
            totalWeight += weight;
         }
         if (totalWeight > 0.0) {
            target.pos = weightedSum / totalWeight;
         }
      }
   }
}

void Brain::CalculateVelocity(ImageFrame** HistoryFrames) const {
   if (HistoryFrames == nullptr) {
      return;
   }
   for (auto& [k, v] : trackedObjects) {
      vec new_vel{0.0, 0.0, 0.0};
      double counted_intervals = 0.0;
      int prevIndex = -1;
      for (uint8_t i = 0U; i < HISTORY_FRAME_COUNTS; ++i) {
         if (HistoryFrames[i] != nullptr && HistoryFrames[i]->detectedObjects.count(k)) {
            if (prevIndex != -1) {
               vec dPos = HistoryFrames[i]->detectedObjects.at(k).pos - HistoryFrames[prevIndex]->detectedObjects.at(k).pos;
               double dt = static_cast<double>(HistoryFrames[i]->timestamp - HistoryFrames[prevIndex]->timestamp);
               if (dt > 1e-6) {
                  new_vel += dPos / dt;
               }
               else {
                  double frameDiff = static_cast<double>(i - prevIndex);
                  new_vel += dPos / frameDiff;
               }
               counted_intervals += 1.0;
            }
            prevIndex = i;
         }
      }
      if (counted_intervals > 0.0) {
         v.vel = new_vel / counted_intervals;
      }
   }
}

void Brain::CalculateAcceleration(ImageFrame** HistoryFrames) const {
   if (HistoryFrames == nullptr) {
      return;
   }
   // Check if all 5 history frames are valid
   bool allFramesValid = true;
   for (uint8_t i = 0; i < HISTORY_FRAME_COUNTS; ++i) {
      if (HistoryFrames[i] == nullptr) {
         allFramesValid = false;
         break;
      }
   }
   double dt = 1.0;
   double deltaTimeSquared = 1.0;
   if (allFramesValid) {
      double totalTime = static_cast<double>(HistoryFrames[HISTORY_FRAME_COUNTS - 1]->timestamp - HistoryFrames[0]->timestamp);
      dt = totalTime / static_cast<double>(HISTORY_FRAME_COUNTS - 1);
      deltaTimeSquared = (dt > 1e-6) ? (dt * dt) : 1.0;
   }
   for (auto& [k, v] : trackedObjects) {
      // Check if object is present across all history frames for Savitzky-Golay
      bool inAllFrames = allFramesValid;
      if (allFramesValid) {
         for (uint8_t i = 0; i < HISTORY_FRAME_COUNTS; ++i) {
            if (!HistoryFrames[i]->detectedObjects.count(k)) {
               inAllFrames = false;
               break;
            }
         }
      }
      if (inAllFrames) {
         // Savitzky-Golay 2nd derivative filter (5-point quadratic fit):
         // acc = (2*P0 - P1 - 2*P2 - P3 + 2*P4) / (7 * dt^2)
         const vec& p0 = HistoryFrames[0]->detectedObjects.at(k).pos;
         const vec& p1 = HistoryFrames[1]->detectedObjects.at(k).pos;
         const vec& p2 = HistoryFrames[2]->detectedObjects.at(k).pos;
         const vec& p3 = HistoryFrames[3]->detectedObjects.at(k).pos;
         const vec& p4 = HistoryFrames[4]->detectedObjects.at(k).pos;
         vec accNumerator = (p0 * 2.0) - p1 - (p2 * 2.0) - p3 + (p4 * 2.0);
         v.acc = accNumerator / (7.0 * deltaTimeSquared);
      }
      else {
         // Fallback: estimate acceleration from available consecutive velocity changes
         std::vector<std::pair<vec, double>> positionsWithTime;
         for (uint8_t i = 0; i < HISTORY_FRAME_COUNTS; ++i) {
            if (HistoryFrames[i] != nullptr && HistoryFrames[i]->detectedObjects.count(k)) {
               positionsWithTime.emplace_back(HistoryFrames[i]->detectedObjects.at(k).pos, static_cast<double>(HistoryFrames[i]->timestamp));
            }
         }
         if (positionsWithTime.size() >= 3) {
            std::vector<std::pair<vec, double>> velocities;
            for (size_t i = 0; i < positionsWithTime.size() - 1; ++i) {
               vec dp = positionsWithTime[i + 1].first - positionsWithTime[i].first;
               double dtInterval = positionsWithTime[i + 1].second - positionsWithTime[i].second;
               if (dtInterval <= 1e-6) {
                  dtInterval = 1.0;
               }
               velocities.emplace_back(dp / dtInterval, (positionsWithTime[i + 1].second + positionsWithTime[i].second) * 0.5);
            }
            vec totalAcc(0.0, 0.0, 0.0);
            double countedAcc = 0.0;
            for (size_t i = 0; i < velocities.size() - 1; ++i) {
               vec dv = velocities[i + 1].first - velocities[i].first;
               double dtVel = velocities[i + 1].second - velocities[i].second;
               if (dtVel <= 1e-6) {
                  dtVel = 1.0;
               }
               totalAcc += dv / dtVel;
               countedAcc += 1.0;
            }
            if (countedAcc > 0.0) {
               v.acc = totalAcc / countedAcc;
            }
         }
      }
   }
}

vec Brain::PredictPos(const obj& target, double timeFuture) const {
   if (timeFuture <= 0.0) {
      return target.pos;
   }
   return target.pos + (target.vel * timeFuture) + (target.acc * (0.5 * timeFuture * timeFuture));
}

double Brain::CalculateTimeToAim(const vec& currentCameraPos, const vec& targetPos) const {
   double deltaYaw = std::abs(currentCameraPos.YawTo(targetPos));
   if (deltaYaw > 180.0) {
      deltaYaw = 360.0 - deltaYaw;
   }
   double deltaPitch = std::abs(currentCameraPos.PitchTo(targetPos));
   double timeX = (MAX_VELOCUTY_X > 1e-6) ? (deltaYaw / MAX_VELOCUTY_X) : 0.0;
   double timeY = (MAX_VELOCUTY_Y > 1e-6) ? (deltaPitch / MAX_VELOCUTY_Y) : 0.0;
   return std::max(timeX, timeY);
}

vec Brain::CalculateInterceptionPoint(const obj& target, const vec& cameraPos) const {
   vec aimOrigin = cameraPos + CAMERA_LASER_OFFSET;
   double t = CalculateTimeToAim(aimOrigin, target.pos);
   // Iteratively refine interception time where turret aim time equals target lead time
   const int maxIterations = 15;
   const double tolerance = 1e-4;
   for (int i = 0; i < maxIterations; ++i) {
      vec futurePos = PredictPos(target, t);
      double tNext = CalculateTimeToAim(aimOrigin, futurePos);
      if (std::abs(tNext - t) < tolerance) {
         t = tNext;
         break;
      }
      // Damped update to ensure convergence
      t = 0.5 * (t + tNext);
   }
   return PredictPos(target, t);
}

double Brain::EvaluateThreatLevel(const obj& target, const vec& cameraPos) const {
   // 1. Hard filters: ignore friendlies or dead targets
   if (target.isFriendly || !target.isAlive) {
      return 0.0;
   }
   vec relPos = target.pos - cameraPos;
   double distance = relPos.magnitude();
   // Beyond maximum engagement perimeter
   if (distance > MAX_DISTANCE || distance < 1e-4) {
      return 0.0;
   }
   // Proximity
   double fDist = std::max(0.0, 1.0 - (distance / MAX_DISTANCE));
   // Radial closing velocity
   double closingVel = -(relPos * target.vel) / distance;
   double fApproach = std::clamp(closingVel / MAX_EXPECTED_VEL, 0.0, 1.0);
   // Radial closing acceleration
   double closingAcc = -(relPos * target.acc) / distance;
   double fAcc = std::clamp(closingAcc / MAX_EXPECTED_ACC, 0.0, 1.0);
   // Aiming Opportunity
   double aimTime = CalculateTimeToAim(cameraPos, target.pos);
   double fAim = std::max(0.0, 1.0 - (aimTime / MAX_AIM_TIME));
   // Kill Confirmation Hysteresis (finish targets already under fire)
   double fFireProgress = std::min(1.0, target.timeUnderFire / TIME_UNDER_FIRE);
   // Weighted Threat Score (normalized base: 0.0 to 1.0)
   double score = (0.30 * fDist) + (0.25 * fApproach) + (0.10 * fAcc) + (0.15 * fAim) + (0.20 * fFireProgress);
   // Guarantees urgent targets jump ahead of standard targets
   if (target.isUrgent) {
      score += 1.0; // Urgent targets scale in range [1.0, 2.0]
   }
   return score;
}

obj& Brain::pickBestTarget(std::unordered_map<uint8_t, obj>& objects, const vec& cameraPos) {
   uint8_t best{};
   double score{-1.0};
   bool found = false;
   for (const auto& [k, v] : objects) {
      double tmp{EvaluateThreatLevel(v, cameraPos)};
      if (tmp > score && isShotSafe(v.pos, MAX_LASER_RADIUS, objects, cameraPos)) {
         score = tmp;
         best = k;
         found = true;
      }
   }
   if (found) {
      return objects.at(best);
   }
   if (!objects.empty()) {
      return objects.begin()->second;
   }
   throw std::out_of_range("No targets available in pickBestTarget");
}

bool Brain::isShotSafe(const vec& targetPos, double laserRadius, const std::unordered_map<uint8_t, obj>& allObjects, vec cameraPos) {
   vec laserOrigin = cameraPos + CAMERA_LASER_OFFSET;
   vec beamDir = targetPos - laserOrigin;
   double beamLengthSq = beamDir * beamDir;
   for (const auto& [id, object] : allObjects) {
      if (!object.isFriendly || !object.isAlive) {
         continue;
      }
      vec toFriendly = object.pos - laserOrigin;
      double t = 0.0;
      if (beamLengthSq > 1e-12) {
         t = (toFriendly * beamDir) / beamLengthSq;
      }
      t = std::clamp(t, 0.0, 1.0);
      vec closestPoint = laserOrigin + (beamDir * t);
      double dist = object.pos.distanceTo(closestPoint);
      if (dist <= laserRadius) {
         return false;
      }
   }
   return true;
}
