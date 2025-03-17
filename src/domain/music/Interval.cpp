#include "domain/music/Interval.h"
#include <iostream>
#include <cmath>
#include <stdexcept>

namespace MusicTrainer {
namespace music {

Interval::Interval(Quality quality, Number number, int semitones)
    : m_quality(quality), m_number(number), m_semitones(semitones) {}

Interval Interval::fromPitches(const MusicTrainer::music::Pitch& lower, const MusicTrainer::music::Pitch& higher) {
    std::cout << "[fromPitches] Lower pitch: " << lower.toString() 
              << " (MIDI: " << lower.getMidiNote() << "), Upper pitch: "
              << higher.toString() << " (MIDI: " << higher.getMidiNote() << ")\n";

    // Calculate semitones between pitches
    int semitones = std::abs(higher.getMidiNote() - lower.getMidiNote());
    
    // Convert semitones to interval
    Interval result = fromSemitones(semitones);

    // Determine if interval direction matches pitch direction
    int lowerNote = static_cast<int>(lower.getNoteName());
    int upperNote = static_cast<int>(higher.getNoteName());
    
    // Normalize interval based on direction
    if (higher.getMidiNote() >= lower.getMidiNote()) {
        // Ascending interval
        if (upperNote < lowerNote) {
            // Crossed octave boundary
            int octaveDiff = higher.getOctave() - lower.getOctave();
            if (octaveDiff > 1) {
                // Compound interval
                return fromQualityAndNumber(result.getQuality(), 
                    static_cast<Number>(static_cast<int>(result.getNumber()) + 7));
            }
        }
    }
    return result;
}

Interval Interval::fromSemitones(int semitones) {
    // Map semitones to quality and number
    Quality quality;
    Number number;
    
    switch (semitones) {
        case 0:  // Perfect unison
            quality = Quality::Perfect;
            number = Number::Unison;
            break;
        case 1:  // Minor second
            quality = Quality::Minor;
            number = Number::Second;
            break;
        case 2:  // Major second
            quality = Quality::Major;
            number = Number::Second;
            break;
        case 3:  // Minor third
            quality = Quality::Minor;
            number = Number::Third;
            break;
        case 4:  // Major third
            quality = Quality::Major;
            number = Number::Third;
            break;
        case 5:  // Perfect fourth
            quality = Quality::Perfect;
            number = Number::Fourth;
            break;
        case 7:  // Perfect fifth
            quality = Quality::Perfect;
            number = Number::Fifth;
            break;
        case 8:  // Minor sixth
            quality = Quality::Minor;
            number = Number::Sixth;
            break;
        case 9:  // Major sixth
            quality = Quality::Major;
            number = Number::Sixth;
            break;
        case 10: // Minor seventh
            quality = Quality::Minor;
            number = Number::Seventh;
            break;
        case 11: // Major seventh
            quality = Quality::Major;
            number = Number::Seventh;
            break;
        case 12: // Perfect octave
            quality = Quality::Perfect;
            number = Number::Octave;
            break;
        default:
            throw std::invalid_argument("Unsupported interval size");
    }
    
    return Interval(quality, number, semitones);
}

Interval Interval::fromQualityAndNumber(Quality quality, Number number) {
    int semitones;
    
    // Base intervals in semitones for perfect/major intervals
    switch (number) {
        case Number::Unison:
            semitones = 0;
            break;
        case Number::Second:
            semitones = 2;
            break;
        case Number::Third:
            semitones = 4;
            break;
        case Number::Fourth:
            semitones = 5;
            break;
        case Number::Fifth:
            semitones = 7;
            break;
        case Number::Sixth:
            semitones = 9;
            break;
        case Number::Seventh:
            semitones = 11;
            break;
        case Number::Octave:
            semitones = 12;
            break;
        default:
            throw std::invalid_argument("Invalid interval number");
    }
    
    // Adjust semitones based on quality
    switch (quality) {
        case Quality::Perfect:
            // No adjustment needed for perfect intervals
            break;
        case Quality::Major:
            // No adjustment needed for major intervals
            break;
        case Quality::Minor:
            semitones--;
            break;
        case Quality::Augmented:
            semitones++;
            break;
        case Quality::Diminished:
            semitones--;
            break;
    }
    
    return Interval(quality, number, semitones);
}

std::string Interval::toString() const {
    std::string qualityStr;
    switch (m_quality) {
        case Quality::Perfect:
            qualityStr = "P";
            break;
        case Quality::Major:
            qualityStr = "M";
            break;
        case Quality::Minor:
            qualityStr = "m";
            break;
        case Quality::Augmented:
            qualityStr = "A";
            break;
        case Quality::Diminished:
            qualityStr = "d";
            break;
    }
    
    return qualityStr + std::to_string(static_cast<int>(m_number));
}

bool Interval::operator==(const Interval& other) const {
    return m_quality == other.m_quality && 
           m_number == other.m_number && 
           m_semitones == other.m_semitones;
}

bool Interval::operator!=(const Interval& other) const {
    return !(*this == other);
}

bool Interval::operator<(const Interval& other) const {
    return m_semitones < other.m_semitones;
}

bool Interval::isDissonant(int semitones) {
    semitones = std::abs(semitones) % 12;
    // Seconds, sevenths, and tritone are dissonant
    return (semitones == 1 || semitones == 2 ||    // Seconds
            semitones == 6 ||                      // Tritone
            semitones == 10 || semitones == 11);    // Sevenths
}

bool Interval::isConsonant(int semitones) {
    semitones = std::abs(semitones) % 12;
    // Perfect consonances: unison, fifth, octave
    if (semitones == 0 || semitones == 7) return true;
    
    // Imperfect consonances: thirds and sixths
    if (semitones == 3 || semitones == 4 ||    // Thirds
        semitones == 8 || semitones == 9)      // Sixths
        return true;
    
    return false;
}

bool Interval::isStepwise(int semitones) {
    semitones = std::abs(semitones);
    return semitones == 1 || semitones == 2;  // Half step or whole step
}

std::string Interval::getIntervalName(int semitones) {
    semitones = std::abs(semitones) % 12;
    switch (semitones) {
        case 0: return "unison/octave";
        case 1: return "minor second";
        case 2: return "major second";
        case 3: return "minor third";
        case 4: return "major third";
        case 5: return "perfect fourth";
        case 6: return "tritone";
        case 7: return "perfect fifth";
        case 8: return "minor sixth";
        case 9: return "major sixth";
        case 10: return "minor seventh";
        case 11: return "major seventh";
        default: return "unknown interval";
    }
}

int Interval::getIntervalInSemitones(const Pitch& p1, const Pitch& p2) {
    return p2.getMidiNote() - p1.getMidiNote();
}

bool Interval::isValidMelodicInterval(int semitones) {
    semitones = std::abs(semitones);
    
    // Valid intervals:
    // - Stepwise motion (1-2 semitones)
    // - Minor/major thirds (3-4 semitones)
    // - Perfect fourth (5 semitones)
    // - Perfect fifth (7 semitones)
    // - Minor/major sixth (8-9 semitones)
    // - Octave (12 semitones)
    
    switch (semitones) {
        case 1: case 2:  // Steps
        case 3: case 4:  // Thirds
        case 5:          // Fourth
        case 7:          // Fifth
        case 8: case 9:  // Sixths
        case 12:         // Octave
            return true;
        default:
            return false;
    }
}

bool Interval::isDiminishedInterval(int semitones) {
    semitones = std::abs(semitones) % 12;
    
    // Diminished intervals:
    // - Diminished fifth (6 semitones)
    // - Diminished octave (11 semitones)
    return semitones == 6 || semitones == 11;
}

} // namespace music
} // namespace MusicTrainer
