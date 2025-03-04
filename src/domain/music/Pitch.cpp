#include "domain/music/Pitch.h"
#include <array>

namespace MusicTrainer {
namespace music {

namespace {
    // Base note values for C major scale (relative to C for each octave)
    constexpr std::array<uint8_t, 7> noteToMidi = {0, 2, 4, 5, 7, 9, 11};  // Semitones from C
    constexpr std::array<const char*, 7> noteNames = {"C", "D", "E", "F", "G", "A", "B"};
}

Pitch Pitch::create(NoteName note, int8_t octave, int8_t accidental) {
    return Pitch(note, octave, accidental);
}

Pitch Pitch::fromMidiNote(uint8_t midiNote) {
    // Convert MIDI note to note name and octave
    int octave = (midiNote / 12) - 1;  // MIDI note 60 is middle C (C4)
    int noteInOctave = midiNote % 12;
    
    // Map the note within the octave to note name and accidental
    static const std::array<std::pair<NoteName, int8_t>, 12> midiToNote = {
        std::make_pair(NoteName::C, 0),    // C
        std::make_pair(NoteName::C, 1),    // C#
        std::make_pair(NoteName::D, 0),    // D
        std::make_pair(NoteName::D, 1),    // D#
        std::make_pair(NoteName::E, 0),    // E
        std::make_pair(NoteName::F, 0),    // F
        std::make_pair(NoteName::F, 1),    // F#
        std::make_pair(NoteName::G, 0),    // G
        std::make_pair(NoteName::G, 1),    // G#
        std::make_pair(NoteName::A, 0),    // A
        std::make_pair(NoteName::A, 1),    // A#
        std::make_pair(NoteName::B, 0)     // B
    };
    
    return Pitch(midiToNote[noteInOctave].first, octave, midiToNote[noteInOctave].second);
}

Pitch::Pitch(NoteName note, int8_t octave, int8_t accidental)
    : noteName(note), octave(octave), accidental(accidental) {}

uint8_t Pitch::getMidiNote() const {
    // Convert note name to semitones from C, then add octave offset and accidentals
    uint8_t base = noteToMidi[static_cast<int>(noteName)];
    return base + (octave + 1) * 12 + accidental;  // Fixed order of operations
}

std::string Pitch::toString() const {
    std::string result;
    result = noteNames[static_cast<int>(noteName)];
    
    for (int i = 0; i < accidental; ++i) result += "#";
    for (int i = 0; i > accidental; --i) result += "b";
    
    result += std::to_string(octave);
    return result;
}

bool Pitch::operator==(const Pitch& other) const {
    return noteName == other.noteName && octave == other.octave && accidental == other.accidental;
}

bool Pitch::operator!=(const Pitch& other) const {
    return !(*this == other);
}

} // namespace music
} // namespace MusicTrainer


