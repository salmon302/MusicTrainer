#pragma once

#include "Event.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include <string>
#include <chrono>
#include <memory>
#include <atomic>

namespace music::events {

class ScoreUpdatedEvent : public MusicTrainer::music::events::Event {
public:
    ScoreUpdatedEvent(const MusicTrainer::music::Score::ScoreSnapshot& snapshot, uint64_t version)
        : version_(version)
        , snapshot_(snapshot)
        , timestamp_(std::chrono::system_clock::now().time_since_epoch().count()) {}
        
    ~ScoreUpdatedEvent() override = default;

    const MusicTrainer::music::Score::ScoreSnapshot& getSnapshot() const {
        return snapshot_;
    }
    
    uint64_t getVersion() const {
        return version_.load(std::memory_order_acquire);
    }

    // Event interface implementation
    std::string getType() const override { return "ScoreUpdated"; }
    
    void apply(MusicTrainer::music::Score& target) const override {
        // Apply snapshot data to target score
        target.setTimeSignature(snapshot_.timeSignature);
        target.setVersion(version_.load(std::memory_order_acquire));
        
        // Create and prepare voices before adding them to score
        std::vector<std::unique_ptr<MusicTrainer::music::Voice>> voices;
        for (const auto& notes : snapshot_.voiceNotes) {
            auto voice = MusicTrainer::music::Voice::create(snapshot_.timeSignature);
            for (const auto& note : notes) {
                voice->addNote(note.getPitch(), note.getDuration());
            }
            voices.push_back(std::move(voice));
        }
        
        // Add voices to score
        for (auto& voice : voices) {
            target.addVoice(std::move(voice));
        }
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return std::chrono::system_clock::time_point(
            std::chrono::system_clock::duration(
                timestamp_.load(std::memory_order_acquire)
            )
        );
    }
    
    std::string getDescription() const override {
        return "Score updated to version " + std::to_string(version_.load(std::memory_order_acquire));
    }
    
    std::unique_ptr<MusicTrainer::music::events::Event> clone() const override {
        return std::make_unique<ScoreUpdatedEvent>(snapshot_, version_.load(std::memory_order_acquire));
    }

private:
    std::atomic<uint64_t> version_;
    const MusicTrainer::music::Score::ScoreSnapshot snapshot_;  // Immutable after construction
    std::atomic<std::chrono::system_clock::time_point::rep> timestamp_;
};

} // namespace music::events


