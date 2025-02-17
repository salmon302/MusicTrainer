#ifndef MUSICTRAINERV3_MOCKMIDIADAPTER_H
#define MUSICTRAINERV3_MOCKMIDIADAPTER_H

#include <memory>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include "domain/ports/MidiPort.h"
#include "domain/errors/ErrorBase.h"
#include "../utils/TrackedLock.h"

namespace music::adapters {

class MidiError : public MusicTrainer::MusicTrainerError {
public:
	explicit MidiError(const std::string& message) 
		: MusicTrainer::MusicTrainerError(message) {}
};

class MockMidiAdapter : public ports::MidiPort {
public:
	static std::unique_ptr<MockMidiAdapter> create();
	~MockMidiAdapter() override = default;
	
	bool open() override;
	void close() override;
	bool isOpen() const override;
	void sendEvent(const ports::MidiEvent& event) override;
	void setEventCallback(std::function<void(const ports::MidiEvent&)> callback) override;
	ports::MidiPortMetrics getMetrics() const override;
	void resetMetrics() override;
	
	// Test control methods
	void setSimulateErrors(bool simulate) { simulateErrors = simulate; }
	void clearEvents() { 
		::utils::TrackedUniqueLock lock(callbackMutex, "MockMidiAdapter::callbackMutex", ::utils::LockLevel::REPOSITORY);
		pendingEvents.clear();
	}
	
private:
	MockMidiAdapter() = default;
	
	struct EventWithPriority {
		ports::MidiEvent event;
		size_t sequence;
		
		EventWithPriority(ports::MidiEvent e, size_t s) 
			: event(std::move(e)), sequence(s) {}
	};
	
	std::atomic<bool> isRunning{false};
	std::atomic<bool> simulateErrors{false};
	std::vector<EventWithPriority> pendingEvents;

	std::function<void(const ports::MidiEvent&)> eventCallback;
	std::shared_mutex callbackMutex;
	size_t sequenceNumber{0};
	
	struct {
		std::atomic<size_t> totalEvents{0};
		std::atomic<size_t> errorCount{0};
		std::atomic<size_t> recoveredErrors{0};
		std::atomic<double> maxLatencyUs{0.0};
		std::chrono::system_clock::time_point lastEventTime;
	} metrics;
	
	void simulateError();
	void processEvents();  // Kept for backward compatibility
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_MOCKMIDIADAPTER_H


