#include "domain/music/Duration.h"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace music {

Duration Duration::create(Type baseType, uint8_t dots) {
	return Duration(baseType, dots);
}

Duration::Duration(Type baseType, uint8_t dots) : baseType(baseType), dots(dots) {}

double Duration::getTotalBeats() const {
	double beats = 4.0 / static_cast<double>(baseType);
	double multiplier = 1.0;
	for (int i = 0; i < dots; ++i) {
		multiplier += multiplier / 2.0;
	}
	return beats * multiplier;
}

std::string Duration::toString() const {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << getTotalBeats();
	return ss.str();
}

bool Duration::operator==(const Duration& other) const {
	return getTotalBeats() == other.getTotalBeats();
}

bool Duration::operator!=(const Duration& other) const {
	return !(*this == other);
}

bool Duration::operator<(const Duration& other) const {
	return getTotalBeats() < other.getTotalBeats();
}

bool Duration::operator>=(const Duration& other) const {
	return !(*this < other);
}

bool Duration::operator>(const Duration& other) const {
	return other < *this;
}

Duration Duration::operator+(const Duration& other) const {
	double totalBeats = getTotalBeats() + other.getTotalBeats();
	
	// Find the closest base duration type
	Type newType = Type::QUARTER;
	uint8_t dots = 0;
	
	// Try different base types to find the best match
	for (Type type : {Type::WHOLE, Type::HALF, Type::QUARTER, Type::EIGHTH, Type::SIXTEENTH, Type::THIRTY_SECOND}) {
		double baseBeats = 4.0 / static_cast<double>(type);
		if (baseBeats <= totalBeats) {
			newType = type;
			break;
		}
	}
	
	// Calculate dots needed
	double remaining = totalBeats;
	double baseValue = 4.0 / static_cast<double>(newType);
	while (remaining > baseValue * 1.5 && dots < 3) { // Limit to triple-dotted
		baseValue *= 1.5;
		dots++;
	}
	
	return create(newType, dots);
}

Duration Duration::operator-(const Duration& other) const {
	double totalBeats = getTotalBeats() - other.getTotalBeats();
	if (totalBeats <= 0) {
		return create(Type::QUARTER, 0); // Return minimal duration if result would be negative
	}
	
	// Use same logic as addition for finding closest duration
	Type newType = Type::QUARTER;
	uint8_t dots = 0;
	
	for (Type type : {Type::WHOLE, Type::HALF, Type::QUARTER, Type::EIGHTH, Type::SIXTEENTH, Type::THIRTY_SECOND}) {
		double baseBeats = 4.0 / static_cast<double>(type);
		if (baseBeats <= totalBeats) {
			newType = type;
			break;
		}
	}
	
	double remaining = totalBeats;
	double baseValue = 4.0 / static_cast<double>(newType);
	while (remaining > baseValue * 1.5 && dots < 3) {
		baseValue *= 1.5;
		dots++;
	}
	
	return create(newType, dots);
}

Duration Duration::operator*(uint8_t multiplier) const {
    double totalBeats = getTotalBeats() * multiplier;
    
    // Find the closest base duration type
    Type newType = Type::QUARTER;
    uint8_t dots = 0;
    
    for (Type type : {Type::WHOLE, Type::HALF, Type::QUARTER, Type::EIGHTH, Type::SIXTEENTH, Type::THIRTY_SECOND}) {
        double baseBeats = 4.0 / static_cast<double>(type);
        if (baseBeats <= totalBeats) {
            newType = type;
            break;
        }
    }
    
    // Calculate dots needed
    double remaining = totalBeats;
    double baseValue = 4.0 / static_cast<double>(newType);
    while (remaining > baseValue * 1.5 && dots < 3) {
        baseValue *= 1.5;
        dots++;
    }
    
    return create(newType, dots);
}

} // namespace music
