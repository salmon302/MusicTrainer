#include "adapters/MockMidiAdapter.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "utils/TrackedLock.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <future>

using namespace MusicTrainer;

namespace music::adapters {

std::unique_ptr<MockMidiAdapter> MockMidiAdapter::create() {
	return std::unique_ptr<MockMidiAdapter>(new MockMidiAdapter());
}

bool MockMidiAdapter::open() {
	isRunning = true;
	return true;
}

void MockMidiAdapter::close() {
	isRunning = false;
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
	
	{
		::utils::TrackedUniqueLock lock(callbackMutex, "MockMidiAdapter::callbackMutex", ::utils::LockLevel::ERROR_LOGGING);
		std::cout << "Adding event type " << static_cast<int>(event.type) << std::endl;
		
		metrics.totalEvents++;  // Increment total events for each event sent
		
		if (eventCallback) {
			if (event.type == ports::MidiEvent::Type::CONTROL_CHANGE) {
				// For CONTROL_CHANGE events, store them for later
				pendingEvents.push_back(EventWithPriority(event, sequenceNumber++));
			} else if (event.type == ports::MidiEvent::Type::NOTE_ON) {
				// For NOTE_ON events, process them first
				std::cout << "Processing event type: " << static_cast<int>(event.type) << std::endl;
				eventCallback(event);
				// Then process any pending CONTROL_CHANGE events
				for (const auto& evt : pendingEvents) {
					std::cout << "Processing event type: " << static_cast<int>(evt.event.type) << std::endl;
					eventCallback(evt.event);
				}
				pendingEvents.clear();
			} else {
				// For other types, process immediately
				std::cout << "Processing event type: " << static_cast<int>(event.type) << std::endl;
				eventCallback(event);
			}
		}
	}
	
	auto end = std::chrono::high_resolution_clock::now();
	auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	metrics.maxLatencyUs = std::max(metrics.maxLatencyUs.load(), static_cast<double>(latency));
	metrics.lastEventTime = std::chrono::system_clock::now();
}










void MockMidiAdapter::setEventCallback(std::function<void(const ports::MidiEvent&)> callback) {
	::utils::TrackedUniqueLock lock(callbackMutex, "MockMidiAdapter::callbackMutex", ::utils::LockLevel::ERROR_LOGGING);
	eventCallback = std::move(callback);
}

ports::MidiPortMetrics MockMidiAdapter::getMetrics() const {
	ports::MidiPortMetrics result;
	result.totalEvents = metrics.totalEvents;
	result.errorCount = metrics.errorCount;
	result.recoveredErrors = metrics.recoveredErrors;
	result.maxLatencyUs = metrics.maxLatencyUs;
	result.lastEventTime = metrics.lastEventTime;
	return result;
}

void MockMidiAdapter::resetMetrics() {
	metrics.totalEvents = 0;
	metrics.errorCount = 0;
	metrics.recoveredErrors = 0;
	metrics.maxLatencyUs = 0.0;
	metrics.lastEventTime = std::chrono::system_clock::now();
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


} // namespace music::adapters


