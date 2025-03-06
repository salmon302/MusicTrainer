#pragma once

#include "domain/music/Note.h"
#include "domain/music/Duration.h"
#include "domain/music/Pitch.h"
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <atomic>

namespace MusicTrainer {
namespace music {

class Voice {
public:
    struct TimeSignature {
        uint8_t beats = 4;  // Initialize with common time defaults
        uint8_t beatType = 4;
        
        TimeSignature() = default;  // Default constructor will use member initializers

        TimeSignature(int beatsArg, const Duration& beatDuration) {
            beats = beatsArg;
            // Set beatType based on Duration - assuming Duration values map to standard notation
            // where 1 = whole, 2 = half, 4 = quarter, 8 = eighth, 16 = sixteenth, etc.
            if (beatDuration.isWhole()) {
                beatType = 1;
            } else if (beatDuration.isHalf()) {
                beatType = 2;
            } else if (beatDuration.isQuarter()) {
                beatType = 4;
            } else if (beatDuration.isEighth()) {
                beatType = 8;
            } else if (beatDuration.isSixteenth()) {
                beatType = 16;
            } else {
                beatType = 4; // Default to quarter note if unrecognized
            }
        }
        
        static TimeSignature commonTime() {
            return TimeSignature{};  // Will use the default member initializers
        }
        
        bool operator==(const TimeSignature& other) const {
            return beats == other.beats && beatType == other.beatType;
        }
    };

    // Factory method
    static std::unique_ptr<Voice> create(const TimeSignature& timeSignature = TimeSignature::commonTime());
    
    // Constructors and assignment operators
    Voice(const Voice& other);
    Voice& operator=(const Voice& other);
    ~Voice();

    // Note management
    void addNote(const Pitch& pitch, double duration, int position = 0);
    void removeNote(int position);
    void clearNotes();
    std::vector<Note> getAllNotes() const;
    Note* getNoteAt(int position);
    const Note* getNoteAt(int position) const;
    std::vector<Note> getNotesInRange(size_t startMeasure, size_t endMeasure) const;

    // Time signature management
    const TimeSignature& getTimeSignature() const;
    void setTimeSignature(const TimeSignature& newTimeSignature);

    // Utility methods
    int getFirstNotePosition() const;
    int getLastNotePosition() const;
    int getDuration() const;
    size_t getHash() const;
    std::unique_ptr<Voice> clone() const;

private:
    explicit Voice(const TimeSignature& timeSignature);
    class VoiceImpl;
    std::unique_ptr<VoiceImpl> m_impl;
};

} // namespace music
} // namespace MusicTrainer

