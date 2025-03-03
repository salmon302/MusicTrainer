#include "adapters/InMemoryScoreRepository.h"
#include "domain/errors/ErrorBase.h"
#include "domain/errors/DomainErrors.h"
#include <iostream>

using namespace MusicTrainer;

namespace music::adapters {

std::unique_ptr<InMemoryScoreRepository> InMemoryScoreRepository::create() {
	return std::unique_ptr<InMemoryScoreRepository>(new InMemoryScoreRepository());
}

void InMemoryScoreRepository::save(const std::string& name, const MusicTrainer::music::Score& score) {
	std::cout << "[Repo] Saving score: " << name << std::endl;
	std::cout << "[Repo] Creating initial snapshot" << std::endl;
	
	auto snapshot = score.createSnapshot();
	auto newScore = std::make_unique<MusicTrainer::music::Score>(snapshot);
	
	updateScores(name, [&](auto& scores) {
		scores[name] = std::move(newScore);
	});
	
	std::cout << "[Repo] Score saved successfully" << std::endl;
}

std::unique_ptr<MusicTrainer::music::Score> InMemoryScoreRepository::load(const std::string& name) {
	auto it = scores.find(name);
	if (it == scores.end()) {
		MusicTrainer::ErrorContext context(__FILE__, __LINE__, __FUNCTION__,
			"Attempted to load non-existent score: " + name);
		throw MusicTrainer::RepositoryError("Score not found: " + name, context);
	}
	
	auto snapshot = it->second->createSnapshot();
	return std::make_unique<MusicTrainer::music::Score>(snapshot);
}

std::vector<std::string> InMemoryScoreRepository::listScores() {
	std::vector<std::string> names;
	names.reserve(scores.size());
	for (const auto& [name, _] : scores) {
		names.push_back(name);
	}
	return names;
}

void InMemoryScoreRepository::remove(const std::string& name) {
	updateScores(name, [&](auto& scores) {
		scores.erase(name);
	});
}

} // namespace music::adapters

