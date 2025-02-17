#ifndef MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H
#define MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H

#include <unordered_map>
#include <memory>
#include <chrono>
#include <shared_mutex>
#include <atomic>
#include "domain/ports/ScoreRepository.h"
#include "domain/music/Score.h"

namespace music::adapters {

class CachingScoreRepository : public ports::ScoreRepository {
public:
	static std::unique_ptr<CachingScoreRepository> create(std::unique_ptr<ScoreRepository> baseRepository);
	
	void save(const std::string& name, const Score& score) override;
	std::unique_ptr<Score> load(const std::string& name) override;
	std::vector<std::string> listScores() override;
	void remove(const std::string& name) override;
	
	// Cache management
	void clearCache();
	void setCacheTimeout(std::chrono::seconds timeout);
	size_t getCacheSize() const;
	double getCacheHitRate() const;

#ifdef TESTING
	std::shared_mutex& getMutexForTesting() const { return cacheMutex; }
#endif

private:
	CachingScoreRepository(std::unique_ptr<ScoreRepository> baseRepository);
	
	struct CacheEntry {
		std::unique_ptr<Score> score;
		std::chrono::system_clock::time_point lastAccess;
		
		CacheEntry() : lastAccess(std::chrono::system_clock::now()) {}
		CacheEntry(std::unique_ptr<Score> s, std::chrono::system_clock::time_point t)
			: score(std::move(s)), lastAccess(t) {}
	};
	
	std::unique_ptr<ScoreRepository> baseRepository;
	std::unordered_map<std::string, CacheEntry> cache;
	std::chrono::seconds cacheTimeout{300}; // 5 minutes default
	mutable std::shared_mutex cacheMutex;  // Mutex for thread safety
	
	// Use atomic for thread-safe counters
	std::atomic<size_t> cacheHits{0};
	std::atomic<size_t> totalAccesses{0};
	
	void cleanExpiredEntries();
	bool isExpired(const CacheEntry& entry) const;
};

} // namespace music::adapters

#endif // MUSICTRAINERV3_CACHINGSCOREREPOSITORY_H