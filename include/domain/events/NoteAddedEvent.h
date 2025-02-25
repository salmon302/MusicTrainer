#ifndef MUSICTRAINERV3_NOTEADDEDEVENT_H
#define MUSICTRAINERV3_NOTEADDEDEVENT_H

#include "Event.h"
#include "../music/Pitch.h"
#include "../music/Duration.h"
#include "../music/Score.h"
#include <atomic>

namespace music::events {

class NoteAddedEvent : public Event {
public:
	static constexpr const char* TYPE = "NoteAdded";
	
	static std::unique_ptr<NoteAddedEvent> create(
		size_t voiceIndex,
		const Pitch& pitch,
		const Duration& duration,
		const Voice::TimeSignature& timeSignature,
		const std::string& correlationId = ""
	);
	
	// Copy operations
	NoteAddedEvent(const NoteAddedEvent& other);
	NoteAddedEvent& operator=(const NoteAddedEvent& other);
	
	// Delete move operations
	NoteAddedEvent(NoteAddedEvent&&) = delete;
	NoteAddedEvent& operator=(NoteAddedEvent&&) = delete;
	
	// Event interface implementation
	void apply(Score& score) const override;
	std::chrono::system_clock::time_point getTimestamp() const override { 
		return std::chrono::system_clock::time_point(
			std::chrono::system_clock::duration(
				timestamp.load(std::memory_order_acquire)
			)
		);
	}
	std::string getDescription() const override;
	std::unique_ptr<Event> clone() const override;
	std::string getType() const override { return TYPE; }
	
	// Event-specific accessors
	size_t getVoiceIndex() const { 
		return voiceIndex.load(std::memory_order_acquire);
	}
	const Pitch& getPitch() const { 
		return pitch;  // Pitch is immutable
	}
	const Duration& getDuration() const { 
		return duration;  // Duration is immutable
	}
	const Voice::TimeSignature& getTimeSignature() const { 
		return timeSignature;  // TimeSignature is immutable
	}

private:
	NoteAddedEvent(
		size_t voiceIndex, 
		const Pitch& pitch, 
		const Duration& duration,
		const Voice::TimeSignature& timeSignature,
		const std::string& correlationId
	);
	
	std::atomic<std::chrono::system_clock::time_point::rep> timestamp{
		std::chrono::system_clock::now().time_since_epoch().count()
	};
	std::atomic<size_t> voiceIndex;
	const Pitch pitch;  // Immutable after construction
	const Duration duration;  // Immutable after construction
	const Voice::TimeSignature timeSignature;  // Immutable after construction
};

} // namespace music::events

#endif // MUSICTRAINERV3_NOTEADDEDEVENT_H
