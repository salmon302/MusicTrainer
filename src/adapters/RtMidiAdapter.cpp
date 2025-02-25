#include "adapters/RtMidiAdapter.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <sstream>
#include <iostream>
#include <future>

using namespace MusicTrainer;


namespace music::adapters {

std::unique_ptr<RtMidiAdapter> RtMidiAdapter::create(size_t portNumber) {
	return std::unique_ptr<RtMidiAdapter>(new RtMidiAdapter(portNumber));
}

RtMidiAdapter::RtMidiAdapter(size_t portNumber) : portNumber(portNumber) {
	try {
		// Try to create MIDI objects with specific API
		#ifdef __linux__
			// Try ALSA first as it's required
			midiIn = std::make_unique<RtMidiIn>(RtMidi::LINUX_ALSA);
			midiOut = std::make_unique<RtMidiOut>(RtMidi::LINUX_ALSA);
			
			#ifndef RTMIDI_NO_JACK
			// Only try JACK if it's available
			try {
				auto jackIn = std::make_unique<RtMidiIn>(RtMidi::UNIX_JACK);
				auto jackOut = std::make_unique<RtMidiOut>(RtMidi::UNIX_JACK);
				// If JACK initialization succeeded, use it instead
				midiIn = std::move(jackIn);
				midiOut = std::move(jackOut);
			} catch (RtMidiError& e) {
				// JACK not available, stick with ALSA
			}
			#endif
		#else
			midiIn = std::make_unique<RtMidiIn>();
			midiOut = std::make_unique<RtMidiOut>();
		#endif
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
	while (isRunning.load(std::memory_order_acquire)) {
		auto event = eventQueue.pop();
		if (event && eventCallback) {
			eventCallback(*event);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}



bool RtMidiAdapter::isOpen() const {
	return isRunning.load(std::memory_order_acquire) && midiIn && midiOut;
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
	eventCallback = std::move(callback);
}


void RtMidiAdapter::handleError(RtMidiError::Type type, const std::string& errorText, void* userData) {
	auto* adapter = static_cast<RtMidiAdapter*>(userData);
	if (!adapter) return;
	
	adapter->metrics.errorCount++;
	
	// Create error message with timestamp and backend info
	std::stringstream ss;
	ss << "[" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
	   << "] MIDI Error (" << (adapter->midiIn ? RtMidi::getApiName(adapter->midiIn->getCurrentApi()) : "unknown") 
	   << "): " << errorText;
	
	// Attempt recovery based on error type
	if (type == RtMidiError::Type::INVALID_DEVICE || 
		type == RtMidiError::Type::DRIVER_ERROR) {
		try {
			adapter->close();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			
			#ifdef __linux__
			// On Linux, try falling back to ALSA if using JACK
			if (adapter->midiIn && RtMidi::getApiName(adapter->midiIn->getCurrentApi()) == "UNIX_JACK") {
				adapter->midiIn = std::make_unique<RtMidiIn>(RtMidi::LINUX_ALSA);
				adapter->midiOut = std::make_unique<RtMidiOut>(RtMidi::LINUX_ALSA);
			}
			#endif
			
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
	
	ports::MidiEvent event(*message);
	event.timestamp = timestamp;
	eventQueue.push(std::move(event));
	
	metrics.totalEvents++;
	metrics.lastEventTime.store(
		std::chrono::system_clock::now().time_since_epoch().count(),
		std::memory_order_release);
}



ports::MidiPortMetrics RtMidiAdapter::getMetrics() const {
	ports::MidiPortMetrics result;
	result.totalEvents = metrics.totalEvents.load(std::memory_order_acquire);
	result.errorCount = metrics.errorCount.load(std::memory_order_acquire);
	result.recoveredErrors = metrics.recoveredErrors.load(std::memory_order_acquire);
	result.maxLatencyUs = metrics.maxLatencyUs.load(std::memory_order_acquire);
	result.lastEventTime = std::chrono::system_clock::time_point(
		std::chrono::system_clock::duration(
			metrics.lastEventTime.load(std::memory_order_acquire)));
	return result;
}

void RtMidiAdapter::resetMetrics() {
	metrics.totalEvents.store(0, std::memory_order_release);
	metrics.errorCount.store(0, std::memory_order_release);
	metrics.recoveredErrors.store(0, std::memory_order_release);
	metrics.maxLatencyUs.store(0.0, std::memory_order_release);
	metrics.lastEventTime.store(
		std::chrono::system_clock::now().time_since_epoch().count(),
		std::memory_order_release);
}

} // namespace music::adapters