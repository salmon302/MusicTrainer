#include "adapters/CachingScoreRepository.h"
#include "domain/music/Score.h"
#include "domain/errors/DomainErrors.h"
#include <algorithm>
#include <iostream>

namespace music::adapters {

std::unique_ptr<CachingScoreRepository> CachingScoreRepository::create(std::unique_ptr<ports::ScoreRepository> baseRepository) {
	return std::unique_ptr<CachingScoreRepository>(new CachingScoreRepository(std::move(baseRepository)));
}

CachingScoreRepository::CachingScoreRepository(std::unique_ptr<ports::ScoreRepository> baseRepository)
	: baseRepository(std::move(baseRepository)) {}

CachingScoreRepository::~CachingScoreRepository() = default;

void CachingScoreRepository::save(const std::string& name, const MusicTrainer::music::Score& score) {
	try {
		baseRepository->save(name, score);
		updateCache(name, [&score, &name](auto& newCache) {
			auto& entry = newCache[name];
			entry.score = std::make_unique<MusicTrainer::music::Score>(score);
			entry.lastAccess = std::chrono::system_clock::now();
			entry.valid.store(true);
		});
	} catch (const MusicTrainer::RepositoryError& e) {
		if (errorHandler) {
			errorHandler(e);
		}
		throw;
	}
}

std::unique_ptr<MusicTrainer::music::Score> CachingScoreRepository::load(const std::string& name) {
	totalAccesses.fetch_add(1);
	
	// Check cache first
	auto it = cache.find(name);
	if (it != cache.end() && it->second.score && !isExpired(it->second)) {
		cacheHits.fetch_add(1);
		it->second.lastAccess = std::chrono::system_clock::now();
		return std::make_unique<MusicTrainer::music::Score>(*it->second.score);
	}
	
	try {
		auto result = baseRepository->load(name);
		if (result) {
			updateCache(name, [&result, &name](auto& newCache) {
				auto& newEntry = newCache[name];
				newEntry.score = std::make_unique<MusicTrainer::music::Score>(*result);
				newEntry.lastAccess = std::chrono::system_clock::now();
				newEntry.valid.store(true);
			});
		}
		return result;
	} catch (const MusicTrainer::RepositoryError& e) {
		if (recoveryStrategy) {
			auto recoveryResult = recoveryStrategy(e);
			if (recoveryResult.success()) {
				return std::move(recoveryResult.score);
			}
		}
		if (errorHandler) {
			errorHandler(e);
		}
		throw;
	}
}

std::vector<std::string> CachingScoreRepository::listScores() {
	try {
		return baseRepository->listScores();
	} catch (const MusicTrainer::RepositoryError& e) {
		if (errorHandler) {
			errorHandler(e);
		}
		throw;
	}
}

void CachingScoreRepository::remove(const std::string& name) {
	try {
		baseRepository->remove(name);
		updateCache(name, [&name](auto& newCache) {
			newCache.erase(name);
		});
	} catch (const MusicTrainer::RepositoryError& e) {
		if (errorHandler) {
			errorHandler(e);
		}
		throw;
	}
}

void CachingScoreRepository::clearCache() {
	cache.clear();
	cacheHits.store(0);
	totalAccesses.store(0);
}

void CachingScoreRepository::setCacheTimeout(std::chrono::seconds timeout) {
	cacheTimeout.store(timeout.count());
}

size_t CachingScoreRepository::getCacheSize() const {
	return cache.size();
}

double CachingScoreRepository::getCacheHitRate() const {
	auto total = totalAccesses.load();
	if (total == 0) return 0.0;
	return static_cast<double>(cacheHits.load()) / total;
}

void CachingScoreRepository::cleanExpiredEntries() {
	auto now = std::chrono::system_clock::now();
	for (auto it = cache.begin(); it != cache.end();) {
		if (isExpired(it->second)) {
			it = cache.erase(it);
		} else {
			++it;
		}
	}
}

bool CachingScoreRepository::isExpired(const CacheEntry& entry) const {
	auto now = std::chrono::system_clock::now();
	auto age = std::chrono::duration_cast<std::chrono::seconds>(now - entry.lastAccess);
	return age.count() > cacheTimeout.load() || !entry.valid.load();
}

} // namespace music::adapters
