#include "domain/music/Score.h"
#include <numeric>

namespace music {

Score::Score(const Voice::TimeSignature& timeSignature)
	: timeSignature(timeSignature) {}

std::unique_ptr<Score> Score::create(const Voice::TimeSignature& timeSignature) {
	return std::unique_ptr<Score>(new Score(timeSignature));
}

Score::Score(const ScoreSnapshot& snapshot)
	: timeSignature(snapshot.timeSignature) {
	for (const auto& voiceNotes : snapshot.voiceNotes) {
		auto voice = Voice::create(timeSignature);
		for (const auto& note : voiceNotes) {
			voice->addNote(note.pitch, note.duration);
		}
		voices.push_back(std::move(voice));
	}
}

Score::Score(const Score& other) {
	performCopy(other);
}

Score& Score::operator=(const Score& other) {
	if (this != &other) {
		performCopy(other);
	}
	return *this;
}

void Score::performCopy(const Score& other) {
	std::vector<std::unique_ptr<Voice>> newVoices;
	newVoices.reserve(other.voices.size());
	
	for (const auto& voice : other.voices) {
		if (voice) {
			newVoices.push_back(voice->clone());
		}
	}
	
	voices = std::move(newVoices);
	timeSignature = other.timeSignature;
	cachedMeasureCount = other.cachedMeasureCount;
	measureCountValid.store(other.measureCountValid.load(std::memory_order_acquire), std::memory_order_release);
	name = other.name;
	version.store(other.version.load(std::memory_order_acquire), std::memory_order_release);
}

size_t Score::getMeasureCount() const {
	if (measureCountValid.load(std::memory_order_acquire)) {
		return cachedMeasureCount;
	}

	size_t maxMeasure = 0;
	for (const auto& voice : voices) {
		if (!voice) continue;
		maxMeasure = std::max(maxMeasure, voice->getMeasureForNote(voice->getNoteCount() - 1));
	}

	cachedMeasureCount = maxMeasure + 1;
	measureCountValid.store(true, std::memory_order_release);
	return cachedMeasureCount;
}

size_t Score::getHash() const {
	size_t hash = 17;
	hash = hash * 31 + timeSignature.beats;
	hash = hash * 31 + static_cast<size_t>(timeSignature.beatUnit.getBaseType());
	for (const auto& voice : voices) {
		if (voice) {
			hash = hash * 31 + voice->getHash();
		}
	}
	return hash;
}

} // namespace music