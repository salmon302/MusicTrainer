#include "adapters/MockMidiAdapter.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "adapters/LockFreeEventQueue.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <future>
#include <atomic>
#include <thread>

using namespace MusicTrainer;

namespace music::adapters {

std::unique_ptr<MockMidiAdapter> MockMidiAdapter::create() {
	return std::unique_ptr<MockMidiAdapter>(new MockMidiAdapter());
}

MockMidiAdapter::MockMidiAdapter() {
	processingThread = std::thread(&MockMidiAdapter::processEvents, this);
}

bool MockMidiAdapter::open() {
	isRunning = true;
	return true;
}

void MockMidiAdapter::close() {
	isRunning = false;
    processingThread.join();
}

bool MockMidiAdapter::isOpen() const {
	return isRunning;
}

void MockMidiAdapter::sendEvent(const ports::MidiEvent& event) {
	if (!isOpen()) return;
	
	if (simulateErrors) {
		simulateError();
	}
	
	auto start = std::chrono::high_resolution_clock::now();
	
	std::cout << "Adding event type " << static_cast<int>(event.type) << std::endl;
	
	metrics.totalEvents++;
	
	if (event.type == ports::MidiEvent::Type::CONTROL_CHANGE) {
		pendingEvents.push_back(EventWithPriority(event, sequenceNumber++));
	} else if (event.type == ports::MidiEvent::Type::NOTE_ON) {
		std::cout << "Processing event type: " << static_cast<int>(event.type) << std::endl;
		eventQueue.push(event);
		for (const auto& evt : pendingEvents) {
			std::cout << "Processing event type: " << static_cast<int>(evt.event.type) << std::endl;
			eventQueue.push(evt.event);
		}
		pendingEvents.clear();
	} else {
		std::cout << "Processing event type: " << static_cast<int>(event.type) << std::endl;
		eventQueue.push(event);
	}
	
	auto end = std::chrono::high_resolution_clock::now();
	auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	metrics.maxLatencyUs = std::max(metrics.maxLatencyUs.load(), static_cast<double>(latency));
	metrics.lastEventTime.store(std::chrono::system_clock::now().time_since_epoch().count(), std::memory_order_release);

}










void MockMidiAdapter::setEventCallback(std::function<void(const ports::MidiEvent&)> callback) {
	eventCallback = std::move(callback);
}


ports::MidiPortMetrics MockMidiAdapter::getMetrics() const {
	ports::MidiPortMetrics result;
	result.totalEvents = metrics.totalEvents;
	result.errorCount = metrics.errorCount;
	result.recoveredErrors = metrics.recoveredErrors;
	result.maxLatencyUs = metrics.maxLatencyUs;
	result.lastEventTime = std::chrono::system_clock::time_point(std::chrono::nanoseconds(metrics.lastEventTime.load()));
	return result;
}

void MockMidiAdapter::resetMetrics() {
	metrics.totalEvents = 0;
	metrics.errorCount = 0;
	metrics.recoveredErrors = 0;
	metrics.maxLatencyUs = 0.0;
	metrics.lastEventTime = std::chrono::system_clock::now().time_since_epoch().count();
}

void MockMidiAdapter::simulateError() {
	metrics.errorCount++;
	
	// First two errors are always recoverable
	if (metrics.errorCount <= 2) {
		metrics.recoveredErrors++;
		return;
	}
	
	// After that, throw unrecoverable errors
	MidiError midiError("Simulated MIDI error #" + std::to_string(metrics.errorCount));
	DomainError error(midiError.what(), "MidiError");
	HANDLE_ERROR(error);
	throw midiError; // Re-throw the original error
}


void MockMidiAdapter::processEvents() {
	while (isRunning) {
		std::optional<ports::MidiEvent> event = eventQueue.pop();
		if (event) {
			if (eventCallback) {
				eventCallback(*event);
			}
		} else {
			std::this_thread::yield();
		}
	}
}

} // namespace music::adapters


