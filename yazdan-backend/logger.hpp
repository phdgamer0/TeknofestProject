#pragma once
#include "comms.hpp"
#include <fstream>
#include <string>
class Logger {
private:
    std::ofstream logFile; // file stream for logging
    friend std::string signalToString(const sig& signal); // helper function to convert signal to string for logging
public:
    Logger(const std::string& filename); // constructor to open the log file
    ~Logger(); // destructor to close the log file
    void log(const std::string& message); // log a message to the file with a timestamp
    void logSignal(const sig& signal); // log a signal with its type and data
};
inline std::string signalToString(const sig& signal); // helper function to convert signal to string for logging
inline std::string priorityToString(Priority priority); // helper function to convert priority level to string for logging