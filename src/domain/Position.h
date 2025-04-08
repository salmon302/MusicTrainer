#ifndef MUSIC_TRAINER_DOMAIN_POSITION_H
#define MUSIC_TRAINER_DOMAIN_POSITION_H

#include <cstdint>
#include <cmath> // For std::fabs
// #include <limits> // Epsilon handled manually now
#include <nlohmann/json.hpp> // Include json library

// Use alias for convenience
using json = nlohmann::json;

namespace MusicTrainer {
namespace Domain {

/**
 * @brief Represents a precise position in musical time.
 *
 * Uses beats as the primary unit, allowing for fractional beats to represent
 * finer subdivisions. Assumes a context (like a Score) provides the mapping
 * from beats to measures based on time signatures.
 */
struct Position {
    double beats = 0.0; // Position in beats from the beginning of the score

    // Default constructor
    Position() = default;

    // Explicit constructor
    explicit Position(double b) : beats(b) {}

    // Comparison operators (C++17 compatible)
    // Use tolerance for floating-point comparisons
    bool operator==(const Position& other) const {
        constexpr double EPSILON = 1e-9; // Define a small tolerance
        return std::fabs(beats - other.beats) < EPSILON;
    }
    bool operator!=(const Position& other) const {
        // Reuse operator== for consistency
        return !(*this == other);
    }
    bool operator<(const Position& other) const {
        // Check for strict inequality considering tolerance
        constexpr double EPSILON = 1e-9;
        return (other.beats - beats) > EPSILON;
    }
    bool operator>(const Position& other) const {
        // Reuse operator<
        return other < *this;
    }
    bool operator<=(const Position& other) const {
        // Reuse operator>
        return !(*this > other);
    }
    bool operator>=(const Position& other) const {
        // Reuse operator<
        return !(*this < other);
    }

    // Basic arithmetic
    Position operator+(double beat_offset) const {
        return Position(beats + beat_offset);
    }
    Position& operator+=(double beat_offset) {
        beats += beat_offset;
        return *this;
    }
    Position operator-(double beat_offset) const {
        return Position(beats - beat_offset);
    }
    Position& operator-=(double beat_offset) {
        beats -= beat_offset;
        return *this;
    }
    // Binary subtraction between two Positions
    double operator-(const Position& other) const {
        return beats - other.beats;
    }
};

// --- JSON Serialization ---
// Allow Position to be serialized/deserialized by nlohmann::json
inline void to_json(json& j, const Position& p) {
    j = p.beats; // Serialize Position simply as its beat number
}

inline void from_json(const json& j, Position& p) {
    if (!j.is_number()) {
        throw json::type_error::create(302, "JSON value for Position must be a number (beats)", j);
    }
    p.beats = j.get<double>();
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_POSITION_H