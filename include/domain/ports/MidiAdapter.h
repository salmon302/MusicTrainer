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
    
    // New method for getting available MIDI output devices
    virtual std::vector<std::string> getAvailableOutputs() const = 0;
};

}