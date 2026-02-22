#include "comms.hpp"
#include <iostream>
#include <stdexcept>
void CommunicatorDevice::Transmit(CommunicatorDevice& receiver, sig signal) { // transmit a signal to another device
    receiver.Receive(signal);
}
void CommunicatorDevice::Receive(sig signal) { // receive a signal and add it to the queue
    signalQueue.push(signal);
}
bool CommunicatorDevice::HasSignals() const { // check if there are any signals in the queue
    return !signalQueue.empty();
}
void CommunicatorDevice::ClearSignals() { // clear all signals from the queue
    while (!signalQueue.empty()) {
        signalQueue.pop();
    }
}
sig CommunicatorDevice::nextSignal() { // retrieve the next signal from the queue without removing it
    if (signalQueue.empty()) {
        throw std::runtime_error("No signals in queue");
    }
    return signalQueue.front();
}
sig CommunicatorDevice::popSignal() { // retrieve and remove the next signal from the queue
    if (signalQueue.empty()) {
        throw std::runtime_error("No signals in queue");
    }
    sig signal = signalQueue.front();
    signalQueue.pop();
    return signal;
}