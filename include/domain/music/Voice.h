#ifndef MUSICTRAINERV3_VOICE_H
#define MUSICTRAINERV3_VOICE_H

#include <vector>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <cstdint>  // for uint8_t
#include "Pitch.h"
#include "Duration.h"
#include "../../utils/TrackedLock.h"

namespace music {

// Forward declaration
class Score;

class Voice {
	friend class Score;
	friend class StateSynchronizationManagerTest;
	friend class StateSynchronizationManagerTest_BasicStateSynchronization_Test;
	friend class StateSynchronizationManagerTest_ConcurrentStateUpdates_Test;
	friend class StateSynchronizationManagerTest_ConflictResolution_Test;
	friend class StateSynchronizationManagerTest_StateRecovery_Test;
	friend class StateSynchronizationManagerTest_EventPropagation_Test;
	friend class StateSynchronizationManagerTest_ComplexStateSync_Test;
	friend class LockingTest;
	friend class LockingTest_DetectsDeadlockPattern_Test;
	friend class LockingTest_TracksLockAcquisition_Test;
	friend class LockingTest_HandlesTimeouts_Test;
	friend class LockingTest_PreservesLockOrder_Test;

public:
	// Represents a note in the voice
	struct Note {
		Pitch pitch;
		Duration duration;
		
		Note(const Pitch& p, const Duration& d) : pitch(p), duration(d) {}
	};

	// Time signature representation
	struct TimeSignature {
		// Add default constructor
		TimeSignature() = default;
		
		uint8_t beats;     // Number of beats per measure
		Duration beatUnit; // Duration of one beat
		
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
		// Take shared lock for reading
		::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
		return VoiceSnapshot{notes, timeSignature};
	}
	
	explicit Voice(const TimeSignature& timeSignature = TimeSignature::commonTime()) 
		: timeSignature(timeSignature), measureNumbersValid(false) {}

	static std::unique_ptr<Voice> create(const TimeSignature& timeSignature = TimeSignature::commonTime()) {
		return std::make_unique<Voice>(timeSignature);
	}
	
	void addNote(const Pitch& pitch, const Duration& duration) {
		// Take unique lock for writing with increased timeout
		::utils::TrackedUniqueLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE,
			std::chrono::milliseconds(1000)); // Reduced timeout to fail faster
		notes.emplace_back(pitch, duration);
		measureNumbersValid = false;
	}
	
	size_t getNoteCount() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE,
			std::chrono::milliseconds(1000)); // Reduced timeout
		return notes.size();
	}
	
	const Note* getNote(size_t index) const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE,
			std::chrono::milliseconds(1000)); // Reduced timeout
		return index < notes.size() ? &notes[index] : nullptr;
	}
	
	std::vector<Note> getNotesInRange(size_t startMeasure, size_t endMeasure) const {
		std::vector<Note> notesSnapshot;
		std::vector<size_t> measureNumbersSnapshot;
		
		{
			// Take shared lock for reading with reduced timeout
			::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE, 
				std::chrono::milliseconds(1000));
			
			// Create copies while holding the lock
			notesSnapshot = notes;
			measureNumbersSnapshot = ensureMeasureNumbersUpdated();
		} // Lock is automatically released here
		
		// Process data without holding lock
		std::vector<Note> result;
		result.reserve(notesSnapshot.size());
		
		for (size_t i = 0; i < notesSnapshot.size(); ++i) {
			if (measureNumbersSnapshot[i] >= startMeasure && 
				measureNumbersSnapshot[i] <= endMeasure) {
				result.push_back(notesSnapshot[i]);
			}
		}
		
		return result;
	}

	
	size_t getMeasureForNote(size_t noteIndex) const;
	
	const TimeSignature& getTimeSignature() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE,
			std::chrono::milliseconds(5000));
		return timeSignature;
	}
	
	void setTimeSignature(const TimeSignature& newTimeSignature) {
		::utils::TrackedUniqueLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE,
			std::chrono::milliseconds(5000));
		timeSignature = newTimeSignature;
		measureNumbersValid = false;
	}
	
	size_t getHash() const;

	Voice(const Voice& other);
	Voice& operator=(const Voice& other);
	std::unique_ptr<Voice> clone() const;

protected:
	// Protected method for Score to access mutex
	std::shared_mutex& getInternalMutex() const { return mutex_; }

#ifdef TESTING
public:
	// Test-only method for mutex access
	std::shared_mutex& getMutexForTesting() const { return mutex_; }
#endif

protected:
	// Protected method for internal cloning without locks
	std::unique_ptr<Voice> internalClone() const {
		return std::make_unique<Voice>(timeSignature);
	}

private:

	std::vector<Note> notes;
	TimeSignature timeSignature;
	
	mutable std::vector<size_t> measureNumbers;
	mutable bool measureNumbersValid = false;
	mutable std::shared_mutex mutex_; // Changed from recursive_mutex to shared_mutex

	// Helper method to safely update and return measure numbers
	std::vector<size_t> ensureMeasureNumbersUpdated() const {
		// Caller must hold mutex_
		if (!measureNumbersValid) {
			updateMeasureNumbers();
		}
		return measureNumbers;
	}

	void updateMeasureNumbers() const {
		// Note: Caller must hold mutex_
		const Duration measureDuration = timeSignature.beatUnit * timeSignature.beats;
		measureNumbers.clear();
		measureNumbers.reserve(notes.size());
		
		size_t currentMeasure = 0;
		Duration currentBeat = Duration::createEmpty();
		
		for (const auto& note : notes) {
			measureNumbers.push_back(currentMeasure);
			currentBeat = currentBeat + note.duration;
			
			while (currentBeat >= measureDuration) {
				currentBeat = currentBeat - measureDuration;
				currentMeasure++;
			}
		}
		measureNumbersValid = true;
	}
	
	void performCopy(const Voice& other);  // Add this line
};

} // namespace music

#endif // MUSICTRAINERV3_VOICE_H
