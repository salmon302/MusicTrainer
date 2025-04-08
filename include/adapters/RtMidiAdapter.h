#ifndef MUSICTRAINERV3_RTMIDIADAPTER_H
#define MUSICTRAINERV3_RTMIDIADAPTER_H

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "domain/ports/MidiPort.h"
#include "domain/ports/MidiAdapter.h"
#include "adapters/LockFreeEventQueue.h"

// Include RtMidi without namespace conflicts
#include <RtMidi.h>

namespace music::adapters {

class RtMidiAdapter : public ports::MidiPort, public MusicTrainer::ports::MidiAdapter {
public:
    static std::unique_ptr<RtMidiAdapter> create(size_t portNumber = 0);
    
    // Implementation of both interfaces
    bool open() override;
    void close() override;
    bool isOpen() const override;
    void sendEvent(const ports::MidiEvent& event) override;
    void setEventCallback(std::function<void(const ports::MidiEvent&)> callback) override;
    ports::MidiPortMetrics getMetrics() const override;
    void resetMetrics() override;
    
    // Implementation of device enumeration methods from MidiAdapter interface
    std::vector<std::string> getAvailableInputs() const override;
    std::vector<std::string> getAvailableOutputs() const override;
    
    // Implementation of device configuration methods from MidiAdapter interface
    bool setInputDevice(int index) override {
        if (index < 0 || static_cast<size_t>(index) >= midiIn->getPortCount()) {
            return false;
        }
        try {
            if (isOpen()) {
                midiIn->closePort();
            }
            midiIn->openPort(index);
            currentInputDevice.store(index, std::memory_order_release);
            return true;
        } catch (RtMidiError& error) {
            return false;
        }
    }
    
    bool setOutputDevice(int index) override {
        if (index < 0 || static_cast<size_t>(index) >= midiOut->getPortCount()) {
            return false;
        }
        try {
            if (isOpen()) {
                midiOut->closePort();
            }
            midiOut->openPort(index);
            currentOutputDevice.store(index, std::memory_order_release);
            return true;
        } catch (RtMidiError& error) {
            return false;
        }
    }
    
    void setMidiThrough(bool enabled) override {
        midiThrough.store(enabled, std::memory_order_release);
    }
    
    void setLatency(int latencyMs) override {
        latencyMilliseconds.store(latencyMs, std::memory_order_release);
    }
    
    int getCurrentInputDevice() const override {
        return currentInputDevice.load(std::memory_order_acquire);
    }
    
    int getCurrentOutputDevice() const override {
        return currentOutputDevice.load(std::memory_order_acquire);
    }
    
    bool getMidiThrough() const override {
        return midiThrough.load(std::memory_order_acquire);
    }
    
    int getLatency() const override {
        return latencyMilliseconds.load(std::memory_order_acquire);
    }
    
    ~RtMidiAdapter() override;

private:
    RtMidiAdapter(size_t portNumber);
    
    // MIDI device handling
    std::unique_ptr<RtMidiIn> midiIn;
    std::unique_ptr<RtMidiOut> midiOut;
    size_t portNumber;
    std::atomic<bool> isRunning{false};
    
    // Device configuration state
    std::atomic<int> currentInputDevice{-1};
    std::atomic<int> currentOutputDevice{-1};
    std::atomic<bool> midiThrough{false};
    std::atomic<int> latencyMilliseconds{0};
    
    // Event processing
    LockFreeEventQueue<ports::MidiEvent, 1024> eventQueue;
    std::thread processingThread;
    std::function<void(const ports::MidiEvent&)> eventCallback;
    
    // Metrics
    struct {
        std::atomic<size_t> totalEvents{0};
        std::atomic<size_t> errorCount{0};
        std::atomic<size_t> recoveredErrors{0};
        std::atomic<double> maxLatencyUs{0.0};
        std::atomic<std::chrono::system_clock::time_point::rep> lastEventTime{
            std::chrono::system_clock::now().time_since_epoch().count()
        };
    } metrics;

    // Enhanced error tracking
    struct ErrorState {
        std::atomic<bool> inErrorState{false};
        std::atomic<size_t> consecutiveErrors{0};
        std::atomic<size_t> errorWindowCount{0};
        std::atomic<std::chrono::system_clock::time_point::rep> lastErrorTime{
            std::chrono::system_clock::now().time_since_epoch().count()
        };
        std::atomic<std::chrono::system_clock::time_point::rep> errorWindowStart{
            std::chrono::system_clock::now().time_since_epoch().count()
        };
        static constexpr size_t MAX_CONSECUTIVE_ERRORS = 3;
        static constexpr size_t ERROR_WINDOW_THRESHOLD = 5;
        static constexpr auto ERROR_WINDOW_DURATION = std::chrono::seconds(60);
    } errorState;
    
    // Processing methods
    void startProcessing();
    void stopProcessing();
    void processEvents();
    static void handleError(RtMidiError::Type type, const std::string& errorText, void* userData);
    static void staticCallback(double timestamp, std::vector<unsigned char>* message, void* userData);
    void processMessage(double timestamp, std::vector<unsigned char>* message);

    // Recovery methods
    bool attemptRecovery(RtMidiError::Type errorType);
    bool performSoftReset();
    bool performHardReset();
    void updateErrorMetrics(RtMidiError::Type errorType);
    bool shouldAttemptRecovery() const;
    
    // Error tracking methods 
    void clearErrorState();
    bool isInErrorWindow() const;
    void recordError();
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_RTMIDIADAPTER_H