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
        uint8_t beats;
        uint8_t beatType;
        
        static TimeSignature commonTime() {
            return {4, 4};
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

