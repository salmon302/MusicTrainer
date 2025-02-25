#include "domain/music/Voice.h"
#include <iostream>

namespace music {

Voice::Voice(const Voice& other) {
	timeSignature = other.timeSignature;
	notes = other.notes;
	measureNumbers = other.measureNumbers;
	measureNumbersValid.store(other.measureNumbersValid.load(std::memory_order_acquire), 
							std::memory_order_release);
}

Voice& Voice::operator=(const Voice& other) {
	if (this != &other) {
		timeSignature = other.timeSignature;
		notes = other.notes;
		measureNumbers = other.measureNumbers;
		measureNumbersValid.store(other.measureNumbersValid.load(std::memory_order_acquire), 
								std::memory_order_release);
	}
	return *this;
}

void Voice::performCopy(const Voice& other) {
	timeSignature = other.timeSignature;
	notes = other.notes;
	measureNumbersValid.store(false, std::memory_order_release);
}

std::unique_ptr<Voice> Voice::clone() const {
	auto clone = internalClone();
	clone->timeSignature = timeSignature;
	clone->notes = notes;
	clone->measureNumbersValid.store(false, std::memory_order_release);
	return clone;
}

size_t Voice::getMeasureForNote(size_t noteIndex) const {
	if (noteIndex >= notes.size()) {
		return 0;
	}
	
	if (!measureNumbersValid.load(std::memory_order_acquire)) {
		updateMeasureNumbers();
	}
	return measureNumbers[noteIndex];
}

size_t Voice::getHash() const {
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





