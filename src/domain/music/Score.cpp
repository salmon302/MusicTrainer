#include "domain/music/Score.h"
#include <algorithm>

namespace MusicTrainer {
namespace music {

std::unique_ptr<Score> Score::create(const Voice::TimeSignature& timeSignature) {
    return std::unique_ptr<Score>(new Score(timeSignature));
}

Score::Score(const Voice::TimeSignature& timeSignature) 
    : timeSignature(timeSignature) {}

Score::Score(const ScoreSnapshot& snapshot) 
    : timeSignature(snapshot.timeSignature) {
    for (const auto& notes : snapshot.voiceNotes) {
        auto voice = Voice::create(timeSignature);
        for (const auto& note : notes) {
            voice->addNote(note.getPitch(), note.getDuration(), note.getPosition());
        }
        addVoice(std::move(voice));
    }
}

Score::Score(const Score& other) {
    performCopy(other);
}

Score& Score::operator=(const Score& other) {
    if (this != &other) {
        performCopy(other);
    }
    return *this;
}

void Score::performCopy(const Score& other) {
    voices.clear();
    for (const auto& voice : other.voices) {
        if (voice) {
            voices.push_back(voice->clone());
        }
    }
    timeSignature = other.timeSignature;
    cachedMeasureCount = other.cachedMeasureCount;
    measureCountValid.store(other.measureCountValid.load(std::memory_order_acquire), 
                          std::memory_order_release);
    name = other.name;
    version.store(other.version.load(std::memory_order_acquire), 
                 std::memory_order_release);
    currentPosition = other.currentPosition;
    isDirtyFlag.store(other.isDirtyFlag.load(std::memory_order_acquire), 
                     std::memory_order_release);
}

size_t Score::getMeasureCount() const {
    if (measureCountValid.load(std::memory_order_acquire)) {
        return cachedMeasureCount;
    }

    size_t maxDuration = 0;
    for (const auto& voice : voices) {
        if (!voice) continue;
        maxDuration = std::max(maxDuration, static_cast<size_t>(voice->getDuration()));
    }

    cachedMeasureCount = maxDuration;
    measureCountValid.store(true, std::memory_order_release);
    return maxDuration;
}

size_t Score::getHash() const {
    size_t hash = std::hash<uint8_t>{}(timeSignature.beats);
    for (const auto& voice : voices) {
        if (!voice) continue;
        hash = hash * 31 + voice->getHash();
    }
    return hash;
}

} // namespace music
} // namespace MusicTrainer