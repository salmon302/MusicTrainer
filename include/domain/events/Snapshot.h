#ifndef MUSICTRAINERV3_SNAPSHOT_H
#define MUSICTRAINERV3_SNAPSHOT_H

#include <memory>
#include <chrono>
#include <vector>
#include <shared_mutex>
#include "Event.h"
#include "../../utils/TrackedLock.h"
#include "../music/Score.h"

namespace music::events {

class Snapshot {
public:
	static std::unique_ptr<Snapshot> create(const Score& score);
	Snapshot(Score score);  // Made public
	
	// Reconstruct score from snapshot
	std::unique_ptr<Score> reconstruct() const;
	
	// Clone this snapshot
	std::unique_ptr<Snapshot> clone() const;  // Declaration only
	
	// Get the version this snapshot represents
	size_t getVersion() const {
		::utils::TrackedUniqueLock lock(mutex_, "Snapshot::mutex_", ::utils::LockLevel::REPOSITORY);
		return version;
	}
	
	// Set the version
	void setVersion(size_t v) {
		::utils::TrackedUniqueLock lock(mutex_, "Snapshot::mutex_", ::utils::LockLevel::REPOSITORY);
		version = v;
	}
	
	// Get the score snapshot
	Score::ScoreSnapshot getSnapshot() const {
		::utils::TrackedUniqueLock lock(mutex_, "Snapshot::mutex_", ::utils::LockLevel::REPOSITORY);
		return score.createSnapshot();
	}
	
	// Get the timestamp when this snapshot was created
	std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }

private:

	Score score;
	std::chrono::system_clock::time_point timestamp;
	size_t version{0};
	mutable std::shared_mutex mutex_;  // Mutex for thread safety
};

} // namespace music::events

#endif // MUSICTRAINERV3_SNAPSHOT_H
