#ifndef MUSIC_TRAINER_DOMAIN_KEYSIGNATURE_H
#define MUSIC_TRAINER_DOMAIN_KEYSIGNATURE_H

#include "Pitch.h" // Depends on Pitch for tonic representation
#include <cstdint>
#include <stdexcept> // For std::invalid_argument
#include <vector>
#include <string>
#include <nlohmann/json.hpp> // Include json library

// Use alias for convenience
using json = nlohmann::json;

namespace MusicTrainer {
namespace Domain {

// Enum to represent the mode (Major or Minor)
enum class Mode : std::uint8_t {
    Major,
    Minor
};

/**
 * @brief Represents a musical key signature.
 *
 * Defines the tonic pitch and the mode (Major/Minor).
 * Can calculate the sharps or flats associated with the key.
 */
class KeySignature {
private:
    Pitch tonic;
    Mode mode;

    // Helper to calculate sharps/flats based on tonic and mode
    // This logic can be complex, providing a basic structure for now
    void calculateAccidentals(std::vector<int>& sharps, std::vector<int>& flats) const {
        // TODO: Implement logic based on circle of fifths
        // For now, clears the vectors
        sharps.clear();
        flats.clear();

        // Example placeholder: C Major / A Minor have no accidentals
        if ((tonic.getMidiNoteNumber() % 12 == 0 && mode == Mode::Major) || // C Major
            (tonic.getMidiNoteNumber() % 12 == 9 && mode == Mode::Minor)) { // A Minor
            // No accidentals
        }
        // Example placeholder: G Major / E Minor have one sharp (F#)
        else if ((tonic.getMidiNoteNumber() % 12 == 7 && mode == Mode::Major) || // G Major
                 (tonic.getMidiNoteNumber() % 12 == 4 && mode == Mode::Minor)) { // E Minor
            sharps.push_back(6); // F# (MIDI note numbers relative to C=0: F=5, F#=6)
        }
        // ... add more key signature logic ...
    }


public:
    // Default constructor (initializes to C Major)
    KeySignature() : tonic(Pitch::MiddleC()), mode(Mode::Major) {} // Default to C Major

    // Constructor with tonic pitch class and mode
    KeySignature(int tonicPitchClass, Mode m)
        : tonic(Pitch(tonicPitchClass + 60)), // Represent tonic relative to Middle C octave for simplicity
          mode(m)
    {
         if (tonicPitchClass < 0 || tonicPitchClass > 11) {
            throw std::invalid_argument("Tonic pitch class must be between 0 (C) and 11 (B).");
         }
         // Tonic pitch object validation is handled by Pitch constructor
    }

     // Constructor with specific Pitch object and mode
    KeySignature(const Pitch& t, Mode m)
        : tonic(t), mode(m) {}

    // Getters
    const Pitch& getTonic() const { return tonic; }
    Mode getMode() const { return mode; }

    // Get accidentals (example implementation)
    std::vector<int> getSharpPitchClasses() const {
        std::vector<int> sharps, flats;
        calculateAccidentals(sharps, flats);
        return sharps; // Returns pitch classes (0-11) that are sharp
    }

    std::vector<int> getFlatPitchClasses() const {
        std::vector<int> sharps, flats;
        calculateAccidentals(sharps, flats);
        return flats; // Returns pitch classes (0-11) that are flat
    }

    // Comparison operators
    bool operator==(const KeySignature& other) const {
        // Compare tonic pitch class and mode
        return (tonic.getMidiNoteNumber() % 12 == other.tonic.getMidiNoteNumber() % 12) &&
               (mode == other.mode);
    }
    bool operator!=(const KeySignature& other) const {
        return !(*this == other);
    }

    // Static common key signatures
    static KeySignature CMajor() { return KeySignature(0, Mode::Major); }
    static KeySignature AMinor() { return KeySignature(9, Mode::Minor); } // Relative minor of C Major

}; // class KeySignature

// --- JSON Serialization ---
inline void to_json(json& j, const KeySignature& ks) {
    j = json{
        // Store tonic as pitch class (0-11) for simplicity in JSON
        {"tonicClass", ks.getTonic().getMidiNoteNumber() % 12},
        {"mode", ks.getMode() == Mode::Major ? "Major" : "Minor"}
    };
}

inline void from_json(const json& j, KeySignature& ks) {
     if (!j.is_object() || !j.contains("tonicClass") || !j.contains("mode")) {
         throw json::type_error::create(302, "JSON object for KeySignature must contain tonicClass and mode", j);
    }
    try {
        int tonicClass = j.at("tonicClass").get<int>();
        std::string modeStr = j.at("mode").get<std::string>();

        Mode mode;
        if (modeStr == "Major") {
            mode = Mode::Major;
        } else if (modeStr == "Minor") {
            mode = Mode::Minor;
        } else {
            throw std::invalid_argument("Invalid mode string: " + modeStr + ". Must be 'Major' or 'Minor'.");
        }

        // Use KeySignature constructor for validation
        ks = KeySignature(tonicClass, mode);

    } catch (const json::type_error& e) {
        throw json::type_error::create(302, std::string("Type error parsing KeySignature JSON: ") + e.what(), j);
    } catch (const std::exception& e) { // Catch validation errors from constructor
         throw json::other_error::create(501, std::string("Validation error creating KeySignature from JSON: ") + e.what(), j);
    }
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_KEYSIGNATURE_H