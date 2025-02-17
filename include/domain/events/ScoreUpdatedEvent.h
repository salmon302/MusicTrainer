#pragma once

#include "Event.h"
#include "domain/music/Score.h"
#include "domain/music/Voice.h"
#include <string>
#include <chrono>
#include <memory>

namespace music::events {

class ScoreUpdatedEvent : public Event {
public:
	ScoreUpdatedEvent(const Score::ScoreSnapshot& snapshot, uint64_t version)
		: version(version)
		, snapshot(snapshot)
		, timestamp(std::chrono::system_clock::now()) {}
		
	~ScoreUpdatedEvent() override = default;

	Score::ScoreSnapshot getSnapshot() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		return snapshot;
	}
	
	uint64_t getVersion() const {
		::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		return version;
	}

	// Event interface implementation
	std::string getType() const override { return "ScoreUpdated"; }
	
	void apply(Score& target) const override {
		// Get snapshot data under our lock
		Score::ScoreSnapshot snapshotCopy;
		uint64_t versionCopy;
		{
			::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
			snapshotCopy = snapshot;
			versionCopy = version;
		}
		
		// Apply snapshot data to target score (Score handles its own locking)
		target.setTimeSignature(snapshotCopy.timeSignature);
		target.setVersion(versionCopy);
		
		// Create and prepare voices before adding them to score
		std::vector<std::unique_ptr<Voice>> voices;
		for (const auto& notes : snapshotCopy.voiceNotes) {
			auto voice = Voice::create(snapshotCopy.timeSignature);
			for (const auto& note : notes) {
				voice->addNote(note.pitch, note.duration);
			}
			voices.push_back(std::move(voice));
		}
		
		// Add voices to score (Score::addVoice handles proper lock ordering)
		for (auto& voice : voices) {
			target.addVoice(std::move(voice));
		}
	}
	
	std::chrono::system_clock::time_point getTimestamp() const override {
		::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		return timestamp;
	}
	
	std::string getDescription() const override {
		::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
		return "Score updated to version " + std::to_string(version);
	}
	
	std::unique_ptr<Event> clone() const override {
		Score::ScoreSnapshot snapshotCopy;
		{
			::utils::TrackedSharedMutexLock lock(mutex_, "Event::mutex_", ::utils::LockLevel::REPOSITORY);
			snapshotCopy = snapshot;
		}
		return std::make_unique<ScoreUpdatedEvent>(snapshotCopy, version);
	}

private:
	uint64_t version;
	Score::ScoreSnapshot snapshot;
	std::chrono::system_clock::time_point timestamp;
};

} // namespace music::events
