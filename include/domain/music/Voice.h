#ifndef MUSICTRAINERV3_VOICE_H
#define MUSICTRAINERV3_VOICE_H

#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstdint>
#include <mutex>
#include "Pitch.h"
#include "Duration.h"

namespace music {

class Score;

namespace rules {
class ParallelFifthsRule;
class ParallelOctavesRule;
class VoiceLeadingRule;
}

class Voice {
	friend class Score;
	friend class rules::ParallelFifthsRule;
	friend class rules::ParallelOctavesRule;
	friend class rules::VoiceLeadingRule;

public:
	struct Note {
		Pitch pitch;
		Duration duration;
		Note(const Pitch& p, const Duration& d) : pitch(p), duration(d) {}
	};

	struct TimeSignature {
		TimeSignature() = default;
		uint8_t beats;
		Duration beatUnit;
		
		TimeSignature(uint8_t b, const Duration& unit) 
			: beats(b), beatUnit(unit) {}
			
		static TimeSignature commonTime() {
			return TimeSignature(4, Duration::createQuarter());
		}
	};

	struct VoiceSnapshot {
		std::vector<Note> notes;
		TimeSignature timeSignature;
	};
	
	VoiceSnapshot createSnapshot() const {
		return VoiceSnapshot{notes, timeSignature};
	}
	
	explicit Voice(const TimeSignature& timeSignature = TimeSignature::commonTime()) 
		: timeSignature(timeSignature) {}

	static std::unique_ptr<Voice> create(const TimeSignature& timeSignature = TimeSignature::commonTime()) {
		return std::make_unique<Voice>(timeSignature);
	}
	
	void addNote(const Pitch& pitch, const Duration& duration) {
		auto newNotes = notes;
		newNotes.emplace_back(pitch, duration);
		notes = std::move(newNotes);
		measureNumbersValid.store(false, std::memory_order_release);
	}
	
	size_t getNoteCount() const {
		return notes.size();
	}
	
	const Note* getNote(size_t index) const {
		return index < notes.size() ? &notes[index] : nullptr;
	}
	
	std::vector<Note> getNotesInRange(size_t startMeasure, size_t endMeasure) const {
		if (!measureNumbersValid.load(std::memory_order_acquire)) {
			updateMeasureNumbers();
		}
		
		std::vector<Note> result;
		result.reserve(notes.size());
		
		for (size_t i = 0; i < notes.size(); ++i) {
			if (measureNumbers[i] >= startMeasure && 
				measureNumbers[i] <= endMeasure) {
				result.push_back(notes[i]);
			}
		}
		
		return result;
	}

	size_t getMeasureForNote(size_t noteIndex) const;
	
	const TimeSignature& getTimeSignature() const {
		return timeSignature;
	}
	
	void setTimeSignature(const TimeSignature& newTimeSignature) {
		timeSignature = newTimeSignature;
		measureNumbersValid.store(false, std::memory_order_release);
	}
	
	size_t getHash() const;

	Voice(const Voice& other);
	Voice& operator=(const Voice& other);
	std::unique_ptr<Voice> clone() const;

	#ifdef TESTING
	std::mutex& getMutexForTesting() { return mutex_; }
	#endif

protected:
	std::unique_ptr<Voice> internalClone() const {
		return std::make_unique<Voice>(timeSignature);
	}

private:
	std::vector<Note> notes;
	TimeSignature timeSignature;
	mutable std::vector<size_t> measureNumbers;
	mutable std::atomic<bool> measureNumbersValid{false};
	mutable std::mutex mutex_;

	void updateMeasureNumbers() const {
		const Duration measureDuration = timeSignature.beatUnit * timeSignature.beats;
		auto newMeasureNumbers = std::vector<size_t>();
		newMeasureNumbers.reserve(notes.size());
		
		size_t currentMeasure = 0;
		Duration currentBeat = Duration::createEmpty();
		
		for (const auto& note : notes) {
			newMeasureNumbers.push_back(currentMeasure);
			currentBeat = currentBeat + note.duration;
			
			while (currentBeat >= measureDuration) {
				currentBeat = currentBeat - measureDuration;
				currentMeasure++;
			}
		}
		
		measureNumbers = std::move(newMeasureNumbers);
		measureNumbersValid.store(true, std::memory_order_release);
	}
	
	void performCopy(const Voice& other);
};

} // namespace music

#endif // MUSICTRAINERV3_VOICE_H

