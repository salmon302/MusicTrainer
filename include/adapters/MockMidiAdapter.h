#ifndef MUSICTRAINERV3_MOCKMIDIADAPTER_H
#define MUSICTRAINERV3_MOCKMIDIADAPTER_H

#include <memory>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include "domain/ports/MidiPort.h"
#include "domain/errors/ErrorBase.h"
#include "adapters/LockFreeEventQueue.h"

namespace music::adapters {

class MidiError : public MusicTrainer::MusicTrainerError {
public:
	explicit MidiError(const std::string& message) 
		: MusicTrainer::MusicTrainerError(message) {}
};

class MockMidiAdapter : public ports::MidiPort {
public:
	static std::unique_ptr<MockMidiAdapter> create();
	~MockMidiAdapter() override { 
		if (isRunning) {
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
	
	void setSimulateErrors(bool simulate) { simulateErrors.store(simulate, std::memory_order_release); }
	void clearEvents() { eventQueue.clear(); }
	
private:
	MockMidiAdapter();
	
	struct EventWithPriority {
		ports::MidiEvent event;
		size_t sequence;
		
		EventWithPriority() : sequence(0) {}
		EventWithPriority(ports::MidiEvent e, size_t s) 
			: event(std::move(e)), sequence(s) {}
	};
	
	std::atomic<bool> isRunning{false};
	std::atomic<bool> simulateErrors{false};
	LockFreeEventQueue<ports::MidiEvent, 1024> eventQueue;
	std::atomic<size_t> sequenceNumber{0};
	std::function<void(const ports::MidiEvent&)> eventCallback;
	std::thread processingThread;
	std::vector<EventWithPriority> pendingEvents;
	
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



