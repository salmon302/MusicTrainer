#include "domain/music/Voice.h"
#include <algorithm>
#include <cstdint>

namespace MusicTrainer {
namespace music {

class Voice::VoiceImpl {
public:
    std::vector<Note> notes;
    TimeSignature timeSignature;
    mutable std::vector<size_t> measureNumbers;
    mutable std::atomic<bool> measureNumbersValid;

    explicit VoiceImpl(const TimeSignature& ts) 
        : timeSignature(ts)
        , measureNumbersValid(false) {}

    // Custom copy constructor to handle atomic member
    VoiceImpl(const VoiceImpl& other)
        : notes(other.notes)
        , timeSignature(other.timeSignature)
        , measureNumbers(other.measureNumbers)
        , measureNumbersValid(other.measureNumbersValid.load(std::memory_order_acquire)) {}
};

std::unique_ptr<Voice> Voice::create(const TimeSignature& timeSignature) {
    return std::unique_ptr<Voice>(new Voice(timeSignature));
}

Voice::Voice(const TimeSignature& timeSignature)
    : m_impl(std::make_unique<VoiceImpl>(timeSignature)) {}

Voice::Voice(const Voice& other)
    : m_impl(std::make_unique<VoiceImpl>(*other.m_impl)) {}

Voice& Voice::operator=(const Voice& other) {
    if (this != &other) {
        m_impl = std::make_unique<VoiceImpl>(*other.m_impl);
    }
    return *this;
}

Voice::~Voice() = default;

void Voice::addNote(const Pitch& pitch, double duration, int position) {
    m_impl->notes.emplace_back(pitch, duration, position);
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

void Voice::removeNote(int position) {
    auto it = std::find_if(m_impl->notes.begin(), m_impl->notes.end(),
        [position](const Note& note) { return note.getPosition() == position; });
    if (it != m_impl->notes.end()) {
        m_impl->notes.erase(it);
        m_impl->measureNumbersValid.store(false, std::memory_order_release);
    }
}

void Voice::clearNotes() {
    m_impl->notes.clear();
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

const std::vector<Note>& Voice::getAllNotes() const {
    return m_impl->notes;
}

Note* Voice::getNoteAt(int position) {
    auto it = std::find_if(m_impl->notes.begin(), m_impl->notes.end(),
        [position](const Note& note) { return note.getPosition() == position; });
    return it != m_impl->notes.end() ? &(*it) : nullptr;
}

const Note* Voice::getNoteAt(int position) const {
    auto it = std::find_if(m_impl->notes.begin(), m_impl->notes.end(),
        [position](const Note& note) { return note.getPosition() == position; });
    return it != m_impl->notes.end() ? &(*it) : nullptr;
}

std::vector<Note> Voice::getNotesInRange(size_t startMeasure, size_t endMeasure) const {
    std::vector<Note> result;
    for (const auto& note : m_impl->notes) {
        size_t noteMeasure = note.getPosition() / m_impl->timeSignature.beats;
        if (noteMeasure >= startMeasure && noteMeasure <= endMeasure) {
            result.push_back(note);
        }
    }
    return result;
}

const Voice::TimeSignature& Voice::getTimeSignature() const {
    return m_impl->timeSignature;
}

void Voice::setTimeSignature(const TimeSignature& newTimeSignature) {
    m_impl->timeSignature = newTimeSignature;
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

int Voice::getFirstNotePosition() const {
    if (m_impl->notes.empty()) return 0;
    return m_impl->notes.front().getPosition();
}

int Voice::getLastNotePosition() const {
    if (m_impl->notes.empty()) return 0;
    return m_impl->notes.back().getPosition();
}

int Voice::getDuration() const {
    if (m_impl->notes.empty()) return 0;
    int maxPos = 0;
    for (const auto& note : m_impl->notes) {
        maxPos = std::max(maxPos, note.getPosition() + static_cast<int>(note.getDuration()));
    }
    return maxPos;
}

size_t Voice::getHash() const {
    size_t hash = std::hash<uint8_t>{}(m_impl->timeSignature.beats);
    for (const auto& note : m_impl->notes) {
        hash = hash * 31 + std::hash<int>{}(note.getPosition());
        hash = hash * 31 + std::hash<double>{}(note.getDuration());
    }
    return hash;
}

std::unique_ptr<Voice> Voice::clone() const {
    return std::make_unique<Voice>(*this);
}

} // namespace music
} // namespace MusicTrainer





