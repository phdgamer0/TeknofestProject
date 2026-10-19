#pragma once
#include "physichs.hpp"
#include <cstdint>
constexpr const char PIPE_FILE_PATH[] = "./pipe/python2cpp.pipe";
constexpr const char* LOG_FILE_PATH = "./yazdan-backend/logs.txt";
constexpr double AIM_ENGAGE_TOLERANCE_DEG = 1.2; // Tighter error threshold to initiate laser fire
constexpr double AIM_HOLD_TOLERANCE_DEG = 1.8;   // Wider hysteresis threshold to maintain fire without chatter
constexpr double AIM_TOLERANCE_DEG = 1.5;
constexpr double MAX_AIM_TIME = 9.0F;
constexpr double MAX_DISTANCE = 20.0F;
constexpr double MAX_EXPECTED_ACC = 10.0F;
constexpr double MAX_EXPECTED_VEL = 15.0F;
constexpr double MAX_LASER_RADIUS = 0.55F; // Minimum safe distance from friendly objects (balloon radius ~0.5m)
constexpr double MAX_PITCH_DEG = 85.0;
constexpr double MAX_VELOCUTY_X = 120.0F;
constexpr double MAX_VELOCUTY_Y = 120.0F;
constexpr double MAX_YAW_DEG = 180.0;
constexpr double MIN_PITCH_DEG = -45.0;
constexpr double MIN_YAW_DEG = -180.0;
constexpr double TIME_UNDER_FIRE = 1.5F;
constexpr double ZOMBIE_QUARANTINE_SECONDS = 3.0; // Cooldown (seconds) to release destroyed target ID for recycling
constexpr uint8_t HISTORY_FRAME_COUNTS = 5U;
constexpr uint8_t MAX_STALE_FRAMES = 5U;                // Prune targets not observed for 5 consecutive frames
constexpr vec CAMERA_LASER_OFFSET = {0.1F, 0.0F, 0.0F}; // offset of the laser from the camera center in the (x,y,z) directions