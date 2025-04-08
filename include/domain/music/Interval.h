#pragma once

#include "domain/music/Pitch.h"
#include <string>
#include <cstdint>

namespace MusicTrainer {
namespace music {

class Interval {
public:
    enum class Quality {
        Perfect,
        Major,
        Minor,
        Augmented,
        Diminished
    };

    enum class Number {
        Unison = 1,
        Second = 2,
        Third = 3,
        Fourth = 4,
        Fifth = 5,
        Sixth = 6,
        Seventh = 7,
        Octave = 8
    };

    // Factory methods
    static Interval fromSemitones(int semitones);
    static Interval fromQualityAndNumber(Quality quality, Number number);
    static Interval fromPitches(const Pitch& lower, const Pitch& higher);

    // Getters
    Quality getQuality() const { return m_quality; }
    Number getNumber() const { return m_number; }
    int getSemitones() const { return m_semitones; }
    std::string toString() const;

    // Operators
    bool operator==(const Interval& other) const;
    bool operator!=(const Interval& other) const;
    bool operator<(const Interval& other) const;

    // Utility functions for interval validation
    static bool isDissonant(int semitones);
    static bool isConsonant(int semitones);
    static bool isStepwise(int semitones);

    // Helper method to get common interval names
    static std::string getIntervalName(int semitones);
    
    // Get interval between two pitches
    static int getIntervalInSemitones(const Pitch& p1, const Pitch& p2);

    // Helper method for checking valid melodic intervals
    static bool isValidMelodicInterval(int semitones);
    
    // Check if an interval is diminished
    static bool isDiminishedInterval(int semitones);

private:
    Interval(Quality quality, Number number, int semitones);
    
    Quality m_quality;
    Number m_number;
    int m_semitones;
};

} // namespace music
} // namespace MusicTrainer