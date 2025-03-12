#pragma once
#include <functional>
#include <string>
#include <vector>
#include "domain/ports/MidiPort.h"

// Use the music::ports namespace types directly
using music::ports::MidiEvent;
using music::ports::MidiPortMetrics;

namespace MusicTrainer::ports {

// Interface that matches music::ports::MidiPort but in MusicTrainer namespace
class MidiAdapter {
public:
    virtual ~MidiAdapter() = default;
    
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual void sendEvent(const MidiEvent& event) = 0;
    virtual void setEventCallback(std::function<void(const MidiEvent&)> callback) = 0;
    
    virtual MidiPortMetrics getMetrics() const = 0;
    virtual void resetMetrics() = 0;
    
    // Methods for getting available MIDI devices
    virtual std::vector<std::string> getAvailableInputs() const = 0;
    virtual std::vector<std::string> getAvailableOutputs() const = 0;
    
    // New methods for device selection
    virtual bool setInputDevice(int index) = 0;
    virtual bool setOutputDevice(int index) = 0;
    virtual void setMidiThrough(bool enabled) = 0;
    virtual void setLatency(int latencyMs) = 0;
    
    virtual int getCurrentInputDevice() const = 0;
    virtual int getCurrentOutputDevice() const = 0;
    virtual bool getMidiThrough() const = 0;
    virtual int getLatency() const = 0;
};

}