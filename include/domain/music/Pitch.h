#ifndef MUSICTRAINERV3_PITCH_H
#define MUSICTRAINERV3_PITCH_H

#include <cstdint>
#include <string>

namespace MusicTrainer {
namespace music {

class Pitch {
public:
    // Default constructor
    Pitch() = default;

    enum class NoteName {
        C, D, E, F, G, A, B
    };

    // Factory method for creating a pitch from note name, octave and accidental
    static Pitch create(NoteName note, int8_t octave, int8_t accidental = 0);
    
    // Factory method for creating a pitch from MIDI note number
    static Pitch fromMidiNote(uint8_t midiNote);
    
    // Immutable accessors
    NoteName getNoteName() const { return noteName; }
    int8_t getOctave() const { return octave; }
    int8_t getAccidental() const { return accidental; }
    uint8_t getMidiNote() const;
    std::string toString() const;

    // Value semantics
    bool operator==(const Pitch& other) const;
    bool operator!=(const Pitch& other) const;
    bool operator<(const Pitch& other) const { return getMidiNote() < other.getMidiNote(); }
    bool operator>(const Pitch& other) const { return getMidiNote() > other.getMidiNote(); }
    bool operator<=(const Pitch& other) const { return getMidiNote() <= other.getMidiNote(); }
    bool operator>=(const Pitch& other) const { return getMidiNote() >= other.getMidiNote(); }

private:
    Pitch(NoteName note, int8_t octave, int8_t accidental);
    
    NoteName noteName;
    int8_t octave;     // MIDI standard: -1 to 9
    int8_t accidental; // Negative for flats, positive for sharps
};

} // namespace music
} // namespace MusicTrainer

#endif // MUSICTRAINERV3_PITCH_H