#ifndef MUSIC_TRAINER_DOMAIN_TIMESIGNATURE_H
#define MUSIC_TRAINER_DOMAIN_TIMESIGNATURE_H

#include <cstdint>
#include <stdexcept> // For std::invalid_argument
#include <cmath>     // For std::log2, std::pow
#include <nlohmann/json.hpp> // Include json library

// Use alias for convenience
using json = nlohmann::json;

namespace MusicTrainer {
namespace Domain {

/**
 * @brief Represents a musical time signature (e.g., 4/4, 3/4).
 *
 * Stores the numerator (beats per measure) and denominator (beat unit).
 * The denominator is stored as the actual value (e.g., 4 for quarter note).
 */
class TimeSignature {
private:
    std::uint8_t numerator;   // e.g., 4 in 4/4
    std::uint8_t denominator; // e.g., 4 in 4/4 (representing quarter note)

    // Helper to check if a value is a power of 2
    static bool isPowerOfTwo(std::uint8_t n) {
        return (n > 0) && ((n & (n - 1)) == 0);
    }

public:
    // Default constructor (initializes to 4/4)
    TimeSignature() : numerator(4), denominator(4) {}

    // Constructor with validation
    TimeSignature(std::uint8_t num, std::uint8_t den)
        : numerator(num), denominator(den)
    {
        if (numerator == 0) {
            throw std::invalid_argument("Time signature numerator must be positive.");
        }
        if (!isPowerOfTwo(denominator)) {
            throw std::invalid_argument("Time signature denominator must be a power of 2 (e.g., 2, 4, 8, 16).");
        }
    }

    // Getters
    std::uint8_t getNumerator() const { return numerator; }
    std::uint8_t getDenominator() const { return denominator; }

    // Calculate beats per measure
    double getBeatsPerMeasure() const {
        // In common practice, the numerator indicates beats per measure
        // when the denominator represents the beat unit (e.g., 4 = quarter note).
        // This interpretation might need refinement based on specific use cases
        // (e.g., compound meters like 6/8 often felt in 2).
        // For now, assume numerator directly gives beats.
        return static_cast<double>(numerator);
    }

     // Calculate the duration of a whole measure in terms of beats (where denominator is the beat unit)
    double getMeasureDurationInBeats() const {
         // If 4/4, numerator=4, denominator=4. Measure has 4 quarter notes.
         // If 3/4, numerator=3, denominator=4. Measure has 3 quarter notes.
         // If 6/8, numerator=6, denominator=8. Measure has 6 eighth notes.
         // Assuming beat unit is quarter note for calculation consistency might be needed elsewhere,
         // but here we calculate based on the denominator itself.
         // A measure has 'numerator' units of '1/denominator' whole notes.
         // Let's express duration in terms of quarter notes for a common reference?
         // Or maybe just return numerator * (4.0 / denominator)?
         // Example: 4/4 -> 4 * (4/4) = 4 quarter notes
         // Example: 3/4 -> 3 * (4/4) = 3 quarter notes
         // Example: 6/8 -> 6 * (4/8) = 3 quarter notes (correct duration, matches 2 dotted quarters)
         // Example: 2/2 -> 2 * (4/2) = 4 quarter notes
         return static_cast<double>(numerator) * (4.0 / static_cast<double>(denominator));
    }


    // Comparison operators
    bool operator==(const TimeSignature& other) const {
        return numerator == other.numerator && denominator == other.denominator;
    }
    bool operator!=(const TimeSignature& other) const {
        return !(*this == other);
    }

    // Static common time signatures
    static TimeSignature CommonTime() { return TimeSignature(4, 4); } // 4/4
    static TimeSignature CutTime() { return TimeSignature(2, 2); }    // 2/2
    static TimeSignature WaltzTime() { return TimeSignature(3, 4); }  // 3/4

}; // class TimeSignature

// --- JSON Serialization ---
inline void to_json(json& j, const TimeSignature& ts) {
    j = json{
        {"numerator", ts.getNumerator()},
        {"denominator", ts.getDenominator()}
    };
}

inline void from_json(const json& j, TimeSignature& ts) {
    if (!j.is_object() || !j.contains("numerator") || !j.contains("denominator")) {
         throw json::type_error::create(302, "JSON object for TimeSignature must contain numerator and denominator", j);
    }
    try {
        // Use intermediate int for potentially wider range in JSON before casting
        int num_int = j.at("numerator").get<int>();
        int den_int = j.at("denominator").get<int>();

        if (num_int <= 0 || num_int > 255 || den_int <= 0 || den_int > 255) {
             throw std::out_of_range("Numerator/Denominator value out of range for uint8_t.");
        }

        // Use TimeSignature constructor for validation (power of 2 etc.)
        ts = TimeSignature(static_cast<uint8_t>(num_int), static_cast<uint8_t>(den_int));

    } catch (const json::type_error& e) {
        throw json::type_error::create(302, std::string("Type error parsing TimeSignature JSON: ") + e.what(), j);
    } catch (const std::exception& e) { // Catch validation errors from constructor
         throw json::other_error::create(501, std::string("Validation error creating TimeSignature from JSON: ") + e.what(), j);
    }
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_TIMESIGNATURE_H