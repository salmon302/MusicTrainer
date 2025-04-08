#ifndef MUSICTRAINERV3_MOCKMIDIADAPTER_H
#define MUSICTRAINERV3_MOCKMIDIADAPTER_H

#include <memory>
#include <atomic>
#include <vector>
#include <deque>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "domain/ports/MidiPort.h"
#include "domain/ports/MidiAdapter.h"
#include "domain/errors/ErrorBase.h"

namespace music::adapters {

class MidiError : public MusicTrainer::MusicTrainerError {
public:
	explicit MidiError(const std::string& message) 
		: MusicTrainer::MusicTrainerError(message) {}
};

class MockMidiAdapter : public ports::MidiPort, public MusicTrainer::ports::MidiAdapter {
public:
	static std::unique_ptr<MockMidiAdapter> create();
	~MockMidiAdapter() override {
		if (isRunning.load()) {
			close();
		}
	}
	
	bool open() override;
	void close() override;
	bool isOpen() const override;
	void sendEvent(const ports::MidiEvent& event) override;
	void setEventCallback(std::function<void(const ports::MidiEvent&)> callback) override;
	ports::MidiPortMetrics getMetrics() const override;
	void resetMetrics() override;
	
	// Implementation of MidiAdapter interface methods
	std::vector<std::string> getAvailableInputs() const override;
	std::vector<std::string> getAvailableOutputs() const override;
	
	// Add missing MidiAdapter interface implementations
	bool setInputDevice(int index) override {
		currentInputDevice.store(index, std::memory_order_release);
		return true; // Mock always succeeds
	}
	
	bool setOutputDevice(int index) override {
		currentOutputDevice.store(index, std::memory_order_release);
		return true; // Mock always succeeds
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
	
	void setSimulateErrors(bool simulate) { simulateErrors.store(simulate, std::memory_order_release); }
	void clearEvents() { 
	    std::lock_guard<std::mutex> lock(queueMutex);
	    eventQueue.clear(); 
	}
	
private:
	MockMidiAdapter();
	
	std::atomic<bool> isRunning{false};
	std::atomic<bool> simulateErrors{false};
	std::atomic<int> currentInputDevice{-1};
	std::atomic<int> currentOutputDevice{-1};
	std::atomic<bool> midiThrough{false};
	std::atomic<int> latencyMilliseconds{0};
	
	std::deque<ports::MidiEvent> eventQueue;
	std::mutex queueMutex;
	std::condition_variable eventsAvailable;
	std::mutex callbackMutex;
	std::function<void(const ports::MidiEvent&)> eventCallback;
	std::thread processingThread;
	
	struct {
		std::atomic<size_t> totalEvents{0};
		std::atomic<size_t> errorCount{0};
		std::atomic<size_t> recoveredErrors{0};
		std::atomic<double> maxLatencyUs{0.0};
		std::atomic<long long> lastEventTime{0};
	} metrics;
	
	void simulateError();
	void processEvents();
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_MOCKMIDIADAPTER_H



