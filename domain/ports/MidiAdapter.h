#ifndef MUSICTRAINER_MIDIADAPTER_H
#define MUSICTRAINER_MIDIADAPTER_H

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace MusicTrainer::ports {

struct MidiEvent {
    enum class Type {
        NOTE_ON,
        NOTE_OFF,
        CONTROL_CHANGE,
        PROGRAM_CHANGE,
        PITCH_BEND,
        AFTERTOUCH,
        SYSEX
    };

    Type type;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
};

struct MidiPortMetrics {
    size_t totalEvents;
    size_t errorCount;
    size_t recoveredErrors;
    double maxLatencyUs;
};

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
    virtual std::vector<std::string> getAvailableOutputs() const = 0;
};

} // namespace MusicTrainer::ports

#endif // MUSICTRAINER_MIDIADAPTER_H