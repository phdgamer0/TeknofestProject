#pragma once
#include "physichs.hpp"
#include <iostream>
#include <unordered_map>
#include <cstdint>
const double TIME_UNDER_FIRE = 3.0F;
const double MAX_VELOCUTY_X = 20.0F;
const double MAX_VELOCUTY_Y = 30.0F;
const vec CAMERA_LASER_OFFSET = {0.1F, 0.0F, 0.0F}; // offset of the laser from the camera center in the (x,y,z) directions
constexpr uint8_t HISTORY_FRAME_COUNTS = 5U; 
