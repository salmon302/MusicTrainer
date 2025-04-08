#ifndef MUSIC_TRAINER_DOMAIN_NOTE_H
#define MUSIC_TRAINER_DOMAIN_NOTE_H

#include "Pitch.h"
#include "Position.h"
#include <cstdint>
#include <stdexcept> // For std::invalid_argument, std::out_of_range
#include <cmath>     // For std::fabs
#include <limits>    // For std::numeric_limits
#include <algorithm> // Potentially needed
#include <nlohmann/json.hpp> // Include json library

// Use alias for convenience
using json = nlohmann::json;

namespace MusicTrainer {
namespace Domain {

/**
 * @brief Represents a single musical note with pitch, timing, duration, and velocity.
 *
 * Designed to be an immutable value object once created.
 */
class Note {
private:
    Pitch pitch;
    Position startTime;
    double durationBeats; // Duration in beats
    std::uint8_t velocity; // MIDI velocity (0-127)
    int voiceId; // Identifier for the voice this note belongs to

public:
    // Constructor with validation
    Note(const Pitch& p, const Position& start, double duration, int vel, int voice)
        : pitch(p), startTime(start), durationBeats(0.0), velocity(0), voiceId(voice) // Initialize members
    {
        // Validate Duration
        if (duration <= 0) {
            throw std::invalid_argument("Note duration must be positive.");
        }
        durationBeats = duration; // Assign after validation

        // Validate Velocity
        if (vel < 0 || vel > 127) {
             throw std::out_of_range("Velocity must be between 0 and 127.");
        }
        velocity = static_cast<std::uint8_t>(vel); // Assign after validation

        // Validate Voice ID
        if (voice < 0) {
             throw std::invalid_argument("Voice ID cannot be negative.");
        }
        // voiceId is already initialized in member initializer list
    }

    // Getters
    const Pitch& getPitch() const { return pitch; }
    const Position& getStartTime() const { return startTime; }
    Position getEndTime() const { return Position(startTime.beats + durationBeats); }
    double getDuration() const { return durationBeats; }
    std::uint8_t getVelocity() const { return velocity; }
    int getVoiceId() const { return voiceId; }

    // Comparison (primarily for sorting or equality checks)

    // C++17 Comparison (Example: sort by start time, then pitch)
    bool operator<(const Note& other) const {
        if (startTime != other.startTime) {
            return startTime < other.startTime;
        }
        // If start times are equal (or close enough for float comparison), compare pitch
        if (pitch != other.pitch) {
             return pitch < other.pitch;
        }
        // Could add voice ID or duration as tertiary sort keys if needed
        return voiceId < other.voiceId;
    }

    bool operator==(const Note& other) const {
        // Use tolerance for floating-point comparisons (startTime.beats and durationBeats)
        constexpr double EPSILON = 1e-9;
        return startTime == other.startTime && // Position::operator== uses tolerance
               pitch == other.pitch &&
               std::fabs(durationBeats - other.durationBeats) < EPSILON &&
               velocity == other.velocity &&
               voiceId == other.voiceId;
    }

     bool operator!=(const Note& other) const {
        return !(*this == other);
    }

    // Note: Full set of comparison operators (<, >, <=, >=) might be needed depending on usage.
    // Implementing only < and == is often sufficient if used with standard algorithms.

}; // class Note

// --- JSON Serialization ---
// Define how Note objects are converted to/from JSON
inline void to_json(json& j, const Note& n) {
    j = json{
        {"pitch", n.getPitch()},             // Uses Pitch's to_json (MIDI number)
        {"start", n.getStartTime()},         // Uses Position's to_json (beats)
        {"duration", n.getDuration()},       // Simple double
        {"velocity", n.getVelocity()},       // Simple uint8_t
        {"voiceId", n.getVoiceId()}          // Simple int
    };
}

inline void from_json(const json& j, Note& n) {
    // Check for required fields and correct types
    if (!j.is_object() || !j.contains("pitch") || !j.contains("start") || !j.contains("duration") || !j.contains("velocity") || !j.contains("voiceId")) {
        throw json::type_error::create(302, "JSON object for Note must contain pitch, start, duration, velocity, and voiceId", j);
    }
    try {
        Pitch pitch = j.at("pitch").get<Pitch>(); // Uses Pitch's from_json
        Position start = j.at("start").get<Position>(); // Uses Position's from_json
        double duration = j.at("duration").get<double>();
        int velocity = j.at("velocity").get<int>(); // Get as int for validation range
        int voiceId = j.at("voiceId").get<int>();

        // Use Note constructor for validation
        n = Note(pitch, start, duration, velocity, voiceId);

    } catch (const json::type_error& e) {
        throw json::type_error::create(302, std::string("Type error parsing Note JSON: ") + e.what(), j);
    } catch (const std::exception& e) { // Catch validation errors from Note/Pitch/Position constructors
         throw json::other_error::create(501, std::string("Validation error creating Note from JSON: ") + e.what(), j);
    }
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_NOTE_H