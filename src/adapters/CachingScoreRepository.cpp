#include "adapters/CachingScoreRepository.h"
#include "domain/errors/DomainErrors.h"
#include "utils/TrackedLock.h"
#include <algorithm>
#include <iostream>
#include <optional>

namespace music::adapters {

std::unique_ptr<CachingScoreRepository> CachingScoreRepository::create(std::unique_ptr<ScoreRepository> baseRepository) {
	return std::unique_ptr<CachingScoreRepository>(new CachingScoreRepository(std::move(baseRepository)));
}

CachingScoreRepository::CachingScoreRepository(std::unique_ptr<ScoreRepository> baseRepo)
	: baseRepository(std::move(baseRepo)) {}

void CachingScoreRepository::save(const std::string& name, const Score& score) {
	std::cout << "[Cache] Saving score: " << name << std::endl;
	try {
		// Save to base repository first without holding the lock
		baseRepository->save(name, score);
		
		// Then update cache
		{
			::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
			cache.erase(name);
			std::cout << "[Cache] Removed from cache: " << name << std::endl;
		}
	} catch (const MusicTrainer::RepositoryError& e) {
		std::cerr << "[Cache] Error saving to base repository: " << e.what() << std::endl;
		MusicTrainer::ErrorContext context(__FILE__, __LINE__, __FUNCTION__,
										  "name: " + name + ", operation: save");
		throw MusicTrainer::RepositoryError("Failed to save score to repository: " + std::string(e.what()), context);
	}
}

std::unique_ptr<Score> CachingScoreRepository::load(const std::string& name) {
	std::cout << "[Cache] Loading score: " << name << std::endl;
	
	std::unique_ptr<Score> result;
	bool needBaseLoad = false;
	
	// First check cache
	{
		::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
		totalAccesses++;
		
		auto it = cache.find(name);
		if (it != cache.end() && !isExpired(it->second)) {
			cacheHits++;
			double hitRate = totalAccesses > 0 ? static_cast<double>(cacheHits) / totalAccesses : 0.0;
			std::cout << "[Cache] Cache hit! Hits: " << cacheHits << ", Hit rate: " << hitRate << std::endl;
			result = std::make_unique<Score>(*it->second.score);
		} else {
			if (it != cache.end()) {
				std::cout << "[Cache] Entry expired, removing from cache" << std::endl;
				cache.erase(it);
			} else {
				std::cout << "[Cache] Cache miss - entry not found" << std::endl;
			}
			needBaseLoad = true;
		}
	}
	
	// Load from base repository if needed
	if (needBaseLoad) {
		try {
			std::cout << "[Cache] Loading from base repository" << std::endl;
			result = baseRepository->load(name);
			if (result) {
				::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
				cache.emplace(name, CacheEntry(std::make_unique<Score>(*result), 
							 std::chrono::system_clock::now()));
				std::cout << "[Cache] Added to cache" << std::endl;
			}
		} catch (const MusicTrainer::RepositoryError& e) {
			std::cerr << "[Cache] Error loading from base repository: " << e.what() << std::endl;
			MusicTrainer::ErrorContext context(__FILE__, __LINE__, __FUNCTION__,
											  "name: " + name + ", operation: load");
			throw MusicTrainer::RepositoryError("Failed to load score from repository: " + std::string(e.what()), context);
		}
	}
	
	return result;
}

std::vector<std::string> CachingScoreRepository::listScores() {
	return baseRepository->listScores();
}

void CachingScoreRepository::remove(const std::string& name) {
	// First remove from base repository without holding any locks
	baseRepository->remove(name);
	
	// Then update cache
	{
		::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
		cache.erase(name);
	}
}

void CachingScoreRepository::clearCache() {
	::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
	cache.clear();
	cacheHits = 0;
	totalAccesses = 0;
}

void CachingScoreRepository::setCacheTimeout(std::chrono::seconds timeout) {
	cacheTimeout = timeout;
}

size_t CachingScoreRepository::getCacheSize() const {
	::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
	return cache.size();
}

double CachingScoreRepository::getCacheHitRate() const {
	::utils::TrackedUniqueLock lock(cacheMutex, "CachingScoreRepository::cacheMutex", ::utils::LockLevel::REPOSITORY);
	if (totalAccesses == 0) return 0.0;
	return static_cast<double>(cacheHits) / totalAccesses;
}

void CachingScoreRepository::cleanExpiredEntries() {
	// No-op - we handle expiration in load()
}

bool CachingScoreRepository::isExpired(const CacheEntry& entry) const {
	auto now = std::chrono::system_clock::now();
	auto age = std::chrono::duration_cast<std::chrono::seconds>(now - entry.lastAccess);
	return age > cacheTimeout;
}

} // namespace music::adapters
