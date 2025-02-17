#include "adapters/InMemoryScoreRepository.h"
#include "utils/TrackedLock.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/DomainErrors.h"
#include <iostream>

using namespace MusicTrainer;

namespace music::adapters {

std::unique_ptr<InMemoryScoreRepository> InMemoryScoreRepository::create() {
	return std::unique_ptr<InMemoryScoreRepository>(new InMemoryScoreRepository());
}

void InMemoryScoreRepository::save(const std::string& name, const Score& score) {
	std::cout << "[Repo] Saving score: " << name << std::endl;
	std::cout << "[Repo] Creating initial snapshot" << std::endl;
	
	// First get a snapshot without holding any locks
	auto snapshot = score.createSnapshot();
	auto newScore = std::make_unique<Score>(snapshot);
	
	// Then store the score under repository lock
	{
		::utils::TrackedUniqueLock lock(mutex_, "InMemoryScoreRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		scores[name] = std::move(newScore);
	}
	std::cout << "[Repo] Score saved successfully" << std::endl;
}

std::unique_ptr<Score> InMemoryScoreRepository::load(const std::string& name) {
	Score::ScoreSnapshot snapshot;
	{
		::utils::TrackedSharedMutexLock lock(mutex_, "InMemoryScoreRepository::mutex_", ::utils::LockLevel::REPOSITORY);
		auto it = scores.find(name);
		if (it == scores.end()) {
			MusicTrainer::ErrorContext context(__FILE__, __LINE__, __FUNCTION__,
				"Attempted to load non-existent score: " + name);
			throw MusicTrainer::RepositoryError("Score not found: " + name, context);
		}
		// Get snapshot without holding Score locks
		snapshot = it->second->createSnapshot();
	}
	// Create new Score outside of repository lock
	return std::make_unique<Score>(snapshot);
}

std::vector<std::string> InMemoryScoreRepository::listScores() {
	::utils::TrackedSharedMutexLock lock(mutex_, "InMemoryScoreRepository::mutex_", ::utils::LockLevel::REPOSITORY);
	std::vector<std::string> names;
	names.reserve(scores.size());
	for (const auto& [name, _] : scores) {
		names.push_back(name);
	}
	return names;
}

void InMemoryScoreRepository::remove(const std::string& name) {
	::utils::TrackedUniqueLock lock(mutex_, "InMemoryScoreRepository::mutex_", ::utils::LockLevel::REPOSITORY);
	scores.erase(name);
}

} // namespace music::adapters
