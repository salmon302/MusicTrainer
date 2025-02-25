#ifndef MUSICTRAINERV3_EVENTSOURCEDREPOSITORY_H
#define MUSICTRAINERV3_EVENTSOURCEDREPOSITORY_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <atomic>
#include <functional>
#include "domain/ports/ScoreRepository.h"
#include "domain/events/Event.h"
#include "domain/events/Snapshot.h"

namespace music::adapters {

class EventSourcedRepository : public ports::ScoreRepository {
public:
	static std::unique_ptr<EventSourcedRepository> create();
	
	void save(const std::string& name, const Score& score) override;
	std::unique_ptr<Score> load(const std::string& name) override;
	std::vector<std::string> listScores() override;
	void remove(const std::string& name) override;
	
	void appendEvents(const std::string& name, const std::vector<std::unique_ptr<events::Event>>& events);
	void createSnapshot(const std::string& name);
	std::vector<std::unique_ptr<events::Event>> getEvents(const std::string& name, size_t fromVersion = 0);

private:
	EventSourcedRepository() = default;
	
	struct ScoreState {
		std::vector<std::unique_ptr<events::Event>> events;
		std::unique_ptr<events::Snapshot> snapshot;
		size_t version{0};
		
		ScoreState() = default;
		ScoreState(const ScoreState& other) = delete;
		ScoreState& operator=(const ScoreState& other) = delete;
		ScoreState(ScoreState&& other) = default;
		ScoreState& operator=(ScoreState&& other) = default;
	};
	
	void updateState(const std::string& name, const std::function<void(ScoreState&)>& updateFn) {
		auto it = scoreStates.find(name);
		if (it != scoreStates.end()) {
			auto newState = ScoreState{};
			newState.events.reserve(it->second.events.size());
			for (const auto& event : it->second.events) {
				newState.events.push_back(event->clone());
			}
			if (it->second.snapshot) {
				newState.snapshot = std::unique_ptr<events::Snapshot>(it->second.snapshot->clone());
			}
			newState.version = it->second.version;
			
			updateFn(newState);
			scoreStates[name] = std::move(newState);
		} else {
			ScoreState newState;
			updateFn(newState);
			scoreStates[name] = std::move(newState);
		}
	}
	
	std::unordered_map<std::string, ScoreState> scoreStates;
	std::unique_ptr<Score> reconstructScore(const ScoreState& state);
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_EVENTSOURCEDREPOSITORY_H