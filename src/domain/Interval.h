#ifndef MUSIC_TRAINER_DOMAIN_INTERVAL_H
#define MUSIC_TRAINER_DOMAIN_INTERVAL_H

#include "Pitch.h"
#include <cmath> // For std::abs
#include <optional> // To represent quality potentially

namespace MusicTrainer {
namespace Domain {

/**
 * @brief Represents the interval between two pitches.
 *
 * Can calculate the size in semitones and potentially quality (Major, Minor, Perfect, etc.).
 */
class Interval {
public:
    // Enum for interval quality (simplified)
    enum class Quality {
        Perfect,
        Major,
        Minor,
        Augmented,
        Diminished
        // Add DoublyAugmented, DoublyDiminished if needed
    };

private:
    int semitones; // Size of the interval in semitones (+ve upwards, -ve downwards)
    // Optional: Store quality and number (e.g., Perfect 5th, Major 3rd) if needed
    // Quality quality;
    // int number; // e.g., 1 (unison), 2 (second), 3 (third), etc.

public:
    // Constructor from two pitches
    Interval(const Pitch& p1, const Pitch& p2)
        : semitones(static_cast<int>(p2.getMidiNoteNumber()) - static_cast<int>(p1.getMidiNoteNumber()))
    {
        // TODO: Implement logic to determine quality and number if required
    }

    // Constructor directly from semitones
    explicit Interval(int st) : semitones(st) {}

    // Getters
    int getSemitones() const {
        return semitones;
    }

    // Get absolute size in semitones (distance)
    int getAbsoluteSemitones() const {
        return std::abs(semitones);
    }

    // TODO: Add methods like getQuality(), getNumber() if implemented

    // Check for specific intervals (common for rules)
    bool isPerfectUnison() const { return semitones == 0; }
    bool isMinorSecond() const { return getAbsoluteSemitones() == 1; }
    bool isMajorSecond() const { return getAbsoluteSemitones() == 2; }
    bool isMinorThird() const { return getAbsoluteSemitones() == 3; }
    bool isMajorThird() const { return getAbsoluteSemitones() == 4; }
    bool isPerfectFourth() const { return getAbsoluteSemitones() == 5; }
    bool isTritone() const { return getAbsoluteSemitones() == 6; } // Augmented 4th / Diminished 5th
    bool isPerfectFifth() const { return getAbsoluteSemitones() == 7; }
    bool isMinorSixth() const { return getAbsoluteSemitones() == 8; }
    bool isMajorSixth() const { return getAbsoluteSemitones() == 9; }
    bool isMinorSeventh() const { return getAbsoluteSemitones() == 10; }
    bool isMajorSeventh() const { return getAbsoluteSemitones() == 11; }
    bool isPerfectOctave() const { return getAbsoluteSemitones() == 12; }

    // Check if interval is compound (greater than an octave)
    bool isCompound() const { return getAbsoluteSemitones() > 12; }

    // Get simple interval (within one octave)
    Interval getSimpleInterval() const {
        int simpleSemitones = getAbsoluteSemitones() % 12;
        // Need to handle unison/octave carefully if sign matters
        if (semitones == 0) return Interval(0);
        if (simpleSemitones == 0 && getAbsoluteSemitones() > 0) simpleSemitones = 12; // Map octaves to 12
        // Preserve original direction? For now, just return positive simple interval
        return Interval(simpleSemitones);
    }


    // Comparison operators
    bool operator==(const Interval& other) const {
        return semitones == other.semitones;
    }
     bool operator!=(const Interval& other) const {
        return semitones != other.semitones;
    }
     bool operator<(const Interval& other) const {
        return semitones < other.semitones;
    }
     // Add >, <=, >= if needed

}; // class Interval

// Helper function (optional alternative to constructor)
inline Interval calculateInterval(const Pitch& p1, const Pitch& p2) {
    return Interval(p1, p2);
}


} // namespace Domain
} // namespace MusicTrainer

#endif // MUSIC_TRAINER_DOMAIN_INTERVAL_H