#include "adapters/RtMidiAdapter.h"
#include "domain/errors/DomainErrors.h"
#include "domain/errors/ErrorHandler.h"
#include <sstream>
#include <iostream>
#include <future>

using namespace MusicTrainer;

namespace music::adapters {

std::unique_ptr<RtMidiAdapter> RtMidiAdapter::create(size_t portNumber) {
    try {
        return std::unique_ptr<RtMidiAdapter>(new RtMidiAdapter(portNumber));
    } catch (const std::exception& e) {
        std::cerr << "Failed to create RtMidiAdapter: " << e.what() << std::endl;
        return nullptr;
    }
}

RtMidiAdapter::RtMidiAdapter(size_t portNumber) : portNumber(portNumber) {
    bool initSuccess = false;
    std::string errorMsg;

    try {
        #ifdef __linux__
            // Try ALSA first as it's required
            try {
                midiIn = std::make_unique<RtMidiIn>(RtMidi::LINUX_ALSA);
                midiOut = std::make_unique<RtMidiOut>(RtMidi::LINUX_ALSA);
                initSuccess = true;
            } catch (RtMidiError& e) {
                errorMsg = "ALSA initialization failed: " + e.getMessage();
            }

            #ifndef RTMIDI_NO_JACK
            // Only try JACK if ALSA failed or if we want to prefer JACK
            if (!initSuccess || getenv("MUSICTRAINER_PREFER_JACK")) {
                try {
                    auto jackIn = std::make_unique<RtMidiIn>(RtMidi::UNIX_JACK);
                    auto jackOut = std::make_unique<RtMidiOut>(RtMidi::UNIX_JACK);
                    // If JACK initialization succeeded, use it
                    midiIn = std::move(jackIn);
                    midiOut = std::move(jackOut);
                    initSuccess = true;
                    errorMsg.clear();
                } catch (RtMidiError& e) {
                    if (!initSuccess) {
                        errorMsg += "\nJACK initialization failed: " + e.getMessage();
                    }
                }
            }
            #endif
        #else
            try {
                midiIn = std::make_unique<RtMidiIn>();
                midiOut = std::make_unique<RtMidiOut>();
                initSuccess = true;
            } catch (RtMidiError& e) {
                errorMsg = "MIDI initialization failed: " + e.getMessage();
            }
        #endif

        if (!initSuccess) {
            throw std::runtime_error("Failed to initialize MIDI: " + errorMsg);
        }

        // Set initial error callbacks
        if (midiIn) midiIn->setErrorCallback(&RtMidiAdapter::handleError, this);
        if (midiOut) midiOut->setErrorCallback(&RtMidiAdapter::handleError, this);

    } catch (const std::exception& e) {
        // Clean up on failure
        midiIn.reset();
        midiOut.reset();
        throw;
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

std::vector<std::string> RtMidiAdapter::getAvailableInputs() const {
    std::vector<std::string> inputs;
    if (!midiIn) return inputs;
    
    try {
        unsigned int portCount = midiIn->getPortCount();
        for (unsigned int i = 0; i < portCount; ++i) {
            inputs.push_back(midiIn->getPortName(i));
        }
    } catch (RtMidiError& error) {
        handleError(error.getType(), error.getMessage(), const_cast<RtMidiAdapter*>(this));
    }
    
    return inputs;
}

std::vector<std::string> RtMidiAdapter::getAvailableOutputs() const {
    std::vector<std::string> outputs;
    if (!midiOut) return outputs;
    
    try {
        unsigned int portCount = midiOut->getPortCount();
        for (unsigned int i = 0; i < portCount; ++i) {
            outputs.push_back(midiOut->getPortName(i));
        }
    } catch (RtMidiError& error) {
        handleError(error.getType(), error.getMessage(), const_cast<RtMidiAdapter*>(this));
    }
    
    return outputs;
}

} // namespace music::adapters