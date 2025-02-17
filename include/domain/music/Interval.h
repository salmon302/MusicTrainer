#ifndef MUSICTRAINERV3_INTERVAL_H
#define MUSICTRAINERV3_INTERVAL_H

#include <cstdint>
#include <string>
#include <ostream>
#include "Pitch.h"

namespace music {

class Interval;  // Forward declaration

// Forward declare the enum classes
enum class IntervalQuality {
	DIMINISHED,
	MINOR,
	PERFECT,
	MAJOR,
	AUGMENTED
};

enum class IntervalNumber {
	UNISON = 1,
	SECOND = 2,
	THIRD = 3,
	FOURTH = 4,
	FIFTH = 5,
	SIXTH = 6,
	SEVENTH = 7,
	OCTAVE = 8
};

inline std::ostream& operator<<(std::ostream& os, const IntervalNumber& num) {
	return os << static_cast<int>(num);
}

inline std::ostream& operator<<(std::ostream& os, const IntervalQuality& quality) {
	switch(quality) {
		case IntervalQuality::DIMINISHED: return os << "DIMINISHED";
		case IntervalQuality::MINOR: return os << "MINOR";
		case IntervalQuality::PERFECT: return os << "PERFECT";
		case IntervalQuality::MAJOR: return os << "MAJOR";
		case IntervalQuality::AUGMENTED: return os << "AUGMENTED";
		default: return os << "UNKNOWN";
	}
}

class Interval {

public:
	using Quality = IntervalQuality;
	using Number = IntervalNumber;


	// Factory methods for creating intervals
	static Interval create(Number number, Quality quality);
	static Interval fromPitches(const Pitch& lower, const Pitch& upper);
	
	// Immutable accessors
	Number getNumber() const { return number; }
	Quality getQuality() const { return quality; }
	int8_t getSemitones() const;
	bool isPerfect() const;
	std::string toString() const;

	// Value semantics
	bool operator==(const Interval& other) const;
	bool operator!=(const Interval& other) const;
	bool operator<(const Interval& other) const;

private:
	Interval(Number number, Quality quality);
	static bool isPerfectNumber(Number num);
	
	Number number;
	Quality quality;
};

} // namespace music


#endif // MUSICTRAINERV3_INTERVAL_H