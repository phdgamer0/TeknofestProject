#pragma once
#include <cstdint>
#include <queue>
enum class SIGNALS:uint8_t{
// CAMERA signals
    CAM_MOVE,
    CAM_STOP,
    CAM_CALIBRATE,
    CAM_RESET,
    CAM_UPDATE,
    CAM_ON_TARGET,
    CAM_UNRESPONSIVE,
// YAW engine signals
    YAW_ENGINE_MOVE,
    YAW_ENGINE_STOP,
    YAW_ENGINE_CALIBRATE,
    YAW_ENGINE_RESET,
    YAW_ENGINE_UPDATE,
    YAW_ENGINE_LIMIT_REACHED,
    YAW_ENGINE_UNRESPONSIVE,
// PITCH engine signals (l/r)
    PITCH_ENGINE_LEFT_MOVE,
    PITCH_ENGINE_LEFT_STOP,
    PITCH_ENGINE_LEFT_CALIBRATE,
    PITCH_ENGINE_LEFT_RESET,
    PITCH_ENGINE_LEFT_UPDATE,
    PITCH_ENGINE_LEFT_LIMIT_REACHED,
    PITCH_ENGINE_LEFT_UNRESPONSIVE,
    PITCH_ENGINE_RIGHT_MOVE,
    PITCH_ENGINE_RIGHT_STOP,
    PITCH_ENGINE_RIGHT_CALIBRATE,
    PITCH_ENGINE_RIGHT_RESET,
    PITCH_ENGINE_RIGHT_UPDATE,
    PITCH_ENGINE_RIGHT_LIMIT_REACHED,
    PITCH_ENGINE_RIGHT_UNRESPONSIVE,
// LASER signals
    LASER_ON,
    LASER_OFF,
    LASER_UNRESPONSIVE,
// SYSTEM signals
    SYSTEM_SHUTDOWN,
    SYSTEM_RESTART,
    SYSTEM_UNRESPONSIVE,
// OBJECT signals
    OBJECT_DETECTED,
    OBJECT_TRACKED,
    OBJECT_SKIPPED,
    OBJECT_DESTROYED,
    OBJECT_ESCAPED,
    OBJECT_OUT_OF_RANGE,
    OBJECT_FRIENDLY_FIRE,
    OBJECT_LOST,
// CUSTOM signals
    SYSTEM_STOP,
    SYSTEM_RESUME,
};
enum class DataTypes:uint8_t{
    VEC_3D,
    STRING,
    INTEGER,
    FLOAT,
    BOOLEAN,
};
typedef struct Signal{
    SIGNALS signal;
    DataTypes dataType; // type of the data being transmitted with the signal
    void* data = nullptr; // optional data pointer for signal parameters
}sig;
class CommunicatorDevice{
private:
    std::queue<sig> signalQueue; // queue to hold incoming signals
public:
    void Transmit(CommunicatorDevice& receiver, sig signal); // transmit a signal to another device
    void Receive(sig signal); // receive a signal and add it to the queue
    bool HasSignals() const; // check if there are any signals in the queue
    void ClearSignals(); // clear all signals from the queue
    sig nextSignal(); // retrieve the next signal from the queue without removing it
    sig popSignal(); // retrieve and remove the next signal from the queue
};