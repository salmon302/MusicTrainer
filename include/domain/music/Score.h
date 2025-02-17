#ifndef MUSICTRAINERV3_SCORE_H
#define MUSICTRAINERV3_SCORE_H

#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include "Voice.h"
#include "../../utils/TrackedLock.h"

namespace music {

class Score {
public:
	struct ScoreSnapshot {
		std::vector<std::vector<Voice::Note>> voiceNotes;
		Voice::TimeSignature timeSignature;
	};

	static std::unique_ptr<Score> create(const Voice::TimeSignature& timeSignature = Voice::TimeSignature::commonTime());
	explicit Score(const ScoreSnapshot& snapshot);
	Score(const Score& other);
	Score& operator=(const Score& other);
	
	void addVoice(std::unique_ptr<Voice> voice) {
		if (!voice) return;
		
		// Set time signature without holding Score lock
		{
			::utils::TrackedUniqueLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE,
				std::chrono::milliseconds(1000));
			voice->setTimeSignature(timeSignature);
		}
		
		// Add to voices vector under Score lock
		{
			::utils::TrackedUniqueLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
				std::chrono::milliseconds(1000));
			voices.push_back(std::move(voice));
			measureCountValid = false;
		}
	}








	size_t getVoiceCount() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		return voices.size();
	}

	const Voice* getVoice(size_t index) const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		return index < voices.size() ? voices[index].get() : nullptr;
	}

	Voice* getVoice(size_t index) {
		::utils::TrackedSharedMutexLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		return index < voices.size() ? voices[index].get() : nullptr;
	}

	size_t getMeasureCount() const;
	
	ScoreSnapshot createSnapshot() const {
		ScoreSnapshot snapshot;
		std::vector<Voice*> voiceRefs;
		
		// Get voice references under brief Score lock
		{
			::utils::TrackedSharedMutexLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
				std::chrono::milliseconds(1000));
			snapshot.timeSignature = timeSignature;
			voiceRefs.reserve(voices.size());
			for (const auto& voice : voices) {
				if (voice) voiceRefs.push_back(voice.get());
			}
		}
		
		// Get voice data without holding Score lock
		snapshot.voiceNotes.reserve(voiceRefs.size());
		for (auto voice : voiceRefs) {
			if (!voice) continue;
			
			std::vector<Voice::Note> notes;
			{
				::utils::TrackedSharedMutexLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE,
					std::chrono::milliseconds(1000));
				notes = voice->getNotesInRange(0, SIZE_MAX);
			}
			snapshot.voiceNotes.push_back(std::move(notes));
		}
		
		return snapshot;
	}


	
	const Voice::TimeSignature& getTimeSignature() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		return timeSignature;
	}

	void setTimeSignature(const Voice::TimeSignature& newTimeSignature) {
		std::vector<Voice*> voiceRefs;
		
		// Get voice references under brief Score lock
		{
			::utils::TrackedSharedMutexLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
				std::chrono::milliseconds(1000));
			voiceRefs.reserve(voices.size());
			for (const auto& voice : voices) {
				if (voice) voiceRefs.push_back(voice.get());
			}
		}
		
		// Update voices without holding Score lock
		for (auto voice : voiceRefs) {
			if (!voice) continue;
			::utils::TrackedUniqueLock voiceLock(voice->getInternalMutex(), "Voice::mutex_", ::utils::LockLevel::VOICE,
				std::chrono::milliseconds(1000));
			voice->setTimeSignature(newTimeSignature);
		}
		
		// Update Score's time signature
		{
			::utils::TrackedUniqueLock scoreLock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
				std::chrono::milliseconds(1000));
			timeSignature = newTimeSignature;
		}
	}



	size_t getHash() const;
	
	std::string getName() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		return name;
	}

	void setName(const std::string& newName) {
		::utils::TrackedUniqueLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		name = newName;
	}

	uint64_t getVersion() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		return version;
	}

	void setVersion(uint64_t newVersion) {
		::utils::TrackedUniqueLock lock(mutex_, "Score::mutex_", ::utils::LockLevel::SCORE,
			std::chrono::milliseconds(1000));
		version = newVersion;
	}

	#ifdef TESTING
		std::shared_mutex& getInternalMutex() const { return mutex_; }
		std::shared_mutex& getMutexForTesting() const { return mutex_; }
	#endif

	private:
	explicit Score(const Voice::TimeSignature& timeSignature);
	
	void performCopy(const Score& other) {
		// Assuming this is called with proper external locking
		timeSignature = other.timeSignature;
		voices.clear();
		voices.reserve(other.voices.size());
		for (const auto& voice : other.voices) {
			if (voice) {
				voices.push_back(voice->clone());
			}
		}
		cachedMeasureCount = other.cachedMeasureCount;
		measureCountValid = other.measureCountValid;
		name = other.name;
		version = other.version;
	}

	std::vector<std::unique_ptr<Voice>> voices;
	Voice::TimeSignature timeSignature;
	mutable size_t cachedMeasureCount = 0;
	mutable bool measureCountValid = false;
	mutable std::shared_mutex mutex_;
	std::string name;
	uint64_t version = 0;
};

} // namespace music

#endif // MUSICTRAINERV3_SCORE_H
