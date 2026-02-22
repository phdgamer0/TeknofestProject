#include "logger.hpp"
#include "physichs.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
inline std::string signalToString(sig signal){
    switch (signal.signal){
        case SIGNALS::CAM_MOVE: return "CAM_MOVE";
        case SIGNALS::CAM_STOP: return "CAM_STOP";
        case SIGNALS::CAM_CALIBRATE: return "CAM_CALIBRATE";
        case SIGNALS::CAM_RESET: return "CAM_RESET";
        case SIGNALS::CAM_UPDATE: return "CAM_UPDATE";
        case SIGNALS::CAM_ON_TARGET: return "CAM_ON_TARGET";
        case SIGNALS::CAM_UNRESPONSIVE: return "CAM_UNRESPONSIVE";
        case SIGNALS::YAW_ENGINE_MOVE: return "YAW_ENGINE_MOVE";
        case SIGNALS::YAW_ENGINE_STOP: return "YAW_ENGINE_STOP";
        case SIGNALS::YAW_ENGINE_CALIBRATE: return "YAW_ENGINE_CALIBRATE";
        case SIGNALS::YAW_ENGINE_RESET: return "YAW_ENGINE_RESET";
        case SIGNALS::YAW_ENGINE_UPDATE: return "YAW_ENGINE_UPDATE";
        case SIGNALS::YAW_ENGINE_LIMIT_REACHED: return "YAW_ENGINE_LIMIT_REACHED";
        case SIGNALS::YAW_ENGINE_UNRESPONSIVE: return "YAW_ENGINE_UNRESPONSIVE";
        case SIGNALS::PITCH_ENGINE_LEFT_MOVE: return "PITCH_ENGINE_LEFT_MOVE";
        case SIGNALS::PITCH_ENGINE_LEFT_STOP: return "PITCH_ENGINE_LEFT_STOP";
        case SIGNALS::PITCH_ENGINE_LEFT_CALIBRATE: return "PITCH_ENGINE_LEFT_CALIBRATE";
        case SIGNALS::PITCH_ENGINE_LEFT_RESET: return "PITCH_ENGINE_LEFT_RESET";
        case SIGNALS::PITCH_ENGINE_LEFT_UPDATE: return "PITCH_ENGINE_LEFT_UPDATE";
        case SIGNALS::PITCH_ENGINE_LEFT_LIMIT_REACHED: return "PITCH_ENGINE_LEFT_LIMIT_REACHED";
        case SIGNALS::PITCH_ENGINE_LEFT_UNRESPONSIVE: return "PITCH_ENGINE_LEFT_UNRESPONSIVE";
        case SIGNALS::PITCH_ENGINE_RIGHT_MOVE: return "PITCH_ENGINE_RIGHT_MOVE";
        case SIGNALS::PITCH_ENGINE_RIGHT_STOP: return "PITCH_ENGINE_RIGHT_STOP";
        case SIGNALS::PITCH_ENGINE_RIGHT_CALIBRATE: return "PITCH_ENGINE_RIGHT_CALIBRATE";
        case SIGNALS::PITCH_ENGINE_RIGHT_RESET: return "PITCH_ENGINE_RIGHT_RESET";
        case SIGNALS::PITCH_ENGINE_RIGHT_UPDATE: return "PITCH_ENGINE_RIGHT_UPDATE";
        case SIGNALS::PITCH_ENGINE_RIGHT_LIMIT_REACHED: return "PITCH_ENGINE_RIGHT_LIMIT_REACHED";
        case SIGNALS::PITCH_ENGINE_RIGHT_UNRESPONSIVE: return "PITCH_ENGINE_RIGHT_UNRESPONSIVE";
        case SIGNALS::LASER_ON: return "LASER_ON";
        case SIGNALS::LASER_OFF: return "LASER_OFF";
        case SIGNALS::LASER_UNRESPONSIVE: return "LASER_UNRESPONSIVE";
        case SIGNALS::SYSTEM_SHUTDOWN: return "SYSTEM_SHUTDOWN";
        case SIGNALS::SYSTEM_RESTART: return "SYSTEM_RESTART";
        case SIGNALS::SYSTEM_UNRESPONSIVE: return "SYSTEM_UNRESPONSIVE";
        case SIGNALS::OBJECT_DETECTED: return "OBJECT_DETECTED";
        case SIGNALS::OBJECT_TRACKED: return "OBJECT_TRACKED";
        case SIGNALS::OBJECT_SKIPPED: return "OBJECT_SKIPPED";
        case SIGNALS::OBJECT_DESTROYED: return "OBJECT_DESTROYED";
        case SIGNALS::OBJECT_ESCAPED: return "OBJECT_ESCAPED";
        case SIGNALS::OBJECT_OUT_OF_RANGE: return "OBJECT_OUT_OF_RANGE";
        case SIGNALS::OBJECT_FRIENDLY_FIRE: return "OBJECT_FRIENDLY_FIRE";
        case SIGNALS::OBJECT_LOST: return "OBJECT_LOST";
        case SIGNALS::SYSTEM_STOP: return "SYSTEM_STOP";
        case SIGNALS::SYSTEM_RESUME: return "SYSTEM_RESUME";
        default: return "UNKNOWN_SIGNAL";
    }
}
inline std::string priorityToString(Priority priority){
    switch (priority){
        case Priority::CRITICAL: return "CRITICAL";
        case Priority::IMPORTANT: return "IMPORTANT";
        case Priority::NORMAL: return "NORMAL";
        case Priority::LOG: return "LOG";
        default: return "UNKNOWN_PRIORITY";
    }
}
Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::out | std::ios::app); // open the log file for appending
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }
}
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close(); // close the log file when the logger is destroyed
    }
}
void Logger::log(const std::string& message) {
    if (logFile.is_open()) {
        // Get the current time for the timestamp
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        auto localTime = std::localtime(&timeT);
        // Write the timestamp and message to the log file
        logFile << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
    }
}
void Logger::logSignal(const sig& signal) {
    if (logFile.is_open()) {
        // Log the signal type and data (if any)
        logFile << "Signal: " << static_cast<int>(signal.signal) << ", Data Type: " << static_cast<int>(signal.dataType);
        if (signal.data != nullptr) {
            switch (signal.dataType) {
                case DataTypes::NONE:{
                    break;
                }
                case DataTypes::VEC_3D: {
                    vec* dataVec = static_cast<vec*>(signal.data);
                    logFile << ", Data: (" << dataVec->x << ", " << dataVec->y << ", " << dataVec->z << ")";
                    break;
                }
                case DataTypes::STRING: {
                    std::string* dataStr = static_cast<std::string*>(signal.data);
                    logFile << ", Data: " << *dataStr;
                    break;
                }
                case DataTypes::INTEGER: {
                    int* dataInt = static_cast<int*>(signal.data);
                    logFile << ", Data: " << *dataInt;
                    break;
                }
                case DataTypes::FLOAT: {
                    float* dataFloat = static_cast<float*>(signal.data);
                    logFile << ", Data: " << *dataFloat;
                    break;
                }
                case DataTypes::BOOLEAN: {
                    bool* dataBool = static_cast<bool*>(signal.data);
                    logFile << ", Data: " << (*dataBool ? "true" : "false");
                    break;
                }
                default:{
                    logFile << ", Data: [Unknown Type]";
                }
            }
            logFile << std::endl;
        }
    }
}