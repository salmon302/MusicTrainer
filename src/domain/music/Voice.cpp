#include "domain/music/Voice.h"
#include <algorithm>
#include <cstdint>
#include <map>
#include <shared_mutex>
#include <mutex>

namespace MusicTrainer {
namespace music {

class Voice::VoiceImpl {
public:
    explicit VoiceImpl(const TimeSignature& ts) : timeSignature(ts), measureNumbersValid(false) {}
    
    // Custom copy constructor to handle non-copyable members
    VoiceImpl(const VoiceImpl& other) 
        : notes(other.notes)
        , timeSignature(other.timeSignature)
        , measureNumbersValid(other.measureNumbersValid.load()) {}
    
    std::map<int, Note> notes;  // Position -> Note map for O(log n) access
    TimeSignature timeSignature;
    std::atomic<bool> measureNumbersValid;
    mutable std::shared_mutex mutex;
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
    std::unique_lock<std::shared_mutex> lock(m_impl->mutex);
    m_impl->notes.insert_or_assign(position, Note(pitch, duration, position));
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

void Voice::removeNote(int position) {
    std::unique_lock<std::shared_mutex> lock(m_impl->mutex);
    m_impl->notes.erase(position);
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

void Voice::clearNotes() {
    std::unique_lock<std::shared_mutex> lock(m_impl->mutex);
    m_impl->notes.clear();
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

std::vector<Note> Voice::getAllNotes() const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    std::vector<Note> result;
    result.reserve(m_impl->notes.size());
    for (const auto& [pos, note] : m_impl->notes) {
        result.push_back(note);
    }
    return result;
}

Note* Voice::getNoteAt(int position) {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    auto it = m_impl->notes.find(position);
    return it != m_impl->notes.end() ? &it->second : nullptr;
}

const Note* Voice::getNoteAt(int position) const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    auto it = m_impl->notes.find(position);
    return it != m_impl->notes.end() ? &it->second : nullptr;
}

std::vector<Note> Voice::getNotesInRange(size_t startMeasure, size_t endMeasure) const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    std::vector<Note> result;
    int startPos = startMeasure * m_impl->timeSignature.beats;
    int endPos = (endMeasure + 1) * m_impl->timeSignature.beats;
    
    auto startIt = m_impl->notes.lower_bound(startPos);
    auto endIt = m_impl->notes.upper_bound(endPos);
    
    for (auto it = startIt; it != endIt; ++it) {
        result.push_back(it->second);
    }
    return result;
}

const Voice::TimeSignature& Voice::getTimeSignature() const {
    return m_impl->timeSignature;
}

void Voice::setTimeSignature(const TimeSignature& newTimeSignature) {
    std::unique_lock<std::shared_mutex> lock(m_impl->mutex);
    m_impl->timeSignature = newTimeSignature;
    m_impl->measureNumbersValid.store(false, std::memory_order_release);
}

int Voice::getFirstNotePosition() const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    return m_impl->notes.empty() ? 0 : m_impl->notes.begin()->first;
}

int Voice::getLastNotePosition() const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    return m_impl->notes.empty() ? 0 : m_impl->notes.rbegin()->first;
}

int Voice::getDuration() const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    if (m_impl->notes.empty()) return 0;
    int maxPos = 0;
    for (const auto& [pos, note] : m_impl->notes) {
        maxPos = std::max(maxPos, pos + static_cast<int>(note.getDuration()));
    }
    return maxPos;
}

size_t Voice::getHash() const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    size_t hash = std::hash<uint8_t>{}(m_impl->timeSignature.beats);
    for (const auto& [pos, note] : m_impl->notes) {
        hash = hash * 31 + std::hash<int>{}(pos);
        hash = hash * 31 + std::hash<double>{}(note.getDuration());
    }
    return hash;
}

std::unique_ptr<Voice> Voice::clone() const {
    std::shared_lock<std::shared_mutex> lock(m_impl->mutex);
    auto newVoice = create(m_impl->timeSignature);
    newVoice->m_impl->notes = m_impl->notes;
    return newVoice;
}

} // namespace music
} // namespace MusicTrainer





