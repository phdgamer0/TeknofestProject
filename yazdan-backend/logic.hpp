#pragma once
#include "physichs.hpp"
extern const double TIME_UNDER_FIRE;
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
extern const vec CAMERA_LASER_OFFSET;
extern struct ImageFrame* HistoryFrames[];
extern std::unordered_map<uint8_t,obj> trackedObjects;
class Brain {
public:
    Brain() = default;
    ~Brain() = default;
    // Core Kinematics
    void SmoothKinematics(std::unordered_map<uint8_t, obj>& objects) const; // smoothing algorithm to reduce jittering
    void CalculateVelocity(ImageFrame** HistoryFrames) const; // updates ALL tracked object's velocity based of the last 5 frames
    void CalculateAcceleration(ImageFrame** HistoryFrames) const; // updates ALL tracked object's velocity based of the last 5 frames
    // Prediction & Targeting
    vec PredictPos(const obj& target, double timeFuture) const; // predicts the position of the tracked target at the next frame
    vec CalculateInterceptionPoint(const obj& target, vec cameraPos) const; // calculates the exact point to intercept the target
    double CalculateTimeToAim(vec currentCameraPos, vec targetPos) const; // returns the shortest time needed to intercept
    // Decision Making
    double EvaluateThreatLevel(const obj& target, vec cameraPos) const; // returns the highest threat level for the greedy algorithm
    obj* pickBestTarget(const std::unordered_map<uint8_t, obj>& objects, vec cameraPos); // returns the pointer to the best target
    bool isShotSafe(vec targetPos, double laserRadius, const std::unordered_map<uint8_t, obj>& allObjects, vec cameraPos); // checks if it is safe to shot the tracked target
};