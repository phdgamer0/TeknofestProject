#pragma once
#include "physichs.hpp"
extern constexpr uint8_t FRAME_HISTORY_COUNT;
extern const double TIME_UNDER_FIRE;
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
extern const vec CAMERA_LASER_OFFSET;
extern struct ImageFrame* HistoryFrames;
class Brain{
	public:
		Brain() = default;
		~Brain() = default;
		void CalculateAcceleration(ImageFrame* HistoryFrames)const;
		void CalculateVelocity(ImageFrame* HistoryFrames)const;
		vec PredictPos(const ImageFrame& lastFrame)const;
		bool isShotSafe(vec targetPos, double laserRadius, const std::unordered_map<uint8_t, obj>& allObjects, vec cameraPos);
		obj* pickBestTarget(const std::unordered_map<uint8_t, obj>& objects, vec cameraPos);
		std::pair<double,double> BestSpeed(vec targetPos) const;
};