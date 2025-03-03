#include "adapters/CachingScoreRepository.h"
#include "domain/errors/DomainErrors.h"
#include <algorithm>
#include <iostream>

namespace music::adapters {

std::unique_ptr<CachingScoreRepository> CachingScoreRepository::create(std::unique_ptr<ScoreRepository> baseRepository) {
	return std::unique_ptr<CachingScoreRepository>(new CachingScoreRepository(std::move(baseRepository)));
}

CachingScoreRepository::CachingScoreRepository(std::unique_ptr<ScoreRepository> baseRepo)
	: baseRepository(std::move(baseRepo)) {}

void CachingScoreRepository::save(const std::string& name, const Score& score) {
	std::cout << "[Cache] Saving score: " << name << std::endl;
	try {
		baseRepository->save(name, score);
		updateCache(name, [&](auto& cache) {
			cache.erase(name);
			std::cout << "[Cache] Removed from cache: " << name << std::endl;
		});
	} catch (const MusicTrainer::RepositoryError& e) {
		std::cerr << "[Cache] Error saving to base repository: " << e.what() << std::endl;
		MusicTrainer::ErrorContext context(__FILE__, __LINE__, __FUNCTION__,
										  "name: " + name + ", operation: save");
		throw MusicTrainer::RepositoryError("Failed to save score to repository: " + std::string(e.what()), context);
	}
}

std::unique_ptr<Score> CachingScoreRepository::load(const std::string& name) {
	std::cout << "[Cache] Loading score: " << name << std::endl;
	
	totalAccesses++;
	
	auto it = cache.find(name);
	if (it != cache.end() && it->second.valid.load(std::memory_order_acquire) && !isExpired(it->second)) {
		cacheHits++;
		double hitRate = static_cast<double>(cacheHits.load()) / totalAccesses.load();
		std::cout << "[Cache] Cache hit! Hits: " << cacheHits << ", Hit rate: " << hitRate << std::endl;
		return std::make_unique<Score>(*it->second.score);
	}
	
	try {
		std::cout << "[Cache] Loading from base repository" << std::endl;
        std::cerr << "[Cache] setErrorHandler called. Address of errorHandler: " << &errorHandler << std::endl;
		auto result = baseRepository->load(name);
		if (result) {
			updateCache(name, [&](auto& cache) {
				auto& newEntry = cache[name];
				newEntry.score = std::make_unique<Score>(*result);
				newEntry.lastAccess = std::chrono::system_clock::now();
				newEntry.valid.store(true);
				std::cout << "[Cache] Added to cache" << std::endl;
			});
		}
		return result;
	} catch (const MusicTrainer::RepositoryError& e) {
		std::cerr << "[Cache] Error loading from base repository: " << e.what() << std::endl;
		MusicTrainer::ErrorContext context(__FILE__, __LINE__, __FUNCTION__,
										  "name: " + name + ", operation: load");
		// Attempt recovery if strategy exists
        if (recoveryStrategy) {
            std::cout << "[Cache] Calling recovery strategy" << std::endl;

            auto result = recoveryStrategy(e);
            if (result.success()) {
                // Call the registered error handler if available to notify about the error that was recovered
                if (errorHandler) {
                    std::cout << "[Cache] Calling error handler after successful recovery" << std::endl;
                    errorHandler(e);
                }
                return std::move(result.score);
            }
        }
        // If recovery fails, also notify the error handler before rethrowing
        if (errorHandler) {
            std::cout << "[Cache] Calling error handler after failed recovery/no strategy" << std::endl;
            std::cerr << "[Cache] Error handler called. Address of errorHandler: " << &errorHandler << std::endl;
            errorHandler(e);
        }
        throw MusicTrainer::RepositoryError("Failed to load score from repository: " + std::string(e.what()), context);
	}
}

std::vector<std::string> CachingScoreRepository::listScores() {
	return baseRepository->listScores();
}

void CachingScoreRepository::remove(const std::string& name) {
	baseRepository->remove(name);
	updateCache(name, [&](auto& cache) {
		cache.erase(name);
	});
}

void CachingScoreRepository::clearCache() {
	updateCache("", [](auto& cache) {
		cache.clear();
	});
	cacheHits.store(0, std::memory_order_release);
	totalAccesses.store(0, std::memory_order_release);
}

void CachingScoreRepository::setCacheTimeout(std::chrono::seconds timeout) {
	cacheTimeout.store(timeout.count(), std::memory_order_release);
}

size_t CachingScoreRepository::getCacheSize() const {
	return cache.size();
}

double CachingScoreRepository::getCacheHitRate() const {
	auto total = totalAccesses.load(std::memory_order_acquire);
	if (total == 0) return 0.0;
	return static_cast<double>(cacheHits.load(std::memory_order_acquire)) / total;
}

void CachingScoreRepository::cleanExpiredEntries() {
	updateCache("", [this](auto& cache) {
		for (auto it = cache.begin(); it != cache.end();) {
			if (isExpired(it->second)) {
				it = cache.erase(it);
			} else {
				++it;
			}
		}
	});
}

bool CachingScoreRepository::isExpired(const CacheEntry& entry) const {
	auto now = std::chrono::system_clock::now();
	auto age = std::chrono::duration_cast<std::chrono::seconds>(now - entry.lastAccess);
	return age.count() > cacheTimeout.load(std::memory_order_acquire);
}

CachingScoreRepository::~CachingScoreRepository() = default;

} // namespace music::adapters
