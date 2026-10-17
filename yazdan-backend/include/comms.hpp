#pragma once
#include "../include/physichs.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>

template <typename T, size_t Capacity = 32>
class RingBuffer {
 private:
   std::array<T, Capacity> buffer{};
   size_t head = 0; // write index
   size_t tail = 0; // read index
   size_t count = 0;

 public:
   constexpr RingBuffer() = default;

   bool push(const T& item);
   bool pop(T& item);
   void pop();
   const T& front() const;
   T& front();
   bool empty() const;
   bool full() const;
   size_t size() const;
   void clear();
};

using SignalValue = std::variant<std::monostate, std::pair<vec, vec>, vec, double, int, bool, std::string>;
enum class SIGNALS : uint8_t {
   // SYSTEM signals
   SYSTEM_START,
   SYSTEM_STOP,
   SYSTEM_RESTART,
   SYSTEM_SHUTDOWN,
   SYSTEM_UNRESPONSIVE,

   // CAMERA signals
   CAM_START,
   CAM_STOP,
   CAM_CALIBRATE,
   CAM_RESET,
   CAM_ON_TARGET,
   CAM_UNRESPONSIVE,

   // YAW engine signals (controls left & right yaw engines)
   YAW_ENGINE_MOVE,
   YAW_ENGINE_STOP,
   YAW_ENGINE_CALIBRATE,
   YAW_ENGINE_RESET,
   YAW_ENGINE_LIMIT_REACHED,
   YAW_ENGINE_UNRESPONSIVE,

   // PITCH engine signals (controls elevation engine)
   PITCH_ENGINE_MOVE,
   PITCH_ENGINE_STOP,
   PITCH_ENGINE_CALIBRATE,
   PITCH_ENGINE_RESET,
   PITCH_ENGINE_LIMIT_REACHED,
   PITCH_ENGINE_UNRESPONSIVE,

   // LASER signals
   LASER_ON,
   LASER_OFF,
   LASER_UNRESPONSIVE,

   // OBJECT / TARGETING events
   OBJECT_DETECTED,
   OBJECT_TRACKED,
   OBJECT_SKIPPED,
   OBJECT_DESTROYED,
   OBJECT_ESCAPED,
   OBJECT_OUT_OF_RANGE,
   OBJECT_FRIENDLY_FIRE,
   OBJECT_LOST,

   // SENTINEL
   NO_SIG
};
enum class Priority : uint8_t {
   CRITICAL,
   IMPORTANT,
   NORMAL,
   LOG,
};
typedef struct Signal {
   SignalValue value;      // value of the signal
   uint64_t timestamp;     // for logging
   SIGNALS signal;         // the literal signal
   Priority priorityLevel; // priority level of the signal
   Signal(SIGNALS signal = SIGNALS::NO_SIG, SignalValue value = std::monostate{}, Priority priorityLevel = Priority::NORMAL, uint64_t timestamp = 0) : value(value), timestamp(timestamp), signal(signal), priorityLevel(priorityLevel) {};
   ~Signal() = default;
} sig;
class CommunicatorDevice {
 private:
   static constexpr size_t QUEUE_CAPACITY = 32;
   RingBuffer<sig, QUEUE_CAPACITY> criticalQueue;  // queue to hold critical signals that need immediate attention
   RingBuffer<sig, QUEUE_CAPACITY> importantQueue; // queue to hold important signals that need attention
   RingBuffer<sig, QUEUE_CAPACITY> normalQueue;    // queue to hold incoming signals
   RingBuffer<sig, QUEUE_CAPACITY> logQueue;       // queue to hold log signals for debugging and monitoring
 public:
   void Transmit(CommunicatorDevice& receiver, const sig& signal); // transmit a signal to another device
   void Receive(const sig& signal);                                // receive a signal and add it to the queue
   bool HasSignals() const;                                        // check if there are any signals in the queue
   void ClearSignals();                                            // clear all signals from the queue
   std::optional<sig> nextSignal() const;                          // retrieve the next signal from the queue without removing it
   std::optional<sig> popSignal();                                 // retrieve and remove the next signal from the queue
};