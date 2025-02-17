#include "domain/music/Voice.h"
#include "utils/DebugUtils.h"
#include <iostream>

namespace music {

Voice::Voice(const Voice& other) {
	// Get all data under a single shared lock
	TimeSignature ts;
	std::vector<Note> notesCopy;
	std::vector<size_t> measureNumbersCopy;
	bool measureNumbersValidCopy;
	
	{
		::utils::TrackedSharedMutexLock lock(other.mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);

		ts = other.timeSignature;
		notesCopy = other.notes;
		measureNumbersCopy = other.measureNumbers;
		measureNumbersValidCopy = other.measureNumbersValid;
	}
	
	// Initialize member variables without holding locks
	timeSignature = std::move(ts);
	notes = std::move(notesCopy);
	measureNumbers = std::move(measureNumbersCopy);
	measureNumbersValid = measureNumbersValidCopy;
}

Voice& Voice::operator=(const Voice& other) {
	if (this != &other) {
		// Get all data under other's shared lock
		TimeSignature ts;
		std::vector<Note> notesCopy;
		std::vector<size_t> measureNumbersCopy;
		bool measureNumbersValidCopy;
		
		{
			::utils::TrackedSharedMutexLock lock(other.mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
			ts = other.timeSignature;
			notesCopy = other.notes;
			measureNumbersCopy = other.measureNumbers;
			measureNumbersValidCopy = other.measureNumbersValid;
		}
		
		// Update our data under our unique lock
		{
			::utils::TrackedUniqueLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
			timeSignature = std::move(ts);
			notes = std::move(notesCopy);
			measureNumbers = std::move(measureNumbersCopy);
			measureNumbersValid = measureNumbersValidCopy;
		}
	}
	return *this;
}

void Voice::performCopy(const Voice& other) {
	// Get data under source shared lock
	TimeSignature ts;
	std::vector<Note> notesCopy;
	{
		::utils::TrackedSharedMutexLock lock(other.mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
		ts = other.timeSignature;
		notesCopy = other.notes;
	}
	
	// Set our data under unique lock
	{
		::utils::TrackedUniqueLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
		timeSignature = std::move(ts);
		notes = std::move(notesCopy);
		measureNumbersValid = false;
	}
}

std::unique_ptr<Voice> Voice::clone() const {
	// Create new voice and get data under single lock
	auto clone = internalClone();
	TimeSignature ts;
	std::vector<Note> notesCopy;
	
	{
		::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
		ts = timeSignature;
		notesCopy = notes;
	}
	
	// Set data without holding lock
	clone->timeSignature = std::move(ts);
	clone->notes = std::move(notesCopy);
	clone->measureNumbersValid = false;
	
	return clone;
}



size_t Voice::getMeasureForNote(size_t noteIndex) const {
	// Always take unique lock to avoid upgrade deadlocks
	::utils::TrackedUniqueLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
	
	if (noteIndex >= notes.size()) {
		return 0;
	}
	
	if (!measureNumbersValid) {
		updateMeasureNumbers();
	}
	return measureNumbers[noteIndex];
}


size_t Voice::getHash() const {

	::utils::TrackedSharedMutexLock lock(mutex_, "Voice::mutex_", ::utils::LockLevel::VOICE);
	
	size_t hash = std::hash<uint8_t>{}(timeSignature.beats);
	size_t beatHash = std::hash<double>{}(timeSignature.beatUnit.getTotalBeats());
	hash ^= beatHash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	
	for (const auto& note : notes) {
		size_t noteHash = note.pitch.getMidiNote();
		noteHash ^= std::hash<double>{}(note.duration.getTotalBeats());
		hash ^= noteHash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
	return hash;
}


} // namespace music




