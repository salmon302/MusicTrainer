#ifndef MUSICTRAINERV3_SCORE_H
#define MUSICTRAINERV3_SCORE_H

#include <vector>
#include <memory>
#include <atomic>
#include <algorithm>
#include "Voice.h"

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
		voice->setTimeSignature(timeSignature);
		voices.push_back(std::move(voice));
		measureCountValid.store(false, std::memory_order_release);
	}

	size_t getVoiceCount() const {
		return voices.size();
	}

	const Voice* getVoice(size_t index) const {
		return index < voices.size() ? voices[index].get() : nullptr;
	}

	Voice* getVoice(size_t index) {
		return index < voices.size() ? voices[index].get() : nullptr;
	}

	size_t getMeasureCount() const;
	
	ScoreSnapshot createSnapshot() const {
		ScoreSnapshot snapshot;
		snapshot.timeSignature = timeSignature;
		snapshot.voiceNotes.reserve(voices.size());
		
		for (const auto& voice : voices) {
			if (!voice) continue;
			snapshot.voiceNotes.push_back(voice->getNotesInRange(0, SIZE_MAX));
		}
		
		return snapshot;
	}
	
	const Voice::TimeSignature& getTimeSignature() const {
		return timeSignature;
	}

	void setTimeSignature(const Voice::TimeSignature& newTimeSignature) {
		for (auto& voice : voices) {
			if (!voice) continue;
			voice->setTimeSignature(newTimeSignature);
		}
		timeSignature = newTimeSignature;
	}

	size_t getHash() const;
	
	std::string getName() const {
		return name;
	}

	void setName(const std::string& newName) {
		name = newName;
	}

	uint64_t getVersion() const {
		return version.load(std::memory_order_acquire);
	}

	void setVersion(uint64_t newVersion) {
		version.store(newVersion, std::memory_order_release);
	}

private:
	explicit Score(const Voice::TimeSignature& timeSignature);
	void performCopy(const Score& other);

	std::vector<std::unique_ptr<Voice>> voices;

	Voice::TimeSignature timeSignature;
	mutable size_t cachedMeasureCount{0};
	mutable std::atomic<bool> measureCountValid{false};
	std::string name;
	std::atomic<uint64_t> version{0};
};

} // namespace music

#endif // MUSICTRAINERV3_SCORE_H


