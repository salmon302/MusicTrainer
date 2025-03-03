#pragma once

#include "domain/music/Voice.h"
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <optional>

namespace MusicTrainer {
namespace music {

class Score {
public:
    struct ScoreSnapshot {
        std::vector<std::vector<Note>> voiceNotes;
        Voice::TimeSignature timeSignature;
    };

    static std::unique_ptr<Score> create(const Voice::TimeSignature& timeSignature = Voice::TimeSignature::commonTime());
    explicit Score(const ScoreSnapshot& snapshot);
    Score(const Score& other);
    Score& operator=(const Score& other);
    
    void addVoice(std::unique_ptr<Voice> voice) {
        if (!voice) return;
        voice->setTimeSignature(timeSignature);
        voices.push_back(std::move(voice));
        measureCountValid.store(false, std::memory_order_release);
    }

    size_t getVoiceCount() const {
        return voices.size();
    }

    const Voice* getVoice(size_t index) const {
        return index < voices.size() ? voices[index].get() : nullptr;
    }

    Voice* getVoice(size_t index) {
        return index < voices.size() ? voices[index].get() : nullptr;
    }

    const std::vector<std::unique_ptr<Voice>>& getVoices() const {
        return voices;
    }

    size_t getMeasureCount() const;
    
    ScoreSnapshot createSnapshot() const {
        ScoreSnapshot snapshot;
        snapshot.timeSignature = timeSignature;
        snapshot.voiceNotes.reserve(voices.size());
        
        for (const auto& voice : voices) {
            if (!voice) continue;
            snapshot.voiceNotes.push_back(voice->getNotesInRange(0, SIZE_MAX));
        }
        
        return snapshot;
    }
    
    const Voice::TimeSignature& getTimeSignature() const {
        return timeSignature;
    }

    void setTimeSignature(const Voice::TimeSignature& newTimeSignature) {
        for (auto& voice : voices) {
            if (!voice) continue;
            voice->setTimeSignature(newTimeSignature);
        }
        timeSignature = newTimeSignature;
        measureCountValid.store(false, std::memory_order_release);
    }

    size_t getHash() const;
    
    std::string getName() const {
        return name;
    }

    void setName(const std::string& newName) {
        name = newName;
    }

    uint64_t getVersion() const {
        return version.load(std::memory_order_acquire);
    }

    void setVersion(uint64_t newVersion) {
        version.store(newVersion, std::memory_order_release);
    }

    // Position tracking methods
    std::optional<int> getCurrentPosition() const {
        return currentPosition;
    }

    void setCurrentPosition(int position) {
        currentPosition = position;
    }

    bool isDirty() const {
        return isDirtyFlag.load(std::memory_order_acquire);
    }

    void setDirty(bool dirty = true) {
        isDirtyFlag.store(dirty, std::memory_order_release);
    }

private:
    explicit Score(const Voice::TimeSignature& timeSignature);
    void performCopy(const Score& other);

    std::vector<std::unique_ptr<Voice>> voices;
    Voice::TimeSignature timeSignature;
    mutable size_t cachedMeasureCount{0};
    mutable std::atomic<bool> measureCountValid{false};
    std::string name;
    std::atomic<uint64_t> version{0};
    std::optional<int> currentPosition;
    std::atomic<bool> isDirtyFlag{false};
};

} // namespace music
} // namespace MusicTrainer


