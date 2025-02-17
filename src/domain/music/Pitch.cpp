#include "domain/music/Pitch.h"

namespace music {

Pitch Pitch::create(NoteName note, int8_t octave, int8_t accidental) {
	return Pitch(note, octave, accidental);
}

Pitch Pitch::fromMidiNote(uint8_t midiNote) {
	static const NoteName midiToNote[] = {
		NoteName::C,  // 0
		NoteName::C,  // 1
		NoteName::D,  // 2
		NoteName::D,  // 3
		NoteName::E,  // 4
		NoteName::F,  // 5
		NoteName::F,  // 6
		NoteName::G,  // 7
		NoteName::G,  // 8
		NoteName::A,  // 9
		NoteName::A,  // 10
		NoteName::B   // 11
	};
	
	int8_t octave = (midiNote / 12) - 1;
	uint8_t noteIndex = midiNote % 12;
	NoteName noteName = midiToNote[noteIndex];
	
	// Calculate accidental based on note position
	static const int8_t naturalPositions[] = {0, 2, 4, 5, 7, 9, 11}; // C, D, E, F, G, A, B positions
	int8_t accidental = 0;
	
	int naturalPosition = naturalPositions[static_cast<int>(noteName)];
	accidental = noteIndex - naturalPosition;
	
	return create(noteName, octave, accidental);
}

Pitch::Pitch(NoteName note, int8_t octave, int8_t accidental)
	: noteName(note), octave(octave), accidental(accidental) {}

uint8_t Pitch::getMidiNote() const {
	static const uint8_t noteToMidi[] = {0, 2, 4, 5, 7, 9, 11}; // C=0, D=2, E=4, F=5, G=7, A=9, B=11
	uint8_t base = noteToMidi[static_cast<int>(noteName)];
	return ((octave + 1) * 12) + base + accidental;
}

std::string Pitch::toString() const {
	static const char* noteNames[] = {"C", "D", "E", "F", "G", "A", "B"};
	std::string result = noteNames[static_cast<int>(noteName)];
	
	for (int i = 0; i < accidental; ++i) result += "#";
	for (int i = 0; i > accidental; --i) result += "b";
	
	result += std::to_string(octave);
	return result;
}

bool Pitch::operator==(const Pitch& other) const {
	return getMidiNote() == other.getMidiNote();
}

bool Pitch::operator!=(const Pitch& other) const {
	return !(*this == other);
}

} // namespace music


