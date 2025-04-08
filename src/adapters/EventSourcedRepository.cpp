#include "adapters/EventSourcedRepository.h"
#include <chrono>

namespace music::adapters {

std::unique_ptr<EventSourcedRepository> EventSourcedRepository::create() {
    return std::unique_ptr<EventSourcedRepository>(new EventSourcedRepository());
}

void EventSourcedRepository::save(const std::string& name, const MusicTrainer::music::Score& score) {
    createSnapshot(name);
}

std::unique_ptr<MusicTrainer::music::Score> EventSourcedRepository::load(const std::string& name) {
    updateState(name, [](ScoreState& state) {
        if (state.events.empty()) {
            state.events.clear();
            state.version = 0;
        }
    });
    
    auto it = scoreStates.find(name);
    if (it != scoreStates.end()) {
        return reconstructScore(it->second);
    }
    
    return nullptr;
}

std::vector<std::string> EventSourcedRepository::listScores() {
    std::vector<std::string> scores;
    scores.reserve(scoreStates.size());
    for (const auto& [name, _] : scoreStates) {
        scores.push_back(name);
    }
    return scores;
}

void EventSourcedRepository::remove(const std::string& name) {
    scoreStates.erase(name);
}

void EventSourcedRepository::appendEvents(
    const std::string& name,
    const std::vector<std::unique_ptr<MusicTrainer::music::events::Event>>& events) 
{
    updateState(name, [&events](ScoreState& state) {
        for (const auto& event : events) {
            state.events.push_back(event->clone());
        }
        state.version += events.size();
    });
}

void EventSourcedRepository::createSnapshot(const std::string& name) {
    updateState(name, [](ScoreState& state) {
        if (state.events.empty()) {
            state.events.clear();
            state.version = 0;
            state.snapshot.reset();
        }
    });
}

std::vector<std::unique_ptr<MusicTrainer::music::events::Event>> 
EventSourcedRepository::getEvents(const std::string& name, size_t fromVersion) 
{
    std::vector<std::unique_ptr<MusicTrainer::music::events::Event>> result;
    auto it = scoreStates.find(name);
    if (it != scoreStates.end()) {
        // Only copy events if we're starting from a version before the current one
        if (fromVersion < it->second.version) {
            size_t eventIndex = 0;
            for (const auto& event : it->second.events) {
                // Use event index to determine version
                if (eventIndex >= fromVersion) {
                    result.push_back(event->clone());
                }
                eventIndex++;
            }
        }
    }
    return result;
}

std::unique_ptr<MusicTrainer::music::Score> 
EventSourcedRepository::reconstructScore(const ScoreState& state) 
{
    std::unique_ptr<MusicTrainer::music::Score> score;
    
    if (state.snapshot) {
        score = state.snapshot->reconstruct();
    }
    
    if (!score) {
        score = MusicTrainer::music::Score::create(MusicTrainer::music::Voice::TimeSignature::commonTime());
    }
    
    for (const auto& event : state.events) {
        event->apply(*score);
    }
    
    return score;
}

} // namespace music::adapters