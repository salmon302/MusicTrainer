#ifndef MUSICTRAINERV3_NOTEADDEDEVENT_H
#define MUSICTRAINERV3_NOTEADDEDEVENT_H

#include "Event.h"
#include "../music/Pitch.h"
#include "../music/Duration.h"
#include "../music/Score.h"
#include "../../utils/TrackedLock.h"

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
		::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
		return timestamp; 
	}
	std::string getDescription() const override;
	std::unique_ptr<Event> clone() const override;
	std::string getType() const override { return TYPE; }
	
	// Event-specific accessors
	size_t getVoiceIndex() const { 
		::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
		return voiceIndex; 
	}
	const Pitch& getPitch() const { 
		::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
		return pitch; 
	}
	const Duration& getDuration() const { 
		::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
		return duration; 
	}
	const Voice::TimeSignature& getTimeSignature() const { 
		::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
		return timeSignature; 
	}

private:
	NoteAddedEvent(
		size_t voiceIndex, 
		const Pitch& pitch, 
		const Duration& duration,
		const Voice::TimeSignature& timeSignature,
		const std::string& correlationId
	);
	
	std::chrono::system_clock::time_point timestamp;
	size_t voiceIndex;
	Pitch pitch;
	Duration duration;
	Voice::TimeSignature timeSignature;
};

} // namespace music::events

#endif // MUSICTRAINERV3_NOTEADDEDEVENT_H
