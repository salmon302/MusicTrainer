#ifndef MUSICTRAINERV3_MIDIPORT_H
#define MUSICTRAINERV3_MIDIPORT_H

#include <functional>
#include <string>
#include <chrono>
#include <sstream>
#include <vector>

namespace music::ports {

struct MidiEvent {
	enum class Type {
		NOTE_ON,
		NOTE_OFF,
		CONTROL_CHANGE,
		PROGRAM_CHANGE,
		PITCH_BEND
	};

	Type type;
	unsigned char channel;
	unsigned char note;
	unsigned char velocity;
	std::vector<unsigned char> message;
	double timestamp;

	// Constructor for note events
	MidiEvent(Type t, unsigned char ch, unsigned char n, unsigned char vel)
		: type(t), channel(ch), note(n), velocity(vel), timestamp(0.0) {
		// Build MIDI message based on type
		switch (type) {
			case Type::NOTE_ON:
				message.push_back(0x90 | (channel & 0x0F));
				message.push_back(note & 0x7F);
				message.push_back(velocity & 0x7F);
				break;
			case Type::NOTE_OFF:
				message.push_back(0x80 | (channel & 0x0F));
				message.push_back(note & 0x7F);
				message.push_back(velocity & 0x7F);
				break;
			case Type::CONTROL_CHANGE:
				message.push_back(0xB0 | (channel & 0x0F));
				message.push_back(note & 0x7F);    // Control number
				message.push_back(velocity & 0x7F); // Control value
				break;
			default:
				break;
		}
	}

	// Constructor for raw message
	explicit MidiEvent(const std::vector<unsigned char>& msg) 
		: message(msg), timestamp(0.0) {
		if (!message.empty()) {
			channel = message[0] & 0x0F;
			unsigned char status = message[0] & 0xF0;
			
			if (status == 0x90 && message.size() >= 3) {
				type = Type::NOTE_ON;
				note = message[1];
				velocity = message[2];
			} else if (status == 0x80 && message.size() >= 3) {
				type = Type::NOTE_OFF;
				note = message[1];
				velocity = message[2];
			} else if (status == 0xB0 && message.size() >= 3) {
				type = Type::CONTROL_CHANGE;
				note = message[1];    // Control number
				velocity = message[2]; // Control value
			} else if (status == 0xE0 && message.size() >= 3) {
				type = Type::PITCH_BEND;
				note = message[1];    // LSB
				velocity = message[2]; // MSB
			}
		}
	}
};

struct MidiPortMetrics {
	size_t totalEvents{0};
	size_t errorCount{0};
	size_t recoveredErrors{0};
	double avgLatencyUs{0.0};
	double maxLatencyUs{0.0};
	size_t queueSize{0};
	size_t queueCapacity{0};
	std::chrono::system_clock::time_point lastEventTime{};
	
	std::string toString() const {
		std::stringstream ss;
		ss << "MIDI Port Metrics:\n"
		   << "Total Events: " << totalEvents << "\n"
		   << "Errors: " << errorCount << " (Recovered: " << recoveredErrors << ")\n"
		   << "Avg Latency: " << avgLatencyUs << "μs\n"
		   << "Max Latency: " << maxLatencyUs << "μs\n"
		   << "Queue: " << queueSize << "/" << queueCapacity;
		return ss.str();
	}
};

class MidiPort {
public:
	virtual ~MidiPort() = default;
	
	virtual bool open() = 0;
	virtual void close() = 0;
	virtual bool isOpen() const = 0;
	virtual void sendEvent(const MidiEvent& event) = 0;
	virtual void setEventCallback(std::function<void(const MidiEvent&)> callback) = 0;
	
	// New methods for metrics
	virtual MidiPortMetrics getMetrics() const = 0;
	virtual void resetMetrics() = 0;
};

} // namespace music::ports

#endif // MUSICTRAINERV3_MIDIPORT_H