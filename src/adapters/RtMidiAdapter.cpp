#include "adapters/RtMidiAdapter.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include "utils/TrackedLock.h"
#include <sstream>
#include <iostream>
#include <future>

using namespace MusicTrainer;
using namespace utils;

namespace music::adapters {

std::unique_ptr<RtMidiAdapter> RtMidiAdapter::create(size_t portNumber) {
	return std::unique_ptr<RtMidiAdapter>(new RtMidiAdapter(portNumber));
}

RtMidiAdapter::RtMidiAdapter(size_t portNumber) : portNumber(portNumber) {
	try {
		midiIn = std::make_unique<RtMidiIn>();
		midiOut = std::make_unique<RtMidiOut>();
	} catch (RtMidiError& error) {
		handleError(error.getType(), error.getMessage(), this);
	}
}


RtMidiAdapter::~RtMidiAdapter() {
	close();
}

bool RtMidiAdapter::open() {
	const int maxRetries = 3;
	int retryCount = 0;
	
	while (retryCount < maxRetries) {
		try {
			if (midiIn->getPortCount() > portNumber) {
				midiIn->openPort(portNumber);
				midiIn->setCallback(&RtMidiAdapter::staticCallback, this);
				midiIn->setErrorCallback(&RtMidiAdapter::handleError, this);
			}
			
			if (midiOut->getPortCount() > portNumber) {
				midiOut->openPort(portNumber);
			}
			
			isRunning = true;
			startProcessing();
			return true;
		} catch (RtMidiError& error) {
			handleError(error.getType(), error.getMessage(), this);
			retryCount++;
			
			if (retryCount < maxRetries) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100 * retryCount));
				try {
					midiIn = std::make_unique<RtMidiIn>();
					midiOut = std::make_unique<RtMidiOut>();
				} catch (RtMidiError& e) {
					handleError(e.getType(), e.getMessage(), this);
				}
			}
		}
	}
	return false;
}


void RtMidiAdapter::close() {
	stopProcessing();
	if (midiIn) midiIn->closePort();
	if (midiOut) midiOut->closePort();
}

void RtMidiAdapter::startProcessing() {
	isRunning = true;
	processingThread = std::thread(&RtMidiAdapter::processEvents, this);
}

void RtMidiAdapter::stopProcessing() {
	isRunning = false;
	if (processingThread.joinable()) {
		processingThread.join();
	}
}

void RtMidiAdapter::processEvents() {
	while (isRunning) {
		if (auto entry = eventQueue.pop()) {
			::utils::TrackedUniqueLock lock(callbackMutex, "RtMidiAdapter::callbackMutex", ::utils::LockLevel::REPOSITORY);
			if (eventCallback) {
				eventCallback(*entry);
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}


bool RtMidiAdapter::isOpen() const {
	return isRunning && midiIn && midiOut;
}

void RtMidiAdapter::sendEvent(const ports::MidiEvent& event) {
	if (!isOpen()) return;
	
	try {
		midiOut->sendMessage(&event.message);
	} catch (RtMidiError& error) {
		handleError(error.getType(), error.getMessage(), this);
	}
}


void RtMidiAdapter::setEventCallback(std::function<void(const ports::MidiEvent&)> callback) {
	::utils::TrackedUniqueLock lock(callbackMutex, "RtMidiAdapter::callbackMutex", ::utils::LockLevel::REPOSITORY);
	eventCallback = std::move(callback);
}

void RtMidiAdapter::handleError(RtMidiError::Type type, const std::string& errorText, void* userData) {
	auto* adapter = static_cast<RtMidiAdapter*>(userData);
	if (!adapter) return;
	
	adapter->metrics.errorCount++;
	
	// Create error message with timestamp
	std::stringstream ss;
	ss << "[" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
	   << "] MIDI Error: " << errorText;
	
	// Attempt recovery based on error type
	if (type == RtMidiError::Type::INVALID_DEVICE || 
		type == RtMidiError::Type::DRIVER_ERROR) {
		try {
			adapter->close();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (adapter->open()) {
				adapter->metrics.recoveredErrors++;
				return;
			}
		} catch (...) {
			// Recovery failed
		}
	}
	
	// Create and handle domain-specific error
	MidiError midiError(ss.str(), "MidiError");
	DomainError error(midiError.what(), "MidiError");
	HANDLE_ERROR(error);
}


void RtMidiAdapter::staticCallback(double timestamp, std::vector<unsigned char>* message, void* userData) {
	auto* adapter = static_cast<RtMidiAdapter*>(userData);
	if (adapter) {
		adapter->processMessage(timestamp, message);
	}
}

void RtMidiAdapter::processMessage(double timestamp, std::vector<unsigned char>* message) {
	if (!message) return;
	
	// Create MidiEvent from raw message
	ports::MidiEvent event(*message);
	event.timestamp = timestamp;
	
	{
		::utils::TrackedUniqueLock lock(callbackMutex, "RtMidiAdapter::callbackMutex", ::utils::LockLevel::REPOSITORY);
		if (eventCallback) {
			eventCallback(event);
		}
	}
}


ports::MidiPortMetrics RtMidiAdapter::getMetrics() const {
    ports::MidiPortMetrics result;
    result.totalEvents = metrics.totalEvents;
    result.errorCount = metrics.errorCount;
    result.recoveredErrors = metrics.recoveredErrors;
    result.maxLatencyUs = metrics.maxLatencyUs;
    result.lastEventTime = metrics.lastEventTime;
    return result;
}

void RtMidiAdapter::resetMetrics() {
    metrics.totalEvents = 0;
    metrics.errorCount = 0;
    metrics.recoveredErrors = 0;
    metrics.maxLatencyUs = 0.0;
    metrics.lastEventTime = std::chrono::system_clock::now();
}

} // namespace music::adapters