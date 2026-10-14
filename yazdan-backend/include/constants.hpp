#pragma once
#include "physichs.hpp"
#include <cstdint>
constexpr double MAX_DISTANCE = 20.0F;
constexpr double TIME_UNDER_FIRE = 3.0F;
constexpr double MAX_VELOCUTY_X = 20.0F;
constexpr double MAX_VELOCUTY_Y = 30.0F;
constexpr double MAX_EXPECTED_VEL = 15.0F;
constexpr double MAX_EXPECTED_ACC = 10.0F;
constexpr double MAX_AIM_TIME = 9.0F;
constexpr double MAX_LASER_RADIUS = 0.001F;
constexpr vec CAMERA_LASER_OFFSET = {0.1F, 0.0F, 0.0F}; // offset of the laser from the camera center in the (x,y,z) directions
constexpr uint8_t HISTORY_FRAME_COUNTS = 5U;