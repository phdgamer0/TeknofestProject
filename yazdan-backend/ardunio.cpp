#include "ardunio.hpp"
#include "physichs.hpp"
#include "comms.hpp"
#include "logger.hpp"
#include <iostream>
#include <stdexcept>
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
extern const vec CAMERA_LASER_OFFSET; // offset of the laser from the camera center in the (x,y,z) directions
void AI::operator<<(const img& frame){
	 processImage(frame);
}
