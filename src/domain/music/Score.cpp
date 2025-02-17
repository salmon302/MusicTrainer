#include "domain/music/Score.h"
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include "utils/TrackedLock.h"

namespace music {

Score::Score(const ScoreSnapshot& snapshot) 
	: timeSignature(snapshot.timeSignature)
	, measureCountValid(true) {
	for (const auto& notes : snapshot.voiceNotes) {
		auto voice = Voice::create(timeSignature);
		for (const auto& note : notes) {
			voice->addNote(note.pitch, note.duration);
		}
		voices.push_back(std::move(voice));
	}
}

std::unique_ptr<Score> Score::create(const Voice::TimeSignature& timeSignature) {
	return std::unique_ptr<Score>(new Score(timeSignature));
}

Score::Score(const Voice::TimeSignature& timeSignature) 
	: timeSignature(timeSignature)
	, cachedMeasureCount(0)
	, measureCountValid(false)
	, name("")
	, version(0) {}

Score::Score(const Score& other) : timeSignature(other.timeSignature) {
	// First get voice references under other's lock
	std::vector<const Voice*> voiceRefs;
	std::string otherName;
	uint64_t otherVersion;
	{
		::utils::TrackedSharedMutexLock otherLock(other.mutex_, "Score::mutex_", ::utils::LockLevel::SCORE);
		voiceRefs.reserve(other.voices.size());
		for (const auto& voice : other.voices) {
			if (voice) voiceRefs.push_back(voice.get());
		}
		otherName = other.name;
		otherVersion = other.version;
	}
	
	// Clone voices with proper lock ordering (level 0)
	std::vector<std::unique_ptr<Voice>> clonedVoices;
	clonedVoices.reserve(voiceRefs.size());
	for (const Voice* voice : voiceRefs) {
		if (voice) {
			::utils::TrackedUniqueLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE);
			clonedVoices.push_back(voice->clone());
		}
	}
	
	// Finally, update our members
	voices = std::move(clonedVoices);
	name = std::move(otherName);
	version = otherVersion;
}

Score& Score::operator=(const Score& other) {
	if (this != &other) {
		// First get voice references under other's lock
		std::vector<const Voice*> voiceRefs;
		std::string otherName;
		uint64_t otherVersion;
		Voice::TimeSignature otherTimeSignature;
		{
			::utils::TrackedSharedMutexLock otherLock(other.mutex_, "Score::mutex_", ::utils::LockLevel::SCORE);
			voiceRefs.reserve(other.voices.size());
			for (const auto& voice : other.voices) {
				if (voice) voiceRefs.push_back(voice.get());
			}
			otherName = other.name;
			otherVersion = other.version;
			otherTimeSignature = other.timeSignature;
		}
		
		// Clone voices with proper lock ordering (level 0)
		std::vector<std::unique_ptr<Voice>> clonedVoices;
		clonedVoices.reserve(voiceRefs.size());
		for (const Voice* voice : voiceRefs) {
			if (voice) {
				::utils::TrackedUniqueLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE);
				clonedVoices.push_back(voice->clone());
			}
		}
		
		// Finally, update our members under our lock
		{
			::utils::TrackedUniqueLock thisLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE);
			voices = std::move(clonedVoices);
			name = std::move(otherName);
			version = otherVersion;
			timeSignature = otherTimeSignature;
			measureCountValid = false;
		}
	}
	return *this;
}

size_t Score::getMeasureCount() const {
	// First get voice references under score lock
	std::vector<const Voice*> voiceRefs;
	bool needsRecalculation = false;
	size_t cachedValue = 0;
	
	{
		::utils::TrackedSharedMutexLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE);
		if (measureCountValid) {
			return cachedMeasureCount;
		}
		voiceRefs.reserve(voices.size());
		for (const auto& voice : voices) {
			if (voice) {
				voiceRefs.push_back(voice.get());
			}
		}
		needsRecalculation = !measureCountValid;
		cachedValue = cachedMeasureCount;
	}
	
	if (!needsRecalculation) {
		return cachedValue;
	}
	
	// Then acquire voice locks in order
	size_t maxMeasure = 0;
	for (const Voice* voice : voiceRefs) {
		::utils::TrackedUniqueLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE);
		for (size_t i = 0; i < voice->getNoteCount(); ++i) {
			maxMeasure = std::max(maxMeasure, voice->getMeasureForNote(i));
		}
	}
	
	// Finally update cache under score lock
	{
		::utils::TrackedUniqueLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE);
		cachedMeasureCount = maxMeasure + 1;
		measureCountValid = true;
		return cachedMeasureCount;
	}
}

size_t Score::getHash() const {
	std::vector<const Voice*> voiceRefs;
	Voice::TimeSignature ts;
	
	// First get voice references and time signature under score lock
	{
		::utils::TrackedSharedMutexLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE);
		ts = timeSignature;
		voiceRefs.reserve(voices.size());
		for (const auto& voice : voices) {
			if (voice) {
				voiceRefs.push_back(voice.get());
			}
		}
	}
	
	// Calculate initial hash from time signature
	size_t hash = std::hash<uint8_t>{}(ts.beats);
	hash ^= std::hash<double>{}(ts.beatUnit.getTotalBeats());
	
	// Then acquire voice locks in order and update hash
	for (const Voice* voice : voiceRefs) {
		::utils::TrackedUniqueLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE);
		hash ^= voice->getHash() + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
	
	return hash;
}

} // namespace music


