#include "../include/comms.hpp"
#include <optional>

template <typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::push(const T& item) {
   if (count == Capacity) {
      // Overwrite oldest item to prevent dropping new signals
      tail = (tail + 1) % Capacity;
      count--;
   }
   buffer[head] = item;
   head = (head + 1) % Capacity;
   count++;
   return true;
}

template <typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::pop(T& item) {
   if (count == 0)
      return false;
   item = buffer[tail];
   tail = (tail + 1) % Capacity;
   count--;
   return true;
}

template <typename T, size_t Capacity>
void RingBuffer<T, Capacity>::pop() {
   if (count > 0) {
      tail = (tail + 1) % Capacity;
      count--;
   }
}

template <typename T, size_t Capacity>
const T& RingBuffer<T, Capacity>::front() const {
   return buffer[tail];
}

template <typename T, size_t Capacity>
T& RingBuffer<T, Capacity>::front() {
   return buffer[tail];
}

template <typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::empty() const {
   return count == 0;
}

template <typename T, size_t Capacity>
bool RingBuffer<T, Capacity>::full() const {
   return count == Capacity;
}

template <typename T, size_t Capacity>
size_t RingBuffer<T, Capacity>::size() const {
   return count;
}

template <typename T, size_t Capacity>
void RingBuffer<T, Capacity>::clear() {
   head = tail = count = 0;
}

// Explicit template instantiations
template class RingBuffer<sig, 4>;
template class RingBuffer<sig, 16>;
template class RingBuffer<sig, 32>;
template class RingBuffer<sig, 64>;

void CommunicatorDevice::Transmit(CommunicatorDevice& receiver, const sig& signal) { // transmit a signal to another device
   receiver.Receive(signal);
}
void CommunicatorDevice::Receive(const sig& signal) { // receive a signal and add it to the queue
   switch (signal.priorityLevel) {
   case Priority::CRITICAL:
      criticalQueue.push(signal);
      break;
   case Priority::IMPORTANT:
      importantQueue.push(signal);
      break;
   case Priority::NORMAL:
      normalQueue.push(signal);
      break;
   case Priority::LOG:
      logQueue.push(signal);
      break;
   default:
      break;
   }
}
bool CommunicatorDevice::HasSignals() const { // check if there are any signals in the queue
   return !criticalQueue.empty() || !importantQueue.empty() || !normalQueue.empty() || !logQueue.empty();
}
void CommunicatorDevice::ClearSignals() { // clear all signals from the queue
   criticalQueue.clear();
   importantQueue.clear();
   normalQueue.clear();
   logQueue.clear();
}
std::optional<sig> CommunicatorDevice::nextSignal() const { // retrieve the next signal from the queue without removing it
   if (!criticalQueue.empty()) {
      return criticalQueue.front();
   }
   else if (!importantQueue.empty()) {
      return importantQueue.front();
   }
   else if (!normalQueue.empty()) {
      return normalQueue.front();
   }
   else if (!logQueue.empty()) {
      return logQueue.front();
   }
   else {
      return std::nullopt;
   }
}
std::optional<sig> CommunicatorDevice::popSignal() { // retrieve and remove the next signal from the queue
   if (!criticalQueue.empty()) {
      sig signal = criticalQueue.front();
      criticalQueue.pop();
      return signal;
   }
   else if (!importantQueue.empty()) {
      sig signal = importantQueue.front();
      importantQueue.pop();
      return signal;
   }
   else if (!normalQueue.empty()) {
      sig signal = normalQueue.front();
      normalQueue.pop();
      return signal;
   }
   else if (!logQueue.empty()) {
      sig signal = logQueue.front();
      logQueue.pop();
      return signal;
   }
   else {
      return std::nullopt;
   }
}