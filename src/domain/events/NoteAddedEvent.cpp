#include "domain/events/NoteAddedEvent.h"
#include "domain/errors/DomainErrors.h"
#include <sstream>
#include <iostream>

namespace music::events {

std::unique_ptr<NoteAddedEvent> NoteAddedEvent::create(
	size_t voiceIndex,
	const MusicTrainer::music::Pitch& pitch,
	const MusicTrainer::music::Duration& duration,
	const MusicTrainer::music::Voice::TimeSignature& timeSignature,
	const std::string& correlationId
) {
	return std::unique_ptr<NoteAddedEvent>(
		new NoteAddedEvent(voiceIndex, pitch, duration, timeSignature, correlationId)
	);
}

NoteAddedEvent::NoteAddedEvent(
	size_t voiceIndex,
	const MusicTrainer::music::Pitch& pitch,
	const MusicTrainer::music::Duration& duration,
	const MusicTrainer::music::Voice::TimeSignature& timeSignature,
	const std::string& correlationId
)
	: voiceIndex(voiceIndex)
	, pitch(pitch)
	, duration(duration)
	, timeSignature(timeSignature)
{
	setCorrelationId(correlationId);
	timestamp.store(
		std::chrono::system_clock::now().time_since_epoch().count(),
		std::memory_order_release
	);
}

NoteAddedEvent::NoteAddedEvent(const NoteAddedEvent& other) 
	: voiceIndex(other.voiceIndex.load(std::memory_order_acquire))
	, pitch(other.pitch)
	, duration(other.duration)
	, timeSignature(other.timeSignature)
{
	timestamp.store(
		other.timestamp.load(std::memory_order_acquire),
		std::memory_order_release
	);
	setCorrelationId(other.getCorrelationId());
}

NoteAddedEvent& NoteAddedEvent::operator=(const NoteAddedEvent& other) {
	if (this != &other) {
		voiceIndex.store(
			other.voiceIndex.load(std::memory_order_acquire),
			std::memory_order_release
		);
		timestamp.store(
			other.timestamp.load(std::memory_order_acquire),
			std::memory_order_release
		);
		// These are immutable, so direct assignment is safe
		const_cast<MusicTrainer::music::Pitch&>(pitch) = other.pitch;
		const_cast<MusicTrainer::music::Duration&>(duration) = other.duration;
		const_cast<MusicTrainer::music::Voice::TimeSignature&>(timeSignature) = other.timeSignature;
		setCorrelationId(other.getCorrelationId());
	}
	return *this;
}

void NoteAddedEvent::apply(MusicTrainer::music::Score& score) const {
	auto currentVoiceIndex = voiceIndex.load(std::memory_order_acquire);
	
	std::cout << "[NoteAddedEvent::apply] Starting to apply event to score. Voice index: " << currentVoiceIndex << std::endl;
	
	try {
		if (currentVoiceIndex >= score.getVoiceCount()) {
			std::cout << "[NoteAddedEvent::apply] Creating new voice" << std::endl;
			auto voice = MusicTrainer::music::Voice::create(timeSignature);
			if (!voice) {
				throw MusicTrainer::RepositoryError("Failed to create voice");
			}
			voice->addNote(pitch, duration);
			score.addVoice(std::move(voice));
		} else {
			std::cout << "[NoteAddedEvent::apply] Adding note to existing voice" << std::endl;
			auto* voice = score.getVoice(currentVoiceIndex);
			if (!voice) {
				throw MusicTrainer::RepositoryError("Voice not found at index " + std::to_string(currentVoiceIndex));
			}
			voice->addNote(pitch, duration);
		}
		std::cout << "[NoteAddedEvent::apply] Successfully applied event" << std::endl;
	} catch (const std::exception& e) {
		std::cout << "[NoteAddedEvent::apply] Failed to apply event: " << e.what() << std::endl;
		throw MusicTrainer::RepositoryError("Failed to apply NoteAddedEvent: " + std::string(e.what()));
	}
}

std::string NoteAddedEvent::getDescription() const {
	std::stringstream ss;
	ss << "Added note " << pitch.toString() << " with duration " 
	   << duration.toString() << " to voice " << voiceIndex.load(std::memory_order_acquire);
	return ss.str();
}

std::unique_ptr<Event> NoteAddedEvent::clone() const {
	return create(
		voiceIndex.load(std::memory_order_acquire),
		pitch,
		duration,
		timeSignature,
		getCorrelationId()
	);
}

} // namespace music::events
