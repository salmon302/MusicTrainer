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
#include <mutex>
#include <condition_variable>

using namespace MusicTrainer;

namespace music::adapters {

std::unique_ptr<MockMidiAdapter> MockMidiAdapter::create() {
	return std::unique_ptr<MockMidiAdapter>(new MockMidiAdapter());
}

MockMidiAdapter::MockMidiAdapter() {
	isRunning = false; // Initialize before starting thread
	processingThread = std::thread(&MockMidiAdapter::processEvents, this);
}

bool MockMidiAdapter::open() {
	isRunning = true;
	return true;
}

void MockMidiAdapter::close() {
	if (!isRunning) return; // Avoid double close
	
	isRunning = false;
	{
	    std::lock_guard<std::mutex> lock(queueMutex);
	    eventsAvailable.notify_one(); // Wake up the processing thread
	}
    
    // Only join if the thread is joinable
    if (processingThread.joinable()) {
        processingThread.join();
    }
}

bool MockMidiAdapter::isOpen() const {
	return isRunning;
}

void MockMidiAdapter::sendEvent(const ports::MidiEvent& event) {
	if (!isOpen()) return;
	
	// We need to let non-recoverable errors propagate up to the test
	// So we'll only catch and handle recoverable ones
	
	// First check if we're simulating errors
	if (simulateErrors) {
		// This will throw for non-recoverable errors
		simulateError();
	}
	
	// If we get here, either we're not simulating errors,
	// or the simulated error was recoverable
	try {
		auto start = std::chrono::high_resolution_clock::now();
		
		std::cout << "Adding event type " << static_cast<int>(event.type) << std::endl;
		
		// Update metrics safely
		metrics.totalEvents++;
		
		// Add event to queue with sequential order
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			// Create a copy of the event to avoid lifetime issues
			eventQueue.push_back(event);
			eventsAvailable.notify_one();
		}
		
		auto end = std::chrono::high_resolution_clock::now();
		auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		metrics.maxLatencyUs = std::max(metrics.maxLatencyUs.load(), static_cast<double>(latency));
		metrics.lastEventTime.store(std::chrono::system_clock::now().time_since_epoch().count(), std::memory_order_release);
	} catch (const std::exception& e) {
		// Log error but don't let it crash the application
		std::cerr << "Error in sendEvent processing: " << e.what() << std::endl;
		metrics.errorCount++;
	}
}

void MockMidiAdapter::setEventCallback(std::function<void(const ports::MidiEvent&)> callback) {
	std::lock_guard<std::mutex> lock(callbackMutex);
	eventCallback = std::move(callback);
}


ports::MidiPortMetrics MockMidiAdapter::getMetrics() const {
	ports::MidiPortMetrics result;
	result.totalEvents = metrics.totalEvents;
	result.errorCount = metrics.errorCount;
	result.recoveredErrors = metrics.recoveredErrors;
	result.maxLatencyUs = metrics.maxLatencyUs;
	// Construct time_point correctly by adding duration to epoch and casting
	   auto ns_count_since_epoch = metrics.lastEventTime.load(std::memory_order_acquire);
	   std::chrono::nanoseconds duration_since_epoch(ns_count_since_epoch);
	   auto precise_time_point = std::chrono::system_clock::time_point() + duration_since_epoch;
	   // Cast to the required duration type of system_clock::time_point
	   result.lastEventTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(precise_time_point);
	return result;
}

void MockMidiAdapter::resetMetrics() {
	metrics.totalEvents = 0;
	metrics.errorCount = 0;
	metrics.recoveredErrors = 0;
	metrics.maxLatencyUs = 0.0;
	metrics.lastEventTime = std::chrono::system_clock::now().time_since_epoch().count();
}

std::vector<std::string> MockMidiAdapter::getAvailableInputs() const {
	// Return mock MIDI input devices
	return {
		"Mock MIDI Input 1",
		"Mock MIDI Input 2"
	};
}

std::vector<std::string> MockMidiAdapter::getAvailableOutputs() const {
	// Return mock MIDI output devices
	return {
		"Mock MIDI Output 1",
		"Mock MIDI Output 2"
	};
}

void MockMidiAdapter::simulateError() {
	metrics.errorCount++;
	
	// First two errors are always recoverable
	if (metrics.errorCount <= 2) {
		metrics.recoveredErrors++;
		return;
	}
	
	// After that, throw unrecoverable errors
	// Use only one error type for consistency - use RepositoryError since tests expect it
	MusicTrainer::RepositoryError error("Simulated MIDI error as Repository Error #" + std::to_string(metrics.errorCount), "Failed to access repository");
	
	std::cout << "Simulating error and explicitly calling the error handler..." << std::endl;
	
	try {
		// Force synchronous execution of the error handler
		auto future = MusicTrainer::ErrorHandler::getInstance().handleError(error);
		future.wait();
		
		// If we need to test specific error types, we can do that here
		std::cout << "After error handler called. Now throwing the original error." << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Error in error handler: " << e.what() << std::endl;
	}
	
	// Always throw the same error object that was handled
	throw error;
}

void MockMidiAdapter::processEvents() {
	while (true) {
		ports::MidiEvent event;
		bool hasEvent = false;
		
		{
		    std::unique_lock<std::mutex> lock(queueMutex);
		    // First check if we should exit
		    if (!isRunning && eventQueue.empty()) {
		        break;
		    }
		    
		    if (eventQueue.empty()) {
		        // Wait for an event with a timeout
		        eventsAvailable.wait_for(lock, std::chrono::milliseconds(10), 
		            [this] { return !eventQueue.empty() || !isRunning; });
		        
		        // Re-check condition after wait
		        if (!isRunning && eventQueue.empty()) {
		            break;
		        }
		        
		        if (eventQueue.empty()) {
		            continue;
		        }
		    }
		    
		    // Get the next event (FIFO order)
		    event = eventQueue.front();
		    eventQueue.pop_front();
		    hasEvent = true;
		}
		
		// Process the event outside the lock
		if (hasEvent) {
		    try {
		        std::lock_guard<std::mutex> callbackLock(callbackMutex);
				if (eventCallback) {
					eventCallback(event);
				}
		    } catch (const std::exception& e) {
		        // Catch and log any exceptions in the callback
		        std::cerr << "Error in MIDI event callback: " << e.what() << std::endl;
		    }
		}
	}
}

} // namespace music::adapters


