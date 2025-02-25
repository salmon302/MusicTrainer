#ifndef MUSICTRAINERV3_SNAPSHOT_H
#define MUSICTRAINERV3_SNAPSHOT_H

#include <memory>
#include <chrono>
#include <vector>
#include <atomic>
#include "../music/Score.h"

namespace music::events {

class Snapshot {
public:
	static std::unique_ptr<Snapshot> create(const Score& score);
	Snapshot(Score score);
	
	// Reconstruct score from snapshot
	std::unique_ptr<Score> reconstruct() const;
	
	// Clone this snapshot
	std::unique_ptr<Snapshot> clone() const;
	
	// Get the version this snapshot represents
	size_t getVersion() const {
		return version.load(std::memory_order_acquire);
	}
	
	// Set the version
	void setVersion(size_t v) {
		version.store(v, std::memory_order_release);
	}
	
	// Get the score snapshot
	Score::ScoreSnapshot getSnapshot() const {
		return score.createSnapshot();
	}
	
	// Get the timestamp when this snapshot was created
	std::chrono::system_clock::time_point getTimestamp() const { 
		return std::chrono::system_clock::time_point(
			std::chrono::system_clock::duration(
				timestamp.load(std::memory_order_acquire)
			)
		); 
	}

private:
	Score score;
	std::atomic<std::chrono::system_clock::time_point::rep> timestamp{
		std::chrono::system_clock::now().time_since_epoch().count()
	};
	std::atomic<size_t> version{0};
};

} // namespace music::events

#endif // MUSICTRAINERV3_SNAPSHOT_H
