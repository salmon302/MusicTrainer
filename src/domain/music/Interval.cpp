#include "domain/music/Interval.h"
#include <iostream>

namespace music {

Interval Interval::create(Number number, Quality quality) {
	return Interval(number, quality);
}

Interval Interval::fromPitches(const Pitch& lower, const Pitch& upper) {
	std::cout << "[Interval::fromPitches] Lower pitch: " << lower.toString() 
			  << " (MIDI: " << lower.getMidiNote() << "), Upper pitch: " 
			  << upper.toString() << " (MIDI: " << upper.getMidiNote() << ")\n";
	
	// Calculate absolute semitone difference
	int semitones = std::abs(upper.getMidiNote() - lower.getMidiNote());
	std::cout << "[Interval::fromPitches] Semitone difference: " << semitones << "\n";
	
	// Check for octave directly
	if (semitones == 12 || (semitones % 12 == 0 && semitones > 0)) {
		std::cout << "[Interval::fromPitches] Detected octave interval\n";
		return create(IntervalNumber::OCTAVE, IntervalQuality::PERFECT);
	}
	
	// Calculate diatonic steps between notes
	int lowerNote = static_cast<int>(lower.getNoteName());
	int upperNote = static_cast<int>(upper.getNoteName());
	int steps = upperNote - lowerNote;
	if (steps < 0) steps += 7;  // Normalize negative steps
	
	std::cout << "[Interval::fromPitches] Diatonic steps before octave adjustment: " << steps << "\n";
	
	// Add octave steps
	int octaveDiff = upper.getOctave() - lower.getOctave();
	if (upperNote < lowerNote) octaveDiff--;  // Adjust for wrapping around
	steps += octaveDiff * 7;
	
	std::cout << "[Interval::fromPitches] Octave difference: " << octaveDiff << "\n";
	std::cout << "[Interval::fromPitches] Total diatonic steps: " << steps << "\n";
	
	// Convert steps to interval number (1-based)
	IntervalNumber number = static_cast<IntervalNumber>((steps % 7) + 1);
	
	// Perfect/Major reference intervals in semitones
	static const int perfectMajorSemitones[] = {0, 2, 4, 5, 7, 9, 11, 12};  // Include octave
	int expectedSemitones = perfectMajorSemitones[steps % 7];
	expectedSemitones += (octaveDiff * 12);  // Add octave semitones
	
	std::cout << "[Interval::fromPitches] Expected semitones: " << expectedSemitones << "\n";
	
	// Determine quality based on difference from perfect/major
	IntervalQuality quality;
	int diff = semitones - expectedSemitones;
	
	std::cout << "[Interval::fromPitches] Difference from expected: " << diff << "\n";
	std::cout << "[Interval::fromPitches] Is perfect number: " << (isPerfectNumber(number) ? "yes" : "no") << "\n";
	
	if (isPerfectNumber(number)) {
		switch (diff) {
			case 0: quality = IntervalQuality::PERFECT; break;
			case -1: quality = IntervalQuality::DIMINISHED; break;
			case 1: quality = IntervalQuality::AUGMENTED; break;
			default: quality = IntervalQuality::DIMINISHED; break;
		}
	} else {
		switch (diff) {
			case 0: quality = IntervalQuality::MAJOR; break;
			case -1: quality = IntervalQuality::MINOR; break;
			case -2: quality = IntervalQuality::DIMINISHED; break;
			case 1: quality = IntervalQuality::AUGMENTED; break;
			default: quality = IntervalQuality::DIMINISHED; break;
		}
	}
	
	auto result = create(number, quality);
	std::cout << "[Interval::fromPitches] Calculated interval: " << result.toString() 
			  << " (number=" << static_cast<int>(number) << ", quality=" << static_cast<int>(quality) << ")\n";
	return result;
}

bool Interval::isPerfectNumber(IntervalNumber num) {
	return num == IntervalNumber::UNISON || num == IntervalNumber::FOURTH || 
		   num == IntervalNumber::FIFTH || num == IntervalNumber::OCTAVE;
}

Interval::Interval(IntervalNumber number, IntervalQuality quality) 
	: number(number), quality(quality) {}

int8_t Interval::getSemitones() const {
	static const int8_t perfectIntervals[] = {0, 2, 4, 5, 7, 9, 11, 12};
	int8_t base = perfectIntervals[static_cast<int>(number) - 1];
	
	switch (quality) {
		case IntervalQuality::DIMINISHED: return base - 1;
		case IntervalQuality::MINOR: return base - 1;
		case IntervalQuality::PERFECT: return base;
		case IntervalQuality::MAJOR: return base;
		case IntervalQuality::AUGMENTED: return base + 1;
		default: return base;
	}
}

bool Interval::isPerfect() const {
	switch (number) {
		case IntervalNumber::UNISON:
		case IntervalNumber::FOURTH:
		case IntervalNumber::FIFTH:
		case IntervalNumber::OCTAVE:
			return true;
		default:
			return false;
	}
}

std::string Interval::toString() const {
	std::string result;
	switch (quality) {
		case IntervalQuality::DIMINISHED: result = "d"; break;
		case IntervalQuality::MINOR: result = "m"; break;
		case IntervalQuality::PERFECT: result = "P"; break;
		case IntervalQuality::MAJOR: result = "M"; break;
		case IntervalQuality::AUGMENTED: result = "A"; break;
	}
	result += std::to_string(static_cast<int>(number));
	return result;
}

bool Interval::operator==(const Interval& other) const {
	return number == other.number && quality == other.quality;
}

bool Interval::operator!=(const Interval& other) const {
	return !(*this == other);
}

bool Interval::operator<(const Interval& other) const {
	return getSemitones() < other.getSemitones();
}

} // namespace music
