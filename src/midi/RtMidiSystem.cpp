#include "RtMidiSystem.h"
#include <RtMidi.h>
#include <iostream> // For error reporting/debugging
#include <chrono>   // For timestamp conversion (example)
#include <stdexcept> // For exceptions

namespace MusicTrainer {
namespace Midi {

// --- Static Callback Wrapper ---

void RtMidiSystem::rtMidiInputCallbackWrapper(double timeStamp, std::vector<unsigned char>* message, void* userData) {
    // Cast userData back to the RtMidiSystem instance
    RtMidiSystem* self = static_cast<RtMidiSystem*>(userData);
    if (self) {
        self->processIncomingMessage(timeStamp, message);
    }
}

// --- Internal Message Processing ---

void RtMidiSystem::processIncomingMessage(double timeStamp, std::vector<unsigned char>* message) {
    if (!message || message->empty()) {
        return;
    }

    // Convert RtMidi timestamp (seconds since start?) and message format to our MidiEvent
    // NOTE: RtMidi timestamp interpretation might need adjustment based on API version/platform.
    // Here, we assume it's seconds and convert to nanoseconds for higher resolution example.
    auto now_ns = std::chrono::high_resolution_clock::now().time_since_epoch();
    std::uint64_t timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_ns).count();
    // Or potentially: timestamp_ns = static_cast<uint64_t>(timeStamp * 1e9);

    MidiEvent event;
    event.timestamp = timestamp_ns; // Use converted timestamp
    event.status = (*message)[0];
    if (message->size() > 1) event.data1 = (*message)[1];
    if (message->size() > 2) event.data2 = (*message)[2];
    // Note: RtMidi might provide messages longer than 3 bytes (e.g., SysEx)
    // This basic conversion only handles typical channel messages.

    // Lock mutex before calling user callback if RtMidi calls back from a separate thread
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (inputCallback) {
        try {
             inputCallback(event);
        } catch (const std::exception& e) {
            std::cerr << "Error in MIDI input callback: " << e.what() << std::endl;
            // Decide how to handle callback exceptions (log, ignore, etc.)
        } catch (...) {
             std::cerr << "Unknown error in MIDI input callback." << std::endl;
        }
    }
}


// --- Constructor / Destructor ---

RtMidiSystem::RtMidiSystem() {
    try {
        midiIn = std::make_unique<RtMidiIn>();
        midiOut = std::make_unique<RtMidiOut>();
    } catch (const RtMidiError &error) {
        // Handle RtMidi initialization error (e.g., no backend available)
        std::cerr << "RtMidi initialization error: " << error.getMessage() << std::endl;
        // We might want to throw here or enter a degraded state
        midiIn.reset();
        midiOut.reset();
         throw; // Re-throw for caller to handle
    }
}

RtMidiSystem::~RtMidiSystem() {
    // Ensure devices are closed (unique_ptr handles deletion)
    closeInputDevice();
    closeOutputDevice();
}

// --- Device Management ---

std::vector<std::string> RtMidiSystem::listInputDevices() {
    std::vector<std::string> names;
    if (!midiIn) return names;
    try {
        unsigned int nPorts = midiIn->getPortCount();
        for (unsigned int i = 0; i < nPorts; ++i) {
            names.push_back(midiIn->getPortName(i));
        }
    } catch (const RtMidiError &error) {
        std::cerr << "RtMidi error listing input devices: " << error.getMessage() << std::endl;
    }
    return names;
}

std::vector<std::string> RtMidiSystem::listOutputDevices() {
    std::vector<std::string> names;
     if (!midiOut) return names;
    try {
        unsigned int nPorts = midiOut->getPortCount();
        for (unsigned int i = 0; i < nPorts; ++i) {
            names.push_back(midiOut->getPortName(i));
        }
    } catch (const RtMidiError &error) {
        std::cerr << "RtMidi error listing output devices: " << error.getMessage() << std::endl;
    }
    return names;
}

bool RtMidiSystem::openInputDevice(const std::string& deviceName) {
    if (!midiIn || isInputDeviceOpen()) {
        closeInputDevice(); // Close previous if any
    }
     if (!midiIn) return false; // If initialization failed

    try {
        unsigned int nPorts = midiIn->getPortCount();
        for (unsigned int i = 0; i < nPorts; ++i) {
            if (midiIn->getPortName(i) == deviceName) {
                midiIn->openPort(i);
                midiIn->setCallback(&rtMidiInputCallbackWrapper, this);
                // Optional: Ignore SysEx, Timing, Active Sensing messages
                midiIn->ignoreTypes(true, true, true);
                openInputDeviceName = deviceName;
                std::cout << "Opened MIDI Input: " << deviceName << std::endl; // Debug
                return true;
            }
        }
        std::cerr << "RtMidi error: Input device not found: " << deviceName << std::endl;
    } catch (const RtMidiError &error) {
        std::cerr << "RtMidi error opening input device '" << deviceName << "': " << error.getMessage() << std::endl;
    }
    openInputDeviceName.clear();
    return false;
}

bool RtMidiSystem::openOutputDevice(const std::string& deviceName) {
    if (!midiOut || isOutputDeviceOpen()) {
        closeOutputDevice(); // Close previous if any
    }
     if (!midiOut) return false; // If initialization failed

    try {
        unsigned int nPorts = midiOut->getPortCount();
        for (unsigned int i = 0; i < nPorts; ++i) {
            if (midiOut->getPortName(i) == deviceName) {
                midiOut->openPort(i);
                openOutputDeviceName = deviceName;
                 std::cout << "Opened MIDI Output: " << deviceName << std::endl; // Debug
                return true;
            }
        }
         std::cerr << "RtMidi error: Output device not found: " << deviceName << std::endl;
    } catch (const RtMidiError &error) {
         std::cerr << "RtMidi error opening output device '" << deviceName << "': " << error.getMessage() << std::endl;
    }
    openOutputDeviceName.clear();
    return false;
}

void RtMidiSystem::closeInputDevice() {
    if (midiIn && midiIn->isPortOpen()) {
        try {
            midiIn->cancelCallback();
            midiIn->closePort();
             std::cout << "Closed MIDI Input: " << openInputDeviceName << std::endl; // Debug
        } catch (const RtMidiError &error) {
             std::cerr << "RtMidi error closing input port: " << error.getMessage() << std::endl;
        }
    }
    openInputDeviceName.clear();
}

void RtMidiSystem::closeOutputDevice() {
    if (midiOut && midiOut->isPortOpen()) {
         try {
            midiOut->closePort();
             std::cout << "Closed MIDI Output: " << openOutputDeviceName << std::endl; // Debug
        } catch (const RtMidiError &error) {
             std::cerr << "RtMidi error closing output port: " << error.getMessage() << std::endl;
        }
    }
    openOutputDeviceName.clear();
}

bool RtMidiSystem::isInputDeviceOpen() const {
    return midiIn && midiIn->isPortOpen();
}

bool RtMidiSystem::isOutputDeviceOpen() const {
    return midiOut && midiOut->isPortOpen();
}

std::optional<std::string> RtMidiSystem::getOpenInputDeviceName() const {
    if (isInputDeviceOpen()) {
        return openInputDeviceName;
    }
    return std::nullopt;
}

std::optional<std::string> RtMidiSystem::getOpenOutputDeviceName() const {
     if (isOutputDeviceOpen()) {
        return openOutputDeviceName;
    }
    return std::nullopt;
}

// --- Input Handling ---

void RtMidiSystem::setInputCallback(MidiInputCallback callback) {
     std::lock_guard<std::mutex> lock(callbackMutex);
     inputCallback = std::move(callback);
}

// --- Output Handling ---

void RtMidiSystem::sendEvent(const MidiEvent& event) {
    if (!isOutputDeviceOpen()) return;

    std::vector<unsigned char> message;
    message.push_back(event.status);
    // Only add data bytes if they are relevant for the status byte type
    // This requires more sophisticated checking based on MIDI spec,
    // but for common messages like NoteOn/Off/CC, 3 bytes are typical.
    if (event.isNoteOn() || event.isNoteOff() || event.isControlChange()) { // Example check
         message.push_back(event.data1);
         message.push_back(event.data2);
    } else if (/* check for 2-byte messages like ProgramChange */ false) {
         message.push_back(event.data1);
    } // Add checks for other message types (PitchBend=3 bytes, etc.)

    if (!message.empty()) {
        try {
            midiOut->sendMessage(&message);
        } catch (const RtMidiError &error) {
             std::cerr << "RtMidi error sending message: " << error.getMessage() << std::endl;
        }
    }
}

void RtMidiSystem::sendNoteOn(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity) {
    sendEvent(MidiEvent::NoteOnEvent(channel, note, velocity));
}

void RtMidiSystem::sendNoteOff(std::uint8_t channel, std::uint8_t note, std::uint8_t velocity) {
     sendEvent(MidiEvent::NoteOffEvent(channel, note, velocity));
}

void RtMidiSystem::sendControlChange(std::uint8_t channel, std::uint8_t ccNumber, std::uint8_t value) {
     sendEvent(MidiEvent::ControlChangeEvent(channel, ccNumber, value));
}


} // namespace Midi
} // namespace MusicTrainer