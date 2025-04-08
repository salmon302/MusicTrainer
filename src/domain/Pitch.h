#ifndef MUSIC_TRAINER_DOMAIN_PITCH_H
#define MUSIC_TRAINER_DOMAIN_PITCH_H

#include <cstdint>
// #include <compare> // Requires C++20
#include <stdexcept> // For std::out_of_range
#include <string>
#include <functional> // For std::hash (optional)
#include <nlohmann/json.hpp> // Include json library

// Use alias for convenience
using json = nlohmann::json;

namespace MusicTrainer {
namespace Domain {

/**
 * @brief Represents a musical pitch using the MIDI note number standard.
 *
 * MIDI note numbers range from 0 to 127, where 60 is Middle C (C4).
 * Provides validation and basic operations.
 */
class Pitch {
private:
    std::uint8_t midiNoteNumber; // Range 0-127

public:
    // Default constructor (initializes to Middle C)
    Pitch() : midiNoteNumber(60) {}

    // Constructor with validation
    explicit Pitch(int number) {
        if (number < 0 || number > 127) {
            throw std::out_of_range("MIDI note number must be between 0 and 127.");
        }
        midiNoteNumber = static_cast<std::uint8_t>(number);
    }

    // Getter for the MIDI note number
    std::uint8_t getMidiNoteNumber() const {
        return midiNoteNumber;
    }

    // Comparison operators (C++17 compatible)
    bool operator==(const Pitch& other) const {
        return midiNoteNumber == other.midiNoteNumber;
    }
    bool operator!=(const Pitch& other) const {
        return midiNoteNumber != other.midiNoteNumber;
    }
    bool operator<(const Pitch& other) const {
        return midiNoteNumber < other.midiNoteNumber;
    }
    bool operator>(const Pitch& other) const {
        return midiNoteNumber > other.midiNoteNumber;
    }
    bool operator<=(const Pitch& other) const {
        return midiNoteNumber <= other.midiNoteNumber;
    }
    bool operator>=(const Pitch& other) const {
        return midiNoteNumber >= other.midiNoteNumber;
    }

    // Basic transposition
    Pitch transpose(int semitones) const {
        int newNumber = static_cast<int>(midiNoteNumber) + semitones;
        // Constructor handles validation
        return Pitch(newNumber);
    }

    // Static helper for Middle C
    static Pitch MiddleC() {
        return Pitch(60);
    }

    // Simple debug representation
    std::string debugName() const {
        return "MIDI(" + std::to_string(midiNoteNumber) + ")";
    }

    // TODO: Add conversion to/from note names (e.g., "C#4") if needed later.
}; // Ensure class definition is closed

// --- JSON Serialization ---
// Allow Pitch to be serialized/deserialized by nlohmann::json
inline void to_json(json& j, const Pitch& p) {
    j = p.getMidiNoteNumber(); // Serialize Pitch simply as its MIDI number
}

inline void from_json(const json& j, Pitch& p) {
    if (!j.is_number_integer()) {
         throw json::type_error::create(302, "JSON value for Pitch must be an integer (MIDI number)", j);
    }
    int midiNumber = j.get<int>();
    // Use Pitch constructor for validation
    try {
        p = Pitch(midiNumber);
    } catch (const std::out_of_range& e) {
        // Convert constructor exception to json exception for consistent error handling
        throw json::other_error::create(501, "Invalid MIDI number in JSON: " + std::to_string(midiNumber) + " (" + e.what() + ")", j);
    }
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_PITCH_H