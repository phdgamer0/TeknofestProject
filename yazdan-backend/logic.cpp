#include "constants.hpp"
#include "logic.hpp"
#include "physichs.hpp"
#include "ardunio.hpp"

extern const double TIME_UNDER_FIRE;
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
extern const vec CAMERA_LASER_OFFSET;
extern struct ImageFrame* HistoryFrames;

void Brain::CalculateVelocity(ImageFrame** HistoryFrames)const {
    for(auto& [k,v]: trackedObjects){
        vec new_vel{0.0,0.0,0.0};
        float counted_frames = 0.0F;
        for(uint8_t i = {0U}; i < HISTORY_FRAME_COUNTS - 1U; i++){
            if(HistoryFrames[i+1]->detectedObjects.count(k) and HistoryFrames[i]->detectedObjects.count(k)){
                new_vel+=(HistoryFrames[i+1]->detectedObjects[k].pos-HistoryFrames[i]->detectedObjects[k].pos);
                counted_frames++;
            }
        }
        v.vel=new_vel/counted_frames;
    }
}