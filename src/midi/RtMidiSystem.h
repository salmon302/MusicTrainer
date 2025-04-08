#ifndef MUSIC_TRAINER_MIDI_RTMIDISYSTEM_H
#define MUSIC_TRAINER_MIDI_RTMIDISYSTEM_H

#include "MidiSystem.h"
#include <RtMidi.h> // Include the RtMidi header (Needs to be found by build system)
#include <memory>   // For std::unique_ptr
#include <string>
#include <vector>
#include <mutex>      // For thread safety if callbacks are on different threads
#include <optional>

namespace MusicTrainer {
namespace Midi {

/**
 * @brief Concrete implementation of MidiSystem using the RtMidi library.
 */
class RtMidiSystem : public MidiSystem {
private:
    std::unique_ptr<RtMidiIn> midiIn;
    std::unique_ptr<RtMidiOut> midiOut;

    MidiInputCallback inputCallback; // Store the user-provided callback

    std::string openInputDeviceName;
    std::string openOutputDeviceName;

    // Mutex for protecting callback access if needed (depends on RtMidi callback thread)
    std::mutex callbackMutex;

    // Static callback wrapper needed for RtMidi C-style callback
    static void rtMidiInputCallbackWrapper(double timeStamp, std::vector<unsigned char>* message, void* userData);

    // Internal method to process the raw message from the wrapper
    void processIncomingMessage(double timeStamp, std::vector<unsigned char>* message);

public:
    RtMidiSystem();
    virtual ~RtMidiSystem() override;

    // Delete copy constructor and assignment operator
    RtMidiSystem(const RtMidiSystem&) = delete;
    RtMidiSystem& operator=(const RtMidiSystem&) = delete;

    // --- MidiSystem Interface Implementation ---

    std::vector<std::string> listInputDevices() override;
    std::vector<std::string> listOutputDevices() override;

    bool openInputDevice(const std::string& deviceName) override;
    bool openOutputDevice(const std::string& deviceName) override;

    void closeInputDevice() override;
    void closeOutputDevice() override;

    bool isInputDeviceOpen() const override;
    bool isOutputDeviceOpen() const override;

    std::optional<std::string> getOpenInputDeviceName() const override;
    std::optional<std::string> getOpenOutputDeviceName() const override;

    void setInputCallback(MidiInputCallback callback) override;
    // setChannelMapping can use the default implementation for now

    void sendEvent(const MidiEvent& event) override;
    void sendNoteOn(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity) override;
    void sendNoteOff(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity = 0) override;
    void sendControlChange(std::uint8_t channel, std::uint8_t ccNumber, std::uint8_t value) override;

}; // class RtMidiSystem

} // namespace Midi
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_MIDI_RTMIDISYSTEM_H