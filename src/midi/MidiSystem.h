#ifndef MUSIC_TRAINER_MIDI_MIDISYSTEM_H
#define MUSIC_TRAINER_MIDI_MIDISYSTEM_H

#include <vector>
#include <string>
#include <cstdint>
#include <functional> // For std::function (callback)
#include <optional>   // For optional return values

namespace MusicTrainer {
namespace Midi {

/**
 * @brief Represents a standard MIDI message.
 */
struct MidiEvent {
    // Using standard MIDI message structure (Status, Data1, Data2)
    // Status byte encodes message type and channel (0-15)
    // Example Status Bytes: NoteOff=0x8n, NoteOn=0x9n, ControlChange=0xBn
    std::uint8_t status = 0;
    std::uint8_t data1 = 0; // e.g., Note Number, CC Number
    std::uint8_t data2 = 0; // e.g., Velocity, CC Value
    std::uint64_t timestamp = 0; // High-resolution timestamp (e.g., nanoseconds from epoch or start)

    // Helper methods to decode status byte
    std::uint8_t getType() const { return status & 0xF0; } // Message type (e.g., 0x80, 0x90)
    std::uint8_t getChannel() const { return status & 0x0F; } // Channel (0-15)

    // Check message type
    bool isNoteOff() const { return getType() == 0x80 || (getType() == 0x90 && data2 == 0); } // NoteOn vel 0 = NoteOff
    bool isNoteOn() const { return getType() == 0x90 && data2 > 0; }
    bool isControlChange() const { return getType() == 0xB0; }
    // Add other types as needed (ProgramChange, PitchBend, etc.)

    // Static factory methods (optional)
    static MidiEvent NoteOnEvent(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity, std::uint64_t ts = 0) {
        return {static_cast<uint8_t>(0x90 | (channel & 0x0F)), note, velocity, ts};
    }
    static MidiEvent NoteOffEvent(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity = 0, std::uint64_t ts = 0) {
         return {static_cast<uint8_t>(0x80 | (channel & 0x0F)), note, velocity, ts};
    }
     static MidiEvent ControlChangeEvent(std::uint8_t channel, std::uint8_t ccNumber, std::uint8_t value, std::uint64_t ts = 0) {
         return {static_cast<uint8_t>(0xB0 | (channel & 0x0F)), ccNumber, value, ts};
    }
};

// Type alias for the callback function processing incoming MIDI events
using MidiInputCallback = std::function<void(const MidiEvent& event)>;

/**
 * @brief Abstract interface for interacting with MIDI input and output devices.
 *
 * Defines the contract for listing devices, opening/closing connections,
 * sending MIDI messages, and receiving messages via a callback.
 * Concrete implementations will use specific MIDI libraries (e.g., RtMidi, ALSA).
 */
class MidiSystem {
public:
    virtual ~MidiSystem() = default;

    // --- Device Management ---

    /** @brief Get a list of available MIDI input device names. */
    virtual std::vector<std::string> listInputDevices() = 0;

    /** @brief Get a list of available MIDI output device names. */
    virtual std::vector<std::string> listOutputDevices() = 0;

    /**
     * @brief Open a connection to a specific MIDI input device by name.
     * @param deviceName The name of the device (from listInputDevices).
     * @return True if successful, false otherwise.
     */
    virtual bool openInputDevice(const std::string& deviceName) = 0;

    /**
     * @brief Open a connection to a specific MIDI output device by name.
     * @param deviceName The name of the device (from listOutputDevices).
     * @return True if successful, false otherwise.
     */
    virtual bool openOutputDevice(const std::string& deviceName) = 0;

    /** @brief Close the currently open MIDI input device connection. */
    virtual void closeInputDevice() = 0;

    /** @brief Close the currently open MIDI output device connection. */
    virtual void closeOutputDevice() = 0;

    /** @brief Check if an input device is currently connected and open. */
    virtual bool isInputDeviceOpen() const = 0;

    /** @brief Check if an output device is currently connected and open. */
    virtual bool isOutputDeviceOpen() const = 0;

    /** @brief Get the name of the currently open input device, if any. */
    virtual std::optional<std::string> getOpenInputDeviceName() const = 0;

     /** @brief Get the name of the currently open output device, if any. */
    virtual std::optional<std::string> getOpenOutputDeviceName() const = 0;


    // --- Input Handling ---

    /**
     * @brief Set the callback function to be invoked when a MIDI message is received.
     * @param callback The function to call with the received MidiEvent.
     */
    virtual void setInputCallback(MidiInputCallback callback) = 0;

    /**
     * @brief (Optional) Map a specific MIDI input channel to an internal application voice index.
     * If not implemented or used, the application might handle channel filtering itself.
     * @param inputChannel MIDI channel (0-15).
     * @param voiceIndex Application-specific voice index.
     */
    virtual void setChannelMapping(int inputChannel, int voiceIndex) {
        // Default implementation does nothing, concrete classes can override
        (void)inputChannel; (void)voiceIndex; // Mark as unused
    }


    // --- Output Handling ---

    /**
     * @brief Send a raw MIDI event to the open output device.
     * @param event The MidiEvent object containing status and data bytes.
     */
    virtual void sendEvent(const MidiEvent& event) = 0;

    /** @brief Convenience method to send a Note On message. */
    virtual void sendNoteOn(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity) = 0;

    /** @brief Convenience method to send a Note Off message. */
    virtual void sendNoteOff(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity = 0) = 0;

    /** @brief Convenience method to send a Control Change message. */
    virtual void sendControlChange(std::uint8_t channel, std::uint8_t ccNumber, std::uint8_t value) = 0;

    // Add other convenience methods as needed (e.g., sendProgramChange)

}; // class MidiSystem

} // namespace Midi
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_MIDI_MIDISYSTEM_H