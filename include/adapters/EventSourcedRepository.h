#ifndef MUSICTRAINERV3_EVENTSOURCEDREPOSITORY_H
#define MUSICTRAINERV3_EVENTSOURCEDREPOSITORY_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <shared_mutex>
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
	
	// Event sourcing specific methods
	void appendEvents(const std::string& name, const std::vector<std::unique_ptr<events::Event>>& events);
	void createSnapshot(const std::string& name);
	std::vector<std::unique_ptr<events::Event>> getEvents(const std::string& name, size_t fromVersion = 0);

#ifdef TESTING
	std::shared_mutex& getMutexForTesting() const { return mutex_; }
#endif

private:
	EventSourcedRepository() = default;
	
	struct ScoreState {
		std::vector<std::unique_ptr<events::Event>> events;
		std::unique_ptr<events::Snapshot> snapshot;
		size_t version{0};
	};
	
	std::unordered_map<std::string, ScoreState> scoreStates;
	mutable std::shared_mutex mutex_;  // Level 4 (REPOSITORY) - Must be acquired after VALIDATION/METRICS locks
	
	// Helper method to reconstruct score from events
	// Lock ordering: Acquires REPOSITORY lock (level 4) after any needed VALIDATION (2) or METRICS (3) locks
	std::unique_ptr<Score> reconstructScore(const ScoreState& state);
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_EVENTSOURCEDREPOSITORY_H