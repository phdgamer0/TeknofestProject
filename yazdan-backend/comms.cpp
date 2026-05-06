#include "comms.hpp"
#include <iostream>
#include <stdexcept>
void CommunicatorDevice::Transmit(CommunicatorDevice& receiver, sig signal) { // transmit a signal to another device
	 receiver.Receive(signal);
}
void CommunicatorDevice::Receive(sig signal) { // receive a signal and add it to the queue
	 switch (signal.priorityLevel)
	 {
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
	 while (!criticalQueue.empty()) {
		  criticalQueue.pop();
	 }
	 while (!importantQueue.empty()) {
		  importantQueue.pop();
	 }
	 while (!normalQueue.empty()) {
		  normalQueue.pop();
	 }
	 while (!logQueue.empty()) {
		  logQueue.pop();
	 }
}
sig CommunicatorDevice::nextSignal() { // retrieve the next signal from the queue without removing it
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
		  throw std::runtime_error("No signals in queue");
	 }
}
sig CommunicatorDevice::popSignal() { // retrieve and remove the next signal from the queue
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
		  throw std::runtime_error("No signals in queue");
	 }
}