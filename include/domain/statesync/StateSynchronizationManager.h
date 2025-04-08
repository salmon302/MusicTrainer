#ifndef MUSICTRAINERV3_STATESYNCHRONIZATIONMANAGER_H
#define MUSICTRAINERV3_STATESYNCHRONIZATIONMANAGER_H

#include "domain/events/EventBus.h"
#include "domain/events/ScoreUpdatedEvent.h"
#include "domain/music/Score.h"
#include <iostream>
#include <memory>
#include <atomic>

namespace music::statesync {

class StateSynchronizationManager {
    MusicTrainer::music::events::EventBus& eventBus;

public:
    StateSynchronizationManager(MusicTrainer::music::events::EventBus& eventBus) 
        : eventBus(eventBus) {}
    
    void synchronize(const MusicTrainer::music::Score& score) {
        try {
            auto snapshot = score.createSnapshot();
            auto event = std::make_unique<music::events::ScoreUpdatedEvent>(
                snapshot, score.getVersion());
            eventBus.publishAsync(std::move(event));
        } catch (const std::exception& e) {
            // Handle synchronization errors
            std::cerr << "State synchronization failed: " << e.what() << std::endl;
        }
    }
};

} // namespace music::statesync

#endif // MUSICTRAINERV3_STATESYNCHRONIZATIONMANAGER_H

