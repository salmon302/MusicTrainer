#include "domain/events/NoteAddedEvent.h"
#include "domain/errors/DomainErrors.h"
#include "utils/TrackedLock.h"
#include <sstream>
#include <iostream>

namespace music::events {

std::unique_ptr<NoteAddedEvent> NoteAddedEvent::create(
	size_t voiceIndex,
	const Pitch& pitch,
	const Duration& duration,
	const Voice::TimeSignature& timeSignature,
	const std::string& correlationId
) {
	return std::unique_ptr<NoteAddedEvent>(
		new NoteAddedEvent(voiceIndex, pitch, duration, timeSignature, correlationId)
	);
}

NoteAddedEvent::NoteAddedEvent(
	size_t voiceIndex,
	const Pitch& pitch,
	const Duration& duration,
	const Voice::TimeSignature& timeSignature,
	const std::string& correlationId
)
	: voiceIndex(voiceIndex)
	, pitch(pitch)
	, duration(duration)
	, timeSignature(timeSignature)
{
	setCorrelationId(correlationId);
	timestamp = std::chrono::system_clock::now();
}

NoteAddedEvent::NoteAddedEvent(const NoteAddedEvent& other) 
    : voiceIndex(other.voiceIndex)
    , pitch(other.pitch)
    , duration(other.duration)
    , timeSignature(other.timeSignature)
    , timestamp(other.timestamp)
{
    setCorrelationId(other.getCorrelationId());
}

NoteAddedEvent& NoteAddedEvent::operator=(const NoteAddedEvent& other) {
if (this != &other) {
	::utils::TrackedUniqueLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
        voiceIndex = other.voiceIndex;
        pitch = other.pitch;
        duration = other.duration;
        timeSignature = other.timeSignature;
        timestamp = other.timestamp;
        setCorrelationId(other.getCorrelationId());
    }
    return *this;
}

void NoteAddedEvent::apply(Score& score) const {
	// Get all needed data under lock first
	size_t currentVoiceIndex;
	Pitch currentPitch;
	Duration currentDuration;
	Voice::TimeSignature currentTimeSignature;
	{
		::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
		currentVoiceIndex = voiceIndex;
		currentPitch = pitch;
		currentDuration = duration;
		currentTimeSignature = timeSignature;
	}
	
	std::cout << "[NoteAddedEvent::apply] Starting to apply event to score. Voice index: " << currentVoiceIndex << std::endl;
	
	try {
		if (currentVoiceIndex >= score.getVoiceCount()) {
			std::cout << "[NoteAddedEvent::apply] Creating new voice" << std::endl;
			auto voice = Voice::create(currentTimeSignature);
			if (!voice) {
				throw MusicTrainer::RepositoryError("Failed to create voice");
			}
			voice->addNote(currentPitch, currentDuration);
			score.addVoice(std::move(voice));
		} else {
			std::cout << "[NoteAddedEvent::apply] Adding note to existing voice" << std::endl;
			auto* voice = score.getVoice(currentVoiceIndex);
			if (!voice) {
				throw MusicTrainer::RepositoryError("Voice not found at index " + std::to_string(currentVoiceIndex));
			}
			voice->addNote(currentPitch, currentDuration);
		}
		std::cout << "[NoteAddedEvent::apply] Successfully applied event" << std::endl;
	} catch (const std::exception& e) {
		std::cout << "[NoteAddedEvent::apply] Failed to apply event: " << e.what() << std::endl;
		throw MusicTrainer::RepositoryError("Failed to apply NoteAddedEvent: " + std::string(e.what()));
	}
}

std::string NoteAddedEvent::getDescription() const {
	::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
	std::stringstream ss;
	ss << "Added note " << pitch.toString() << " with duration " 
	   << duration.toString() << " to voice " << voiceIndex;
	return ss.str();
}

std::unique_ptr<Event> NoteAddedEvent::clone() const {
	::utils::TrackedSharedMutexLock lock(mutex_, "NoteAddedEvent::mutex_", ::utils::LockLevel::REPOSITORY);
	return create(voiceIndex, pitch, duration, timeSignature, getCorrelationId());
}

} // namespace music::events
