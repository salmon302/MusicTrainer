#ifndef MUSICTRAINERV3_DURATION_H
#define MUSICTRAINERV3_DURATION_H

#include <cstdint>
#include <string>

namespace MusicTrainer {
namespace music {

class Duration {
public:
	// Default constructor
	Duration() = default;

	enum class Type {
		WHOLE = 1,
		HALF = 2,
		QUARTER = 4,
		EIGHTH = 8,
		SIXTEENTH = 16,
		THIRTY_SECOND = 32
	};

	// Factory method for creating a duration
	static Duration create(Type baseType, uint8_t dots = 0);
	
	// Static factory methods for common durations
	static Duration createWhole() { return create(Type::WHOLE); }
	static Duration createHalf() { return create(Type::HALF); }
	static Duration createQuarter() { return create(Type::QUARTER); }
	static Duration createEighth() { return create(Type::EIGHTH); }
	static Duration createEmpty() { return create(Type::QUARTER, 0); }
	
	// Immutable accessors
	Type getBaseType() const { return baseType; }
	uint8_t getDots() const { return dots; }
	double getTotalBeats() const;
	std::string toString() const;

	// Type checking methods
	bool isWhole() const { return baseType == Type::WHOLE; }
	bool isHalf() const { return baseType == Type::HALF; }
	bool isQuarter() const { return baseType == Type::QUARTER; }
	bool isEighth() const { return baseType == Type::EIGHTH; }
	bool isSixteenth() const { return baseType == Type::SIXTEENTH; }
	bool isThirtySecond() const { return baseType == Type::THIRTY_SECOND; }

	// Value semantics
	bool operator==(const Duration& other) const;
	bool operator!=(const Duration& other) const;
	bool operator<(const Duration& other) const;
	bool operator>=(const Duration& other) const;
	bool operator>(const Duration& other) const;

	// Arithmetic operators
	Duration operator+(const Duration& other) const;
	Duration operator-(const Duration& other) const;
	Duration operator*(uint8_t multiplier) const;
	
	// Add operator+=
	Duration& operator+=(const Duration& other) {
		*this = *this + other;
		return *this;
	}

private:
	Duration(Type baseType, uint8_t dots);
	
	Type baseType;
	uint8_t dots;  // Number of dots extending the duration
};

} // namespace music
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_DURATION_H